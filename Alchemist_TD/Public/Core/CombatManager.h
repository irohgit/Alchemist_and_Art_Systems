#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"

class UBoxComponent;
class AEnemyBase;
class APhaseManager;
class AVillagerDefenderManager;
enum class EGamePhase: uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyEnterGateAttackZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGateAttackZoneEmpty);

/**
 * ACombatManager
 *
 * Level-placed manager actor that owns gate-zone combat orchestration:
 * GateAttackZone, enemies inside the zone, attack reporting, and damage
 * dispatch to VillagerDefenderManager.
 *
 * Extracted from APhaseManager in session S-028. PhaseManager becomes a
 * pure phase state machine; CombatManager owns zone overlap and damage flow.
 *
 * Communicates via events:
 *   - Broadcasts OnFirstEnemyEnteredGate / OnGateZoneEmpty (PhaseManager listens
 *     to decide phase transitions).
 *   - Subscribes to PhaseManager::OnPhaseChanged to self-disable during
 *     OmeletteCapture.
 *
 * Skeleton only — state, components, handlers, and events land in subsequent
 * steps of S-028.
 */
UCLASS()
class ALCHEMIST_0_API ACombatManager : public AActor
{
	GENERATED_BODY()
public:
	ACombatManager();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Combat") TObjectPtr<UBoxComponent> GateAttackZone;
	
	// DELEGATES — bind in Blueprint or C++ for phase transitions and reactions
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Combat") FOnEnemyEnterGateAttackZone OnEnemyEnterGateAttackZone;
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Combat") FOnGateAttackZoneEmpty OnGateAttackZoneEmpty;

	// MANAGER-REFS
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Combat") TObjectPtr<AVillagerDefenderManager> VillagerDefenderManagerRef = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Combat") TObjectPtr<APhaseManager> PhaseManagerRef = nullptr;

	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Alchemist|Combat") int32 GetEnemyCountInAttackZone() const;
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Combat") void ReportAttack(int32 DamageValue); // Damage entry point — called by enemy OnEnemyAttacking delegate. Forwards to VDM.

protected:
	virtual void BeginPlay() override;

private:
	TArray<TWeakObjectPtr<AEnemyBase>> EnemiesInAttackZone;

	UFUNCTION() void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION() void OnPhaseChangedHandler(EGamePhase NewPhase); // Subscribed to PhaseManager::OnPhaseChanged — self-disables zone during OmeletteCapture
	
};