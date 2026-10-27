#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveData.h" 
#include "SporeTrackerComponent.h"
#include "Core/FormationData.h"

#include "WaveManager.generated.h" //Final Include OK

/*
 * AWaveManager
 *
 * (1) - OWNERSHIP 
 * Owns the full wave lifecycle. Loads FWaveConfig from WaveDataTable, spawns
 * enemy batches at timed intervals using FEnemyPlacement data (class + offset
 * per unit), and broadcasts delegate events on wave start, wave complete, and
 * all-waves complete.
 * (2) - PLACEMENT
 * One instance in the level. Wired to BP_SpawnPoint, TargetWaypointActors,
 * and EscapeWaypointActors via editor properties.
 * (3) - PUBLIC API
 * StartNextWave()   — Advances to the next wave and begins spawning.
 * NotifyEnemyDied() — Called by each enemy on death. Triggers wave complete
 *                     when ActiveEnemyCount reaches zero.
 * (4) - DELEGATES
 * OnWaveStarted(WaveNumber)   — Fires when a new wave begins.
 * OnWaveComplete(WaveNumber)  — Fires when all enemies in the wave are dead.
 * OnAllWavesComplete          — Fires when the final wave is cleared.
 * (5) - DEPENDENCIES
 * WaveDataTable (FWaveConfig rows)
 * AEnemyBase subclasses
 * USporeTrackerComponent
 */

class UEnemyPoolComponent;
class APhaseManager;
class AEnemyBase;
class APhaseManager;
enum class EGamePhase : uint8;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveComplete, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesComplete);

UCLASS()
class ALCHEMIST_0_API AWaveManager : public AActor
{
    GENERATED_BODY()

public:
    AWaveManager();

    // --- DELEGATES ---
    UPROPERTY(BlueprintAssignable) FOnAllWavesComplete OnAllWavesComplete;
    UPROPERTY(BlueprintAssignable) FOnWaveStarted OnWaveStarted;
    UPROPERTY(BlueprintAssignable) FOnWaveComplete OnWaveComplete;

    // --- STATE ---
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Wave") int32 ActiveEnemyCount = 0;
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Wave") int32 CurrentWaveNumber = 0;
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Wave") int32 TotalWaveEnemyCount = 0;
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Wave") int32 EnemiesKilledThisRun = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alchemist|Wave") int32 TotalWaves = 0;

    
    // --- CONFIGURATION ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") TObjectPtr<AActor> SpawnPoint;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") TObjectPtr<APhaseManager> PhaseManagerRef;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") TObjectPtr<UDataTable> WaveDataTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") TObjectPtr<UFormationLibrary> FormationLibrary;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") TArray<TObjectPtr<AActor>> WaypointActors;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") TArray<TObjectPtr<AActor>> EscapeWaypointActors;
    
    // --- COMPONENTS ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Wave") TObjectPtr<UEnemyPoolComponent> EnemyPool;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Wave") TObjectPtr<USporeTrackerComponent> SporeTracker;

    // --- PUBLIC API ---
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Wave") void NotifyEnemyDied();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Wave") void StartNextWave();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Wave") void NotifyCapturerSpawned();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Wave") void NotifyCapturerDied();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alchemist|Wave") float OmeletteAutoSaveDelay = 7.0f;
protected:
    virtual void BeginPlay() override;

private:
    bool bCapturerActive = false;
    bool bWaveCompletePendingCapturerDeath = false;
    bool bWaveCompletePendingOmeletteSave = false;
    bool bHasValidWaveConfig = false;
    bool bAllBatchesSpawned = false;
    bool bBatchComplete = false;
    int32 CurrentBatchIndex = 0;
    int32 CurrentBatchSpawnCount = 0;
    FTimerHandle AutoSaveTimer;
    FTimerHandle BatchDelayTimer;
    FTimerHandle BatchAdvanceTimer;
    FWaveConfig CurrentWaveConfig;
    TArray<TWeakObjectPtr<AEnemyBase>> CurrentBatchEnemies;
    void SpawnBatch(int32 BatchIndex);
    void SpawnSingleEnemy(EEnemyType EnemyType, FVector2D FormationOffset);
    
    
    void TryCompleteOrSuspendForOmelette();
    
    UFUNCTION() void HandleOmeletteSaved();
    UFUNCTION() void ForceOmeletteAutoSaveCallback();
};