#include "Combat/AttackSlotManager.h"
#include "Enemies/EnemyBase.h"
#include "Core/PhaseManager.h"
#include "Components/BoxComponent.h"
#include "Engine/OverlapResult.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"

AAttackSlotManager::AAttackSlotManager()
{
    PrimaryActorTick.bCanEverTick = false;

    GateBox = CreateDefaultSubobject<UBoxComponent>(TEXT("GateBox"));
    RootComponent = GateBox;
}

void AAttackSlotManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialise occupied slots array — one entry per slot position, all null
    OccupiedSlots.SetNum(SlotPositions.Num());

    GetWorldTimerManager().SetTimer(
        QueryTimer,
        this,
        &AAttackSlotManager::QueryForEnemies,
        QueryInterval,
        true,
        QueryInterval
    );
}

// -----------------------------------------------------------------------------
// QueryForEnemies
// Box overlap finds all enemies inside the gate box.
// Assigns unclaimed enemies to free slots in order.
// -----------------------------------------------------------------------------
void AAttackSlotManager::QueryForEnemies()
{
    if (SlotPositions.Num() == 0) return;

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        GateBox->GetComponentLocation(),
        GateBox->GetComponentQuat(),
        ECollisionChannel::ECC_GameTraceChannel2,
        GateBox->GetCollisionShape(),
        QueryParams
    );

    for (const FOverlapResult& Result : OverlapResults)
    {
        AEnemyBase* Enemy = Cast<AEnemyBase>(Result.GetActor());
        if (!IsValid(Enemy)) continue;
        if (Enemy->bIsDead) continue;
        // Find first free slot
        for (int32 i = 0; i < OccupiedSlots.Num(); i++)
        {
            if (OccupiedSlots[i] == nullptr)
            {
                ClaimSlot(i, Enemy);
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// ClaimSlot
// Assigns enemy to slot index — moves enemy to slot world position.
// -----------------------------------------------------------------------------
void AAttackSlotManager::ClaimSlot(int32 SlotIndex, AEnemyBase* Enemy)
{
    if (!IsValid(Enemy)) return;
    // if (Enemy->ClaimedSlot != nullptr) return;
    if (!OccupiedSlots.IsValidIndex(SlotIndex)) return;
    if (OccupiedSlots[SlotIndex] != nullptr) return;

    OccupiedSlots[SlotIndex] = Enemy;
    // Enemy->ClaimedSlot = this;
    // Enemy->bIsAtAttackSlot = true;

    if (AAIController* AI = Cast<AAIController>(Enemy->GetController()))
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(SlotPositions[SlotIndex]);
        MoveRequest.SetAcceptanceRadius(10.f);
        FNavPathSharedPtr NavPath;
        AI->MoveTo(MoveRequest, &NavPath);
    }
    
}

// -----------------------------------------------------------------------------
// ReleaseSlot
// Called by EnemyBase::OnDeath — finds which slot this enemy occupied and frees it.
// -----------------------------------------------------------------------------
void AAttackSlotManager::ReleaseSlot(AEnemyBase* Enemy)
{
    for (int32 i = 0; i < OccupiedSlots.Num(); i++)
    {
        if (OccupiedSlots[i] == Enemy)
        {
            OccupiedSlots[i] = nullptr;
            
            return;
        }
    }
}

int32 AAttackSlotManager::GetOccupiedSlotCount() const
{
    int32 Count = 0;
    for (const AEnemyBase* Enemy : OccupiedSlots)
    {
        if (IsValid(Enemy)) Count++;
    }
    return Count;
}

bool AAttackSlotManager::AreAllSlotsOccupied() const
{
    if (OccupiedSlots.Num() == 0) return false;
    for (const AEnemyBase* Enemy : OccupiedSlots)
    {
        if (!IsValid(Enemy)) return false;
    }
    return true;
}

bool AAttackSlotManager::AreAllSlotsEmpty() const
{
    for (const AEnemyBase* Enemy : OccupiedSlots)
    {
        if (IsValid(Enemy)) return false;
    }
    return true;
}

void AAttackSlotManager::DisableSlotQuery()
{
    GetWorldTimerManager().ClearTimer(QueryTimer);
}