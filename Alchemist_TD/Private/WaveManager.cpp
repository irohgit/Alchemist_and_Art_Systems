#include "WaveManager.h"
#include "Enemies/EnemyPoolComponent.h"
#include "Enemies/EnemyBase.h"
#include "Core/PhaseManager.h"
#include "Kismet/GameplayStatics.h"

AWaveManager::AWaveManager()
{
    PrimaryActorTick.bCanEverTick = false;
    SporeTracker = CreateDefaultSubobject<USporeTrackerComponent>(TEXT("SporeTracker"));
    EnemyPool = CreateDefaultSubobject<UEnemyPoolComponent>(TEXT("EnemyPool"));
}
void AWaveManager::BeginPlay()
{
    Super::BeginPlay();
    if (WaveDataTable)
    {
        TotalWaves = WaveDataTable->GetRowMap().Num();
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: TotalWaves derived from %s = %d"), *WaveDataTable->GetName(), TotalWaves);
    }
    else
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("WaveManager: WaveDataTable not assigned — TotalWaves=0"));
    }
    PhaseManagerRef = Cast<APhaseManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APhaseManager::StaticClass()));
    if (!PhaseManagerRef)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("WaveManager: PhaseManager not found in level — wave gating disabled"));
        UE_LOG(LogTemp, Error, TEXT("WaveManager: PhaseManager not found in level"));
    }
    else
    {
        PhaseManagerRef->OnOmeletteSavedSignal.AddDynamic(this, &AWaveManager::HandleOmeletteSaved);
    }
}
void AWaveManager::ForceOmeletteAutoSaveCallback()
{
    UE_LOG(LogTemp, Warning, TEXT("AutoSaveTimer expired — forcing omelette auto-save"));
    if (PhaseManagerRef)
    {
        PhaseManagerRef->ForceAutoSaveOmelette();
        // Auto-save fires OnOmeletteSavedSignal → HandleOmeletteSaved → wave completes
    }
}
void AWaveManager::HandleOmeletteSaved()
{
    // Cancel the auto-save timer if it's running — manual save beat the deadline
    GetWorldTimerManager().ClearTimer(AutoSaveTimer);

    if (bWaveCompletePendingOmeletteSave)
    {
        bWaveCompletePendingOmeletteSave = false;
        UE_LOG(LogTemp, Warning, TEXT("HandleOmeletteSaved: resolving suspended wave complete"));
        TryCompleteOrSuspendForOmelette();
    }
}
void AWaveManager::NotifyCapturerSpawned()
{
    bCapturerActive = true;
}
void AWaveManager::NotifyCapturerDied()
{
    bCapturerActive = false;
    if (bWaveCompletePendingCapturerDeath)
    {
        bWaveCompletePendingCapturerDeath = false;
        UE_LOG(LogTemp, Warning, TEXT("NotifyCapturerDied: resolving suspended wave complete"));
        TryCompleteOrSuspendForOmelette();
    }
}
void AWaveManager::NotifyEnemyDied()
{
    ActiveEnemyCount--;
    EnemiesKilledThisRun++;
    UE_LOG(LogTemp, Warning, TEXT("NotifyEnemyDied: ActiveEnemyCount=%d bAllBatchesSpawned=%d TotalWaveEnemyCount=%d KilledThisRun=%d"), ActiveEnemyCount, (int32)bAllBatchesSpawned, TotalWaveEnemyCount, EnemiesKilledThisRun);
    
    if (ActiveEnemyCount <= 0 && bAllBatchesSpawned && TotalWaveEnemyCount > 0)
    {
        ActiveEnemyCount = 0;
        if (bCapturerActive)
        {
            bWaveCompletePendingCapturerDeath = true;
            UE_LOG(LogTemp, Warning, TEXT("NotifyEnemyDied: Wave complete suspended — Capturer still active"));
            return;
        }
        TryCompleteOrSuspendForOmelette();
    }
}
void AWaveManager::SpawnSingleEnemy(EEnemyType EnemyType, FVector2D FormationOffset)
{
    if (!SpawnPoint) return;
    if (!IsValid(EnemyPool)) { UE_LOG(LogTemp, Error, TEXT("SpawnSingleEnemy: EnemyPool is null — assign UEnemyPoolComponent on BP_WaveManager")); return;}

    AEnemyBase* Enemy = EnemyPool->GetFromPool(EnemyType);
    if (!Enemy) {UE_LOG(LogTemp, Error, TEXT("SpawnSingleEnemy: Pool exhausted for EnemyType=%d — increase PoolSizes"), (int32)EnemyType); return;}
    
    const FVector SpawnOrigin = SpawnPoint->GetActorLocation();
    const FRotator SpawnRotation = SpawnPoint->GetActorRotation();
    const FVector LaneForward = SpawnRotation.Vector();
    const FVector LaneLateral = FRotationMatrix(SpawnRotation).GetScaledAxis(EAxis::Y);

    FVector SpawnLocation = SpawnOrigin + (LaneLateral * FormationOffset.X) + (LaneForward * FormationOffset.Y);

    Enemy->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
    Enemy->LateralOffset = FormationOffset.X;
    Enemy->TargetActors = WaypointActors;
    Enemy->WaveManagerRef = this;
    Enemy->PhaseManagerRef = this->PhaseManagerRef;
    
    // Bind phase change delegate now that PhaseManagerRef is valid
    if (IsValid(Enemy->PhaseManagerRef))
    {
        Enemy->PhaseManagerRef->OnPhaseChanged.AddDynamic(Enemy, &AEnemyBase::HandlePhaseChanged);
        UE_LOG(LogTemp, Warning, TEXT("SpawnSingleEnemy: Bound OnPhaseChanged on %s"), *Enemy->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnSingleEnemy: PhaseManagerRef is null — cannot bind OnPhaseChanged"));
    }
    // Bind status effect delegates — removed on ReturnToPool, re-added here on each spawn
    if (IsValid(Enemy->StatusEffectComponent))
    {
        Enemy->StatusEffectComponent->OnStatusApplied.AddDynamic(Enemy, &AEnemyBase::OnStatusApplied);
        Enemy->StatusEffectComponent->OnStatusExpired.AddDynamic(Enemy, &AEnemyBase::OnStatusExpired);
    }
    Enemy->bMovementHeld = true;
    Enemy->CurrentWaypointIndex = 0;
  
    
    ActiveEnemyCount++;
    TotalWaveEnemyCount++;
    
    UE_LOG(LogTemp, Warning, TEXT("SpawnSingleEnemy: ActiveEnemyCount now=%d"), ActiveEnemyCount);
    CurrentBatchEnemies.Add(TWeakObjectPtr<AEnemyBase>(Enemy));
}
void AWaveManager::SpawnBatch(int32 BatchIndex)
{
    bBatchComplete = false;
    if (!bHasValidWaveConfig) return;
    if (!CurrentWaveConfig.SpawnEntries.IsValidIndex(BatchIndex)) return;
    if (!FormationLibrary)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnBatch: FormationLibrary is null — assign it on BP_WaveManager"));
        return;
    }
    if (!IsValid(EnemyPool))
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnBatch: EnemyPool is null — assign UEnemyPoolComponent on BP_WaveManager"));
        return;
    }

    const FEnemySpawnEntry& Entry = CurrentWaveConfig.SpawnEntries[BatchIndex];

    TObjectPtr<UDataTable>* FoundTable = FormationLibrary->Formations.Find(Entry.Formation);
    if (!FoundTable || !*FoundTable)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnBatch: No DataTable for formation in batch %d"), BatchIndex);
        return;
    }

    TArray<FEnemyPlacement*> RawPlacements;
    (*FoundTable)->GetAllRows<FEnemyPlacement>(TEXT("SpawnBatch"), RawPlacements);
    if (RawPlacements.Num() == 0) return;

    CurrentBatchEnemies.Empty();

    // Activate all placements instantly from pool — no timer needed
    for (FEnemyPlacement* P : RawPlacements)
    {
        if (!P) continue;
        SpawnSingleEnemy(P->EnemyType, FVector2D(P->OffsetX, P->OffsetY));
    }

    bBatchComplete = true;

    FTimerManager& WorldTimerManager = GetWorldTimerManager();
    // Batch 0 has no preceding batch to space from — release immediately. Subsequent batches wait the configured TimeBetweenBatches.
    const float ReleaseDelay = (BatchIndex == 0) ? UE_KINDA_SMALL_NUMBER : CurrentWaveConfig.TimeBetweenBatches;
    WorldTimerManager.SetTimer(BatchDelayTimer,[this, BatchIndex]()
        {
            if (!IsValid(this)) return;

            // Capture current batch before anything else
            TArray<TWeakObjectPtr<AEnemyBase>> BatchCopy = CurrentBatchEnemies;
            CurrentBatchEnemies.Empty();
            // Spawn next batch first — it holds while this batch releases
            int32 NextBatch = BatchIndex + 1;
            if (CurrentWaveConfig.SpawnEntries.IsValidIndex(NextBatch))
            {
                CurrentBatchIndex = NextBatch;
                SpawnBatch(NextBatch);
            }
            else
            {
                bAllBatchesSpawned = true;// No next batch — all enemies now releasing
            }
            // Release this batch after next is spawned and held
            for (auto& EnemyPtr : BatchCopy)
            {
                if (EnemyPtr.IsValid())
                {
                    EnemyPtr->bMovementHeld = false;
                    EnemyPtr->MoveToNextWaypoint();
                }
            }
        },ReleaseDelay,false);
}
void AWaveManager::StartNextWave()
{
    UE_LOG(LogTemp, Warning, TEXT("WaveManager::StartNextWave called — CurrentWaveNumber=%d"), CurrentWaveNumber);

    bAllBatchesSpawned = false;
    if (!WaveDataTable) return;
    
    CurrentWaveNumber++;
    CurrentBatchIndex = 0;
    ActiveEnemyCount = 0;
    TotalWaveEnemyCount = 0;
    
    FString RowName = FString::Printf(TEXT("Wave_%d"), CurrentWaveNumber);
    FWaveConfig* FoundConfig = WaveDataTable->FindRow<FWaveConfig>(FName(*RowName), TEXT("WaveManager::StartNextWave"));
    if (!FoundConfig) return;
    CurrentWaveConfig = *FoundConfig;
    bHasValidWaveConfig = true;
    OnWaveStarted.Broadcast(CurrentWaveNumber);
    SpawnBatch(0);
}
void AWaveManager::TryCompleteOrSuspendForOmelette()
{
    // No PhaseManager — fall back to broadcasting normally
    if (!PhaseManagerRef)
    {
        OnWaveComplete.Broadcast(CurrentWaveNumber);
        if (CurrentWaveNumber >= TotalWaves) OnAllWavesComplete.Broadcast();
        return;
    }

    const bool bInOmeletteCapture = (PhaseManagerRef->CurrentPhase == EGamePhase::OmeletteCapture);
    const bool bSaved = PhaseManagerRef->bOmeletteSaved;

    if (bInOmeletteCapture && !bSaved)
    {
        bWaveCompletePendingOmeletteSave = true;
        UE_LOG(LogTemp, Warning, TEXT("TryCompleteOrSuspendForOmelette: suspended — omelette not saved. Starting %.1fs auto-save timer."), OmeletteAutoSaveDelay);
        GetWorldTimerManager().SetTimer(AutoSaveTimer, this, &AWaveManager::ForceOmeletteAutoSaveCallback, OmeletteAutoSaveDelay, false);
        return;
    }

    // Conditions met — broadcast wave complete
    OnWaveComplete.Broadcast(CurrentWaveNumber);
    if (CurrentWaveNumber >= TotalWaves) OnAllWavesComplete.Broadcast();
}

