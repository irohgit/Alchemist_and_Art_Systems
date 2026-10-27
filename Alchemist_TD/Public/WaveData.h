#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Core/FormationData.h"
#include "WaveData.generated.h"



// -----------------------------------------------------------------------------
// FEnemySpawnEntry
// One batch in a wave. Points to a named formation in UFormationLibrary.
// SpawnInterval drives time between individual enemy spawns within the batch.
// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave")
    EFormationName Formation = EFormationName::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave")
    float SpawnInterval = 0.05f;
};

// -----------------------------------------------------------------------------
// FWaveConfig
// One row in the WaveDataTable — defines a full wave.
// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FWaveConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") int32 WaveNumber = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") TArray<FEnemySpawnEntry> SpawnEntries;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") float TimeBetweenBatches = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") bool bIsBossWave = false;
};