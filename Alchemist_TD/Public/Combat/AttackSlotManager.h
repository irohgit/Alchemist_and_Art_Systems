#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AttackSlotManager.generated.h"

class AEnemyBase;
class APhaseManager;
class UBoxComponent;

UCLASS()
class ALCHEMIST_0_API AAttackSlotManager : public AActor
{
    GENERATED_BODY()

public:
    AAttackSlotManager();

    // -------------------------------------------------------------------------
    // CONFIGURATION
    // -------------------------------------------------------------------------

    // Box collision — sized to cover full gate width and height
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|AttackSlot") TObjectPtr<UBoxComponent> GateBox;

    // 10 world space slot positions — set manually in Details panel
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|AttackSlot")
    TArray<FVector> SlotPositions;

    // How often the manager queries for unclaimed enemies inside the box
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|AttackSlot")
    float QueryInterval = 0.5f;

    // Set by PhaseManager on BeginPlay
    UPROPERTY(BlueprintReadWrite, Category = "Alchemist|AttackSlot") TObjectPtr<APhaseManager> PhaseManagerRef = nullptr;

    // -------------------------------------------------------------------------
    // STATE
    // -------------------------------------------------------------------------

    // Index = slot index. Value = assigned enemy. Null = slot free.
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|AttackSlot") TArray<TObjectPtr<AEnemyBase>> OccupiedSlots;

    // -------------------------------------------------------------------------
    // PUBLIC API
    // -------------------------------------------------------------------------

    // Called by EnemyBase::OnDeath — frees the slot this enemy occupied
    UFUNCTION(BlueprintCallable, Category = "Alchemist|AttackSlot") void ReleaseSlot(AEnemyBase* Enemy);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Alchemist|AttackSlot") int32 GetOccupiedSlotCount() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Alchemist|AttackSlot") bool AreAllSlotsOccupied() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Alchemist|AttackSlot") bool AreAllSlotsEmpty() const;
    UFUNCTION(BlueprintCallable, Category = "Alchemist|AttackSlot") void DisableSlotQuery();

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle QueryTimer;

    void QueryForEnemies();
    void ClaimSlot(int32 SlotIndex, AEnemyBase* Enemy);
};