#include "Characters/VillagerDefender.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"

AVillagerDefender::AVillagerDefender()
{
	PrimaryActorTick.bCanEverTick = false;
	WeaponRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponRight"));WeaponRight->SetupAttachment(GetMesh(), FName("Weapon_R"));WeaponRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponLeft")); WeaponLeft->SetupAttachment(GetMesh(), FName("Weapon_L")); WeaponLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AVillagerDefender::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(AttackAnimQueryTimer,this,&AVillagerDefender::QueryForEnemiesInRange,0.5f,true,0.5f);
}

void AVillagerDefender::MoveToPosition(FVector TargetPosition)
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		FAIMoveRequest MoveRequest; MoveRequest.SetGoalLocation(TargetPosition); MoveRequest.SetAcceptanceRadius(20.f);
		FNavPathSharedPtr NavPath;
		AI->MoveTo(MoveRequest, &NavPath);
	}
}
float AVillagerDefender::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health -= DamageAmount;
	UE_LOG(LogTemp, Warning, TEXT("VillagerDefender %s took %.1f damage — Health now %.1f / %.1f"),  *VillagerName, DamageAmount, Health, MaxHealth);
	return DamageAmount;
}

void AVillagerDefender::PlayDeathSequence()
{
	if (AAIController* AI = Cast<AAIController>(GetController())) {	AI->StopMovement(); }  
	if (DeathAnimation && GetMesh())
	{
		GetMesh()->PlayAnimation(DeathAnimation, false);
		float DeathDuration = DeathAnimation->GetPlayLength();
		GetWorldTimerManager().SetTimer(DestroyTimer, this, &AVillagerDefender::DestroyVillager, DeathDuration, false);
	}
	else{ GetWorldTimerManager().SetTimer(DestroyTimer, this, &AVillagerDefender::DestroyVillager, 0.1f, false);}
}

void AVillagerDefender::QueryForEnemiesInRange()
{
	if (!IsValid(this)) return;
	if (!bIsAliveForRun) return;
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(AttackAnimationRadius);
	FCollisionQueryParams QueryParams; QueryParams.AddIgnoredActor(this);
	const bool bFoundEnemy = GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(),FQuat::Identity,ECollisionChannel::ECC_GameTraceChannel2, CollisionShape,QueryParams) && OverlapResults.ContainsByPredicate([](const FOverlapResult& R){ return IsValid(R.GetActor()) && R.GetActor()->ActorHasTag(TEXT("Enemy"));});
	if (bFoundEnemy && !bEnemyCurrentlyInRange) { bEnemyCurrentlyInRange = true; OnEnemyInRange();}
	else if (!bFoundEnemy && bEnemyCurrentlyInRange){bEnemyCurrentlyInRange = false; OnNoEnemiesInRange();}
}


void AVillagerDefender::DestroyVillager()
{
	UE_LOG(LogTemp, Warning, TEXT("VillagerDefender %s: entering death state (hidden + collision off)"), *VillagerName);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}
}
void AVillagerDefender::OnRevived()
{
	UE_LOG(LogTemp, Warning, TEXT("VillagerDefender %s: revived — Health=%.1f"), *VillagerName, Health);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	OnReviveBP();   // BP wires this to play idle/rise animation, particle effects, audio sting
}

void AVillagerDefender::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(AttackAnimQueryTimer);
	GetWorldTimerManager().ClearTimer(DestroyTimer);
	Super::EndPlay(EndPlayReason);
}

// --- FUTURE IMPLEMENTATION

/*

void AVillagerDefender::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted){
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple,
		FString::Printf(TEXT("OnDeathMontageEnded — interrupted: %s"), bInterrupted ? TEXT("YES") : TEXT("NO")));
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AVillagerDefender::DestroyVillager, 0.5f, false);
}


void AVillagerDefender::PlayDeathSequence()
{
	
	 * This function will play a death montage
	 * (1) Stop Movement (2) Play Death Montage and Bind End Delegate (else2) Destroy if no montage assigned
	
// Stop all movement
if (AAIController* AI = Cast<AAIController>(GetController()))
{
	AI->StopMovement();
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
FString::Printf(TEXT("AIController valid: %s"), 
	Cast<AAIController>(GetController()) ? TEXT("YES") : TEXT("NO")));
}

// Play death montage and bind end delegate
if (DeathMontage && GetMesh() && GetMesh()->GetAnimInstance())
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	//AnimInstance->OnMontageEnded.AddDynamic(this, &AVillagerDefender::OnDeathMontageEnded);
	AnimInstance->Montage_Play(DeathMontage);
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White,
FString::Printf(TEXT("%s montage played"), *VillagerName));
}
else
{
	// No montage assigned — destroy immediately
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AVillagerDefender::DestroyVillager, 0.1f, false);
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red,
FString::Printf(TEXT("%s no montage"), *VillagerName));
}
}

 */

// --- FUTURE IMPLEMENTATION