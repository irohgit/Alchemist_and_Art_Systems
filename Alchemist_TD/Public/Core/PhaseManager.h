#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhaseManager.generated.h"

class AVillagerDefender;
class AVillagerDefenderManager;
class ACombatManager;
class AOmelette;
class AOmeletteGoal;
class ACapturer;
class AEnemyBase;

// -----------------------------------------------------------------------------
// EGamePhase
// Drives all phase-dependent logic across the game.
// -----------------------------------------------------------------------------
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	LaneDefence		UMETA(DisplayName = "Lane Defence"),
	VillagerDefence	UMETA(DisplayName = "Villager Defence"),
	OmeletteCapture	UMETA(DisplayName = "Omelette Capture")
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCapturerSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCapturerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOmeletteSavedSignal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverSignal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBarricadeDestroyed, FVector, BarricadeLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EGamePhase, NewPhase);

UCLASS()
class ALCHEMIST_0_API APhaseManager : public AActor
{
	GENERATED_BODY()

public:

	APhaseManager();


	// DELEGATES — bind in Blueprint for UI and VFX
	
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Phase") FOnBarricadeDestroyed OnBarricadeDestroyed;
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Phase") FOnCapturerSpawned OnCapturerSpawned;
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Phase") FOnCapturerDied OnCapturerDied;
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Phase") FOnGameOverSignal OnGameOverSignal;
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Phase") FOnPhaseChanged OnPhaseChanged;
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Phase") FOnOmeletteSavedSignal OnOmeletteSavedSignal;
	

	// CONFIGURATION — assign in level via Details panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Phase") TObjectPtr<AOmelette> Omelette; // Omelette Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Phase") TObjectPtr<AOmeletteGoal> OmeletteGoal; // Omelette Goal Box 

	// STATE — read only from Blueprint
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Phase") EGamePhase CurrentPhase = EGamePhase::LaneDefence;
	UPROPERTY(BlueprintReadWrite, Category = "Alchemist|Phase") TObjectPtr<ACapturer> ActiveCapturer = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Phase") TObjectPtr<AEnemyBase> ActiveRetriever = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Phase") bool bOmeletteSaved = false;

	// Reference to villager-defender manager — looked up in BeginPlay via GetActorOfClass. Exposed BlueprintReadOnly so BPs can route villager queries through it.
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Phase") TObjectPtr<AVillagerDefenderManager> VillagerDefenderManagerRef = nullptr;
	// Reference to Combat Manager - looked up in BeginPlay via GetActorOfClass.
	UPROPERTY(BlueprintReadOnly, Category="Alchemist|Phase") TObjectPtr<ACombatManager> CombatManagerRef = nullptr;

	// PUBLIC API
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Phase") float OmeletteRetrieveQueryRadius = 3000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Phase") TArray<TObjectPtr<AActor>> OmelettePhaseBarricades;
	
	UFUNCTION() void OnPlayerPickedUpOmeletteHandler();
	UFUNCTION() void OnOmeletteDroppedHandler(AOmelette* DroppedOmelette);
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Phase") void OnActiveRetrieverLost(AEnemyBase* DyingRetriever);
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Phase") void TriggerOmeletteCapture(); // Called when all villagers die (VillagerDefenderManagerRef->bAllVillagersDeadForRun)
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Phase") void TriggerGameOver();
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Phase") void ResetToLaneDefence();
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Phase") void HandleCapturerKilled();
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Phase") void OmelettePhaseBarricadeDestroy();
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Phase") void ForceAutoSaveOmelette();

protected:

	virtual void BeginPlay() override;

	// BLUEPRINT HOOKS — implement for VFX/audio/UI on phase enter
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Phase") void OnEnterVillagerDefence();
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Phase") void OnExitVillagerDefence();
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Phase") void OnEnterOmeletteCapture();
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Phase") void OnOmeletteSaved();
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Phase") void OnGameOver();
private:
	// Subscribed to CombatManager delegates — translate zone activity into phase transitions
	UFUNCTION() void HandleEnemyEnterGateAttackZone();
	UFUNCTION() void HandleGateAttackZoneEmpty();
	UFUNCTION() void HandleOmeletteReturned();

	
	// PHASE TRANSITIONS
	void EnterVillagerDefence();
	void ExitVillagerDefence();
	
	// HELPERS
	AEnemyBase* FindClosestEnemy(FVector Location, float Radius, AActor* ActorToIgnore) const;
	
};