#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VillagerDefenderManager.generated.h"

class AVillagerDefender;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVillagerDied, AVillagerDefender*, Villager);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllVillagersDead);

/**
 * AVillagerDefenderManager
 *
 * Level-placed manager actor that owns all villager-defender state and behaviour:
 * roster, slot positions, current defender, damage application, redistribution,
 * revival, and "all dead" tracking.
 *
 */
UCLASS()
class ALCHEMIST_0_API AVillagerDefenderManager : public AActor
{
    GENERATED_BODY()

public:
    AVillagerDefenderManager();
    //DELEGATES
    UPROPERTY(BlueprintAssignable, Category = "Alchemist|Villagers") FOnVillagerDied OnVillagerDied;
    UPROPERTY(BlueprintAssignable, Category="Alchemist|Villagers") FOnAllVillagersDead OnAllVillagersDead;
    
    // CONFIG — level designer fills this with the 7 defender villagers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villagers") TArray<TObjectPtr<AVillagerDefender>> Villagers;

    // STATE — true once all villagers have died this run
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Villagers") bool bAllVillagersDeadForRun = false;

    // PUBLIC API — villager operations
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Villagers") void RedistributeVillagers();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Villagers") void ReviveRandomVillager();
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Alchemist|Villagers") int32 GetLivingVillagerCount() const;
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Villagers") void ApplyDamageToRandomVillager(int32 DamageValue);

protected:
    virtual void BeginPlay() override;

private:
    // Cached world locations of villagers at level start — used by RedistributeVillagers
    TArray<FVector> VillagerSlotPositions;

    void BuildSlotPositions();
};
