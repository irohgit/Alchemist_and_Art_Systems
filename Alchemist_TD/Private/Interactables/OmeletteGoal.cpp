#include "Interactables/OmeletteGoal.h"
#include "Components/BoxComponent.h"
#include "Characters/Omelette.h"
#include "Player/AlchemistPlayer.h"

AOmeletteGoal::AOmeletteGoal()
{
	PrimaryActorTick.bCanEverTick = false;
	OmeletteGoalVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ReturnedGoalVolume"));
	RootComponent = OmeletteGoalVolume;
	OmeletteGoalVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OmeletteGoalVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	OmeletteGoalVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	OmeletteGoalVolume->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
}

void AOmeletteGoal::BeginPlay()
{
	Super::BeginPlay();
	OmeletteGoalVolume->OnComponentBeginOverlap.AddDynamic(this, &AOmeletteGoal::OnOmeletteGoalVolumeBeginOverlap);
}

void AOmeletteGoal::OnOmeletteGoalVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAlchemistPlayer* Player = Cast<AAlchemistPlayer>(OtherActor);
	if (!Player) return;
	if (!Player->bIsCarryingOmelette) return;

	AOmelette* Omelette = Player->CarriedOmelette;
	if (!IsValid(Omelette)) return;

	UE_LOG(LogTemp, Warning, TEXT("AOmeletteGoal: Player Returned Omelette"));

	Omelette->OnSavedAtGoalVolume(GetActorLocation());
	Player->CarriedOmelette = nullptr;
	Player->bIsCarryingOmelette = false;

	OnOmeletteReturned.Broadcast();
}

