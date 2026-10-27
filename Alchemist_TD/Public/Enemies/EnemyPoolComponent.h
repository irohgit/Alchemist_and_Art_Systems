#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FormationData.h"
#include "EnemyPoolComponent.generated.h"

class AEnemyBase;
/*
 * UEnemyPoolComponent - SUMMARY
 *
 * (1) - OWNERSHIP
 * Attached to AWaveManager. Owns all pre-spawned enemy actors for the level.
 * Replaces SpawnActor/DestroyActor calls with activate/deactivate operations.
 *
 * (2) - POOL STRUCTURE
 * One TArray per EEnemyType. All actors pre-spawned at BeginPlay, hidden,
 * collision disabled, tick disabled. Activated on demand by SpawnBatch.
 * Returned to pool on enemy death instead of DestroyActor.
 *
 * (3) - PUBLIC API
 * InitializePool()              — Call from WaveManager::BeginPlay. Pre-spawns all actors.
 * GetFromPool(EEnemyType)       — Returns first inactive enemy of given type. Null if pool exhausted.
 * ReturnToPool(AEnemyBase*)     — Resets enemy state and deactivates. Call from AEnemyBase::OnDeath.
 *
 * (4) - CONFIGURATION
 * EnemyClasses  — TMap of EEnemyType to Blueprint subclass. Assign in Details panel.
 * PoolSizes     — TMap of EEnemyType to pre-spawn count. Assign in Details panel.
 *
 * (5) - DEPENDENCIES
 * AEnemyBase — all pooled enemies must inherit from AEnemyBase
 * EEnemyType — defined in FormationData.h
 */

UCLASS(ClassGroup=(Alchemist), meta=(BlueprintSpawnableComponent))
class ALCHEMIST_0_API UEnemyPoolComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UEnemyPoolComponent();
    // --- CONFIGURATION — assign in Details panel on BP_WaveManager ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Pool") TMap<EEnemyType, TSubclassOf<AEnemyBase>> EnemyClasses;// Which Blueprint class to spawn for each enemy type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Pool") TMap<EEnemyType, int32> PoolSizes;    // How many of each type to pre-spawn at level load
    // --- PUBLIC API ---
    // Call from WaveManager::BeginPlay — pre-spawns all actors into the pool
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Pool") void InitializePool();
    // Resets enemy state and returns it to the pool — call from AEnemyBase on death
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Pool") void ReturnToPool(AEnemyBase* Enemy);
   
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Pool") AEnemyBase* GetFromPool(EEnemyType EnemyType); // Returns first inactive enemy of the given type — null if pool exhausted,Caller is responsible for setting position, WaveManagerRef, TargetActors before use
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Alchemist|Pool") int32 GetActiveCount(EEnemyType EnemyType) const; // Returns current active count for a given type — useful for debugging
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Alchemist|Pool") int32 GetPoolSize(EEnemyType EnemyType) const; // Returns pool size for a given type
protected:
    virtual void BeginPlay() override;
private:
    TMap<EEnemyType, TArray<TObjectPtr<AEnemyBase>>> Pool; // One array per enemy type — all actors live here for the lifetime of the level
    void SpawnIntoPool(EEnemyType EnemyType, TSubclassOf<AEnemyBase> EnemyClass, int32 Count); // Spawns a single enemy into the pool at world origin, hidden and inactive
    void Deactivate(AEnemyBase* Enemy); // Deactivates an enemy — hides it, disables collision, disables tick
    void Activate(AEnemyBase* Enemy); // Activates an enemy — shows it, enables collision, enables tick
};