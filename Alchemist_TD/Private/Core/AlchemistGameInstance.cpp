#include "Core/AlchemistGameInstance.h"

void UAlchemistGameInstance::Init()
{
	Super::Init();

	Gold = 0;
	Essence = 0;
	CurrentWaveIndex = 0;
	UnlockedVillagerSlots = 2;

	// Starting potions
	UnlockedPotions.Add(FName("Fire"));
	UnlockedPotions.Add(FName("Ice"));

	// First two villagers unlocked by default
	UnlockedVillagers.Add(FName("Vick"));
	UnlockedVillagers.Add(FName("Lexy"));
}

void UAlchemistGameInstance::UnlockPotion(FName PotionName)
{
	UnlockedPotions.Add(PotionName);
}

bool UAlchemistGameInstance::IsPotionUnlocked(FName PotionName) const
{
	return UnlockedPotions.Contains(PotionName);
}

void UAlchemistGameInstance::KillVillager(FName VillagerName)
{
	LivingCombatVillagers.Remove(VillagerName);
	DeadCombatVillagers.AddUnique(VillagerName);
}

void UAlchemistGameInstance::ReviveVillager(FName VillagerName)
{
	DeadCombatVillagers.Remove(VillagerName);
	LivingCombatVillagers.AddUnique(VillagerName);
}

void UAlchemistGameInstance::ResetRun()
{
	// Reset living/dead back to whoever was selected for this run
	LivingCombatVillagers = SelectedCombatVillagers;
	DeadCombatVillagers.Empty();
	PotionInventory.Empty();
	Gold = 0;
	Essence = 0;
	CurrentWaveIndex = 0;

	// NOT reset — permanent progression
	// UnlockedVillagers
	// UnlockedVillagerSlots
	// UnlockedPotions
	// UnlockedModifiers
	// UnlockedTowers
}