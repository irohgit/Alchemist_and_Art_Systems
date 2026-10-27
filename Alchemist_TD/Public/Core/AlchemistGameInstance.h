#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AlchemistGameInstance.generated.h"

UCLASS()
class ALCHEMIST_0_API UAlchemistGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// --- POTION UNLOCKS ---
	UPROPERTY(BlueprintReadWrite, Category = "Unlocks") TSet<FName> UnlockedPotions;
	UPROPERTY(BlueprintReadWrite, Category = "Unlocks") TSet<FName> UnlockedModifiers;

	// --- TOWER UNLOCKS ---
	UPROPERTY(BlueprintReadWrite, Category = "Unlocks") TSet<FName> UnlockedTowers;

	// --- VILLAGER ROSTER ---
	UPROPERTY(BlueprintReadWrite, Category = "Villagers") TArray<FName> LivingCombatVillagers;
	UPROPERTY(BlueprintReadWrite, Category = "Villagers") TArray<FName> DeadCombatVillagers;
	UPROPERTY(BlueprintReadWrite, Category = "Roster") int32 UnlockedVillagerSlots;
	UPROPERTY(BlueprintReadWrite, Category = "Roster") TArray<FName> UnlockedVillagers; // villagers the player has unlocked
	UPROPERTY(BlueprintReadWrite, Category = "Roster") TArray<FName> SelectedCombatVillagers;
	
	// --- PLAYER INVENTORY ---
	UPROPERTY(BlueprintReadWrite, Category = "Inventory") TMap<FName, int32> PotionInventory;
	UPROPERTY(BlueprintReadWrite, Category = "Inventory") int32 Gold;
	UPROPERTY(BlueprintReadWrite, Category = "Inventory") int32 Essence;

	// --- WAVE PROGRESS ---
	UPROPERTY(BlueprintReadWrite, Category = "Progress") int32 CurrentWaveIndex;
	UFUNCTION(BlueprintCallable, Category = "Unlocks") void UnlockPotion(FName PotionName);
	UFUNCTION(BlueprintCallable, Category = "Unlocks") bool IsPotionUnlocked(FName PotionName) const;
	UFUNCTION(BlueprintCallable, Category = "Villagers") void KillVillager(FName VillagerName);
	UFUNCTION(BlueprintCallable, Category = "Villagers") void ReviveVillager(FName VillagerName);
	UFUNCTION(BlueprintCallable, Category = "Run") void ResetRun();
protected:
	virtual void Init() override;
};