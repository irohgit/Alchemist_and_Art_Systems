#include "Core/VillagerDefenderManager.h"
#include "Characters/VillagerDefender.h"
#include "Engine/DamageEvents.h"

AVillagerDefenderManager::AVillagerDefenderManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AVillagerDefenderManager::BeginPlay()
{
    Super::BeginPlay();
    BuildSlotPositions();
}
void AVillagerDefenderManager::BuildSlotPositions()
{
    VillagerSlotPositions.Empty();
    for (AVillagerDefender* Villager : Villagers)
    {
        if (IsValid(Villager))
        {
            VillagerSlotPositions.Add(Villager->GetActorLocation());
        }
    }
}
void AVillagerDefenderManager::RedistributeVillagers()
{
    if (VillagerSlotPositions.Num() == 0) return;

    TArray<AVillagerDefender*> LivingVillagers;
    for (AVillagerDefender* Villager : Villagers)
    {
        if (IsValid(Villager) && Villager->bIsAliveForRun)
        {
            LivingVillagers.Add(Villager);
        }
    }
    // Centre-pack: with 7 slots and 5 living, fill slots 1-5 (one empty each side) rather than 0-4 (left-stacked, two empty on the right).
    const int32 NumSlots = VillagerSlotPositions.Num();
    const int32 NumLiving = LivingVillagers.Num();
    const int32 StartIndex = (NumSlots - NumLiving) / 2;

    for (int32 i = 0; i < NumLiving; i++)
    {
        const int32 SlotIndex = StartIndex + i;
        if (!VillagerSlotPositions.IsValidIndex(SlotIndex)) break;
        LivingVillagers[i]->AssignedSlotIndex = SlotIndex;
        LivingVillagers[i]->MoveToPosition(VillagerSlotPositions[SlotIndex]);
    }
}
void AVillagerDefenderManager::ReviveRandomVillager()
{
    TArray<AVillagerDefender*> DeadVillagers;
    for (AVillagerDefender* Villager : Villagers)
    {
        if (IsValid(Villager) && !Villager->bIsAliveForRun)
        {
            DeadVillagers.Add(Villager);
        }
    }

    if (DeadVillagers.Num() == 0) return;

    const int32 RandomIndex = FMath::RandRange(0, DeadVillagers.Num() - 1);
    AVillagerDefender* Revived = DeadVillagers[RandomIndex];

    Revived->bIsAliveForRun = true;
    Revived->Health = Revived->MaxHealth;
    bAllVillagersDeadForRun = false;
    Revived->OnRevived();
}
int32 AVillagerDefenderManager::GetLivingVillagerCount() const
{
    int32 Count = 0;
    for (const AVillagerDefender* Villager : Villagers)
    {
        if (IsValid(Villager) && Villager->bIsAliveForRun)
        {
            Count++;
        }
    }
    return Count;
}
void AVillagerDefenderManager::ApplyDamageToRandomVillager(int32 DamageValue)
{
    TArray<AVillagerDefender*> LivingVillagers;
    for (AVillagerDefender* Villager : Villagers)
    {
        if (IsValid(Villager) && Villager->bIsAliveForRun) LivingVillagers.Add(Villager);
    }
    if (LivingVillagers.IsEmpty()) return;

    const int32 Index = FMath::RandRange(0, LivingVillagers.Num() - 1);
    AVillagerDefender* Target = LivingVillagers[Index];
    UE_LOG(LogTemp, Warning, TEXT("ApplyDamageToRandomVillager — dealing %d damage to %s"), DamageValue, *Target->VillagerName);

    Target->TakeDamage(static_cast<float>(DamageValue), FDamageEvent(), nullptr, this);
    UE_LOG(LogTemp, Warning, TEXT("ApplyDamageToRandomVillager — %s Health after damage: %.1f bIsAliveForRun: %d"), *Target->VillagerName, Target->Health, (int32)Target->bIsAliveForRun);

    if (Target->Health <= 0.f && Target->bIsAliveForRun)
    {
        Target->bIsAliveForRun = false;
        OnVillagerDied.Broadcast(Target);
        Target->PlayDeathSequence();

        if (GetLivingVillagerCount() <= 0)
        {
            bAllVillagersDeadForRun = true;
            OnAllVillagersDead.Broadcast();
            return;
        }
        RedistributeVillagers();
    }
}