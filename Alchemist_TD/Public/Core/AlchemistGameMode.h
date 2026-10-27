#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AlchemistGameMode.generated.h"

class AWaveManager;
class APhaseManager;
class AVillagerDefenderManager;

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	LaneDefence         UMETA(DisplayName = "Lane Defence"),
	WaveComplete        UMETA(DisplayName = "Wave Complete"),
	BetweenWaves        UMETA(DisplayName = "Between Waves"),
	VillageExploration  UMETA(DisplayName = "Village Exploration"),
	RunComplete         UMETA(DisplayName = "Run Complete")
};

UENUM(BlueprintType)
enum class ERunOutcome : uint8
{
	Victory  UMETA(DisplayName = "Victory"),
	Defeat   UMETA(DisplayName = "Defeat")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunEnded, ERunOutcome, Outcome);

UCLASS()
class ALCHEMIST_0_API AAlchemistGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AAlchemistGameMode();
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Run") float RunStartTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Run") float RunEndTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Run") ERunOutcome RunOutcome = ERunOutcome::Victory;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Run") bool bRunEnded = false;
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Run") FOnRunEnded OnRunEnded;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Wave") EWaveState CurrentWaveState;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Wave") int32 CurrentWaveIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Wave") float BetweenWaveDelay = 3.f;
	UPROPERTY(EditDefaultsOnly, Category = "Alchemist|Run") TSubclassOf<UUserWidget> RunCompleteWidgetClass;
	
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Wave") void StartNextWave();
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Run") void HandleRunEnded(ERunOutcome Outcome);
	UFUNCTION() void OnGameOverHandler();
protected:
	virtual void BeginPlay() override;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist") TObjectPtr<AWaveManager> WaveManager;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist") TObjectPtr<APhaseManager> PhaseManager;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist") TObjectPtr<AVillagerDefenderManager> VillagerDefenderManager;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist") TObjectPtr<class UUserWidget> RunCompleteWidget;
	
private:
	FTimerHandle BetweenWaveTimer;
	void ShowRunCompleteScreen();
	void BeginBetweenWaveSequence(bool bOmeletteSaved);
	
	UFUNCTION() void OnCapturerSpawned();
	UFUNCTION() void OnCapturerDied();
	UFUNCTION() void OnWaveCompleted(int32 WaveNumber);
	UFUNCTION() void OnAllWavesCompleted();
	
};