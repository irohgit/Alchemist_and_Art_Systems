#include "Enemies/EnemyPoolComponent.h"
#include "Characters/Omelette.h"
#include "Enemies/EnemyBase.h"
#include "Engine/World.h"
#include "Core/PhaseManager.h"

UEnemyPoolComponent::UEnemyPoolComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}
void UEnemyPoolComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializePool();
}
void UEnemyPoolComponent::Activate(AEnemyBase* Enemy)
{
    if (!IsValid(Enemy)) return;
    Enemy->bIsInUse = true;
    Enemy->SetActorHiddenInGame(false);
    Enemy->SetActorEnableCollision(true);
    Enemy->GetMesh()->SetVisibility(true);
}
void UEnemyPoolComponent::Deactivate(AEnemyBase* Enemy)
{
    if (!IsValid(Enemy)) return;
    Enemy->bIsInUse = false;
    Enemy->SetActorHiddenInGame(true);
    Enemy->SetActorEnableCollision(false);
    Enemy->GetMesh()->SetVisibility(false);
}
void UEnemyPoolComponent::InitializePool()
{
    for (auto& Entry : PoolSizes)
    {
        const EEnemyType EnemyType = Entry.Key;
        const int32 Count = Entry.Value;

        TSubclassOf<AEnemyBase>* FoundClass = EnemyClasses.Find(EnemyType);
        if (!FoundClass || !*FoundClass)
        {
            UE_LOG(LogTemp, Error, TEXT("EnemyPoolComponent: No class assigned for EnemyType=%d — skipping pool init for this type"), (int32)EnemyType);
            continue;
        }
        SpawnIntoPool(EnemyType, *FoundClass, Count);
    }
    UE_LOG(LogTemp, Log, TEXT("EnemyPoolComponent: Pool initialized — %d types"), Pool.Num());
}
void UEnemyPoolComponent::ReturnToPool(AEnemyBase* Enemy)
{
    if (!IsValid(Enemy)) return;
    Enemy->GetWorldTimerManager().ClearTimer(Enemy->ChaseTimer);
    if (Enemy->bIsCarryingOmelette && IsValid(Enemy->CarriedOmelette))
    {
        Enemy->DropOmelette();
    }
    Enemy->OnReturnToPool();
    Enemy->CurrentWaypointIndex = 0;
    Enemy->bIsDead = false;
    Enemy->bMovementHeld = true;
    Enemy->bIsRetrievingOmelette = false;
    Enemy->bIsInUse = false;
    Enemy->LateralOffset = 0.f;
    if (IsValid(Enemy->PhaseManagerRef)){Enemy->PhaseManagerRef->OnPhaseChanged.RemoveDynamic(Enemy, &AEnemyBase::HandlePhaseChanged);}
    Enemy->PhaseManagerRef = nullptr;
    Enemy->WaveManagerRef = nullptr;
    Enemy->CarriedOmelette = nullptr;
    // Stop AI movement
    if (AAIController* AI = Cast<AAIController>(Enemy->GetController()))
    {
        AI->StopMovement();
    }
    // Clear status effects 
    if (IsValid(Enemy->StatusEffectComponent))
    {
        //TODO Enemy->StatusEffectComponent->ClearAllEffects();
        Enemy->StatusEffectComponent->OnStatusApplied.RemoveDynamic(Enemy, &AEnemyBase::OnStatusApplied);
        Enemy->StatusEffectComponent->OnStatusExpired.RemoveDynamic(Enemy, &AEnemyBase::OnStatusExpired);
    }
    Deactivate(Enemy);
}
void UEnemyPoolComponent::SpawnIntoPool(EEnemyType EnemyType, TSubclassOf<AEnemyBase> EnemyClass, int32 Count)
{
    UWorld* World = GetWorld();
    if (!World) return;
    TArray<TObjectPtr<AEnemyBase>>& TypePool = Pool.FindOrAdd(EnemyType);
    TypePool.Reserve(TypePool.Num() + Count);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    for (int32 i = 0; i < Count; i++)
    {
        AEnemyBase* Enemy = World->SpawnActor<AEnemyBase>(EnemyClass,FVector::ZeroVector,FRotator::ZeroRotator,Params);
        Enemy->bIsPooled = true;
        Deactivate(Enemy);
        TypePool.Add(Enemy);
    }
}
AEnemyBase* UEnemyPoolComponent::GetFromPool(EEnemyType EnemyType)
{
    TArray<TObjectPtr<AEnemyBase>>* TypePool = Pool.Find(EnemyType);
    if (!TypePool)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyPoolComponent: No pool exists for EnemyType=%d — was InitializePool called?"), (int32)EnemyType);
        return nullptr;
    }
    for (TObjectPtr<AEnemyBase>& EnemyPtr : *TypePool)
    {
        if (!IsValid(EnemyPtr)) continue;
        if (EnemyPtr->bIsInUse) continue;
        Activate(EnemyPtr);
        return EnemyPtr;
    }
    UE_LOG(LogTemp, Error, TEXT("EnemyPoolComponent: Pool exhausted for EnemyType=%d — increase PoolSizes"), (int32)EnemyType);
    return nullptr;
}
int32 UEnemyPoolComponent::GetActiveCount(EEnemyType EnemyType) const
{
    const TArray<TObjectPtr<AEnemyBase>>* TypePool = Pool.Find(EnemyType);
    if (!TypePool) return 0;
    int32 Count = 0;
    for (const TObjectPtr<AEnemyBase>& EnemyPtr : *TypePool)
    {
        if (IsValid(EnemyPtr) && EnemyPtr->bIsInUse) Count++;
    }
    return Count;
}
int32 UEnemyPoolComponent::GetPoolSize(EEnemyType EnemyType) const
{
    const TArray<TObjectPtr<AEnemyBase>>* TypePool = Pool.Find(EnemyType);
    return TypePool ? TypePool->Num() : 0;
}