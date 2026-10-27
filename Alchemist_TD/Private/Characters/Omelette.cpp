#include "Characters/Omelette.h"
#include "Enemies/EnemyBase.h"
#include "Components/SphereComponent.h"
#include "Enemies/EnemyCapturer.h"
#include "Player/AlchemistPlayer.h"

AOmelette::AOmelette()
{
	PrimaryActorTick.bCanEverTick = false;

	OmeletteMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("OmeletteMesh"));
	RootComponent = OmeletteMesh;
	OmeletteMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CaptureTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("CaptureTrigger"));
    CaptureTrigger->SetupAttachment(RootComponent);
    CaptureTrigger->SetSphereRadius(CaptureRadius);
    CaptureTrigger->SetCollisionProfileName(TEXT("Trigger"));
	CaptureTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}
void AOmelette::BeginPlay()
{
	Super::BeginPlay();
	CaptureTrigger->OnComponentBeginOverlap.AddDynamic(
		this, &AOmelette::OnCarrierOverlap);
}

void AOmelette::OnCarrierOverlap( UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCarrierOverlap fired — bIsDropped=%d OtherActor=%s"), 
	(int32)bIsDropped, *OtherActor->GetName());
	if (!bIsDropped) return;
	if (ACapturer* Capturer = Cast<ACapturer>(OtherActor)) { Capturer->PickUpOmelette(this); return; } // Capturer picks up
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor)) {if(Enemy->bIsRetrievingOmelette){Enemy->PickUpOmelette(this);return;}}	// Enemy retriever picks up
	if (AAlchemistPlayer* Player = Cast<AAlchemistPlayer>(OtherActor)){Player->PickUpOmelette(this);}// Player picks up
}

void AOmelette::OnDropped()
{
	bIsDropped = true;
	CaptureTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CaptureTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Snap to floor so navmesh-pathing retrievers can actually reach the goal location.
	// Without this the omelette stays at the carrier's socket height (mid-air) after
	// detach, and AI MoveTo never satisfies its acceptance radius.
	{
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0.f, 0.f, 500.f);
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
		{
			SetActorLocation(Hit.Location);
		}
	}
	
	OnOmeletteDropped.Broadcast(this);
	CaptureTrigger->UpdateOverlaps();
}

void AOmelette::OnPickedUp()
{
	UE_LOG(LogTemp, Warning, TEXT("AOmelette::OnPickedUp called"));
	bIsDropped = false;
	CaptureTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AOmelette::OnSavedAtGoalVolume(const FVector& SaveLocation)
{
	UE_LOG(LogTemp, Warning, TEXT("AOmelette::OnSavedAtGoalVolume called — placing at %s"), *SaveLocation.ToString());
	bIsDropped = false;
	CaptureTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Detach from carrier (player), then place upright on the goal
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocationAndRotation(SaveLocation, FRotator(0.f, 90.f, 0.f));
	// Stays visible — sits on the goal as a "saved" indicator
}