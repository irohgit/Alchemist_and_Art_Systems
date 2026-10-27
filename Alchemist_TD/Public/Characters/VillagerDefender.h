#pragma once

#include "CoreMinimal.h"
#include "Characters/VillagerNPC.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "VillagerDefender.generated.h"


/**
 * AVillagerDefender
 *
 * Combat child of AVillagerNPC. Pre-placed in combat scene — 7 total.
 * Owned by PhaseManager for redistribution.
 *
 * OWNS:
 *   Movement to assigned slot position + Attack animation state. + Nearest enemy detection for attack animation trigger.
 *
 * INHERITS FROM AVillagerNPC:
 *   Health, MaxHealth, bIsAliveForRun, VillagerName, VillagerID. + All Blueprint hooks from VillagerNPC.
 *
 * DOES NOT OWN:
 *   Damage application — CombatManager applies damage directly to Health.
 *   Phase transitions — PhaseManager owns those.
 *   Slot position array — PhaseManager owns that.
 */
UCLASS()
class ALCHEMIST_0_API AVillagerDefender : public AVillagerNPC
{
    GENERATED_BODY()

public:

    AVillagerDefender();

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Villager|Combat") int32 AssignedSlotIndex = -1; //Current slot index assigned by PhaseManager — used for redistribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villager|Combat") float AttackAnimationRadius = 200.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Villager|Combat") TObjectPtr<UStaticMeshComponent> WeaponRight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Villager|Combat") TObjectPtr<UStaticMeshComponent> WeaponLeft;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Villager|Combat") TObjectPtr<UAnimSequence> AttackAnimation = nullptr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Villager|Combat") TObjectPtr<UAnimSequence> DeathAnimation = nullptr;
    
    // PUBLIC API
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Villager|Combat") void MoveToPosition(FVector TargetPosition); // Called by PhaseManager to move villager to a new slot position
    
    // BLUEPRINT HOOKS — combat specific
    UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Villager|Combat") void OnEnemyInRange();
    UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Villager|Combat") void OnNoEnemiesInRange();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Villager|Combat") void PlayDeathSequence();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Villager|Combat") void OnRevived();
    UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Villager|Combat") void OnReviveBP();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:

    FTimerHandle DestroyTimer;
    void DestroyVillager();

    //Attack
    FTimerHandle AttackAnimQueryTimer;
    bool bEnemyCurrentlyInRange = false;
    void QueryForEnemiesInRange();
   
};

// --- FUTURE IMPLEMENTATION ---

/*

// Called when death montage completes — destroys the actor
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Villager|Combat")
    void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
 
 MONTAGE ANIMATION - TO BE ADDED WHEN ANIM BP IS READY
// Attack Montage - assign in Blueprint child defaults
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villager|Combat")
UAnimMontage* AttackMontage = nullptr;
// Death montage — assign in Blueprint child defaults
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villager|Combat")
UAnimMontage* DeathMontage = nullptr;
*/

/// --- FUTURE IMPLEMENTATION 