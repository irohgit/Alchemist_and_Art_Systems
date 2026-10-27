#include "Core/CombatManager.h"
#include "Components/BoxComponent.h"
#include "Core/PhaseManager.h"
#include "Core/VillagerDefenderManager.h"
#include "Enemies/EnemyBase.h"
#include "Kismet/GameplayStatics.h"

ACombatManager::ACombatManager()
{
	PrimaryActorTick.bCanEverTick = false;
	GateAttackZone = CreateDefaultSubobject<UBoxComponent>(TEXT("GateAttackZone"));
	RootComponent = GateAttackZone;
	GateAttackZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GateAttackZone->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GateAttackZone->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
	GateAttackZone->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	GateAttackZone->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Ignore);
}
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	VillagerDefenderManagerRef = Cast<AVillagerDefenderManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AVillagerDefenderManager::StaticClass()));
	if (!VillagerDefenderManagerRef)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("CombatManager: VillagerDefenderManager not found in level"));
	}

	PhaseManagerRef = Cast<APhaseManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APhaseManager::StaticClass()));
	if (!PhaseManagerRef)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("CombatManager: PhaseManager not found in level"));
	}
	else
	{
		PhaseManagerRef->OnPhaseChanged.AddDynamic(this, &ACombatManager::OnPhaseChangedHandler);
	}

	if (GateAttackZone)
	{
		GateAttackZone->OnComponentBeginOverlap.AddDynamic(this, &ACombatManager::OnZoneBeginOverlap);
		GateAttackZone->OnComponentEndOverlap.AddDynamic(this, &ACombatManager::OnZoneEndOverlap);
	}
}
int32 ACombatManager::GetEnemyCountInAttackZone() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AEnemyBase>& Ptr : EnemiesInAttackZone)
	{
		if (Ptr.IsValid()) Count++;
	}
	return Count;;
}
void ACombatManager::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
    if (!IsValid(Enemy)) return;
    if (Enemy->bIsDead) return;
    EnemiesInAttackZone.AddUnique(TWeakObjectPtr<AEnemyBase>(Enemy));
    Enemy->StartAttacking();
    Enemy->OnEnemyAttacking.AddDynamic(this, &ACombatManager::ReportAttack);
    UE_LOG(LogTemp, Warning, TEXT("GateAttackZone: %s entered zone"), *Enemy->GetName());
	OnEnemyEnterGateAttackZone.Broadcast();
}
void ACombatManager::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
    if (!IsValid(Enemy)) return;
    EnemiesInAttackZone.RemoveAll(
        [Enemy](const TWeakObjectPtr<AEnemyBase>& Ptr)
        {
            return Ptr.Get() == Enemy;
        }
    );
    Enemy->StopAttacking();
    Enemy->OnEnemyAttacking.RemoveDynamic(this, &ACombatManager::ReportAttack);
	if (EnemiesInAttackZone.IsEmpty())
	{
		OnGateAttackZoneEmpty.Broadcast();
	}
    UE_LOG(LogTemp, Warning, TEXT("GateAttackZone: %s left zone"), *Enemy->GetName());
}
void ACombatManager::ReportAttack(int32 DamageValue)
{
    if (VillagerDefenderManagerRef)
    {
        VillagerDefenderManagerRef->ApplyDamageToRandomVillager(DamageValue);
    }
}
void ACombatManager::OnPhaseChangedHandler(EGamePhase NewPhase)
{
    if (NewPhase == EGamePhase::OmeletteCapture)
    {
        // Stop all current attackers, clear zone, disable collision
        for (TWeakObjectPtr<AEnemyBase>& Ptr : EnemiesInAttackZone)
        {
            if (Ptr.IsValid())
            {
                Ptr->StopAttacking();
                Ptr->OnEnemyAttacking.RemoveDynamic(this, &ACombatManager::ReportAttack);
            }
        }
        EnemiesInAttackZone.Empty();
        if (GateAttackZone)
        {
            GateAttackZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    else if (NewPhase == EGamePhase::LaneDefence)
    {
        // Re-enable zone for next wave
        if (GateAttackZone)
        {
            GateAttackZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }
}