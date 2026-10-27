#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tutorial/TutorialTypes.h"
#include "TutorialDirector.generated.h"

class ALevelEventTrigger;
class UDialogueSequenceAsset;
class UDialogueSubsystem;
class UUserWidget;
class ULevelStreamingDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllTripletsComplete);

/** One triplet pathway: its trigger, sublevel, cutscenes, and spawn-point tags. Authored on the BP child. */
USTRUCT(BlueprintType)
struct FTutorialPathwayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathway") ETriplet Triplet = ETriplet::Tito;
    /** Must match the EventId on the placed ALevelEventTrigger for this pathway. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathway") FName EventId;
    /** Sublevel streamed in when this pathway starts (e.g. L_Tito_Tutorial). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathway", meta = (AllowedClasses = "/Script/Engine.World")) TSoftObjectPtr<UWorld> Sublevel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathway") TObjectPtr<UDialogueSequenceAsset> IntroScene;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathway") TObjectPtr<UDialogueSequenceAsset> PostScene;
    /** Tag on a TargetPoint inside the sublevel — where the player is placed for the tutorial. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathway") FName TutorialPlayerStartLocationTag;
    /** Tag on a TargetPoint in the hub — where the player is returned afterwards. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathway") FName TutorialPlayerReturnLocationTag;
};

UENUM()
enum class ETutorialDirectorState : uint8
{
    Idle,
    LoadingIn,
    IntroCutscene,
    Gameplay,
    OutroCutscene,
    LoadingOut
};

/**
 * Brain of the triplet tutorial flow. Binds every ALevelEventTrigger in the level; on a matching
 * overlap it streams the pathway sublevel behind a loading screen, plays the intro cutscene, hands
 * control to the sublevel's win-logic, then on NotifyTutorialComplete plays the outro, returns the
 * player, unloads the sublevel, and marks the triplet done. All three done -> OnAllTripletsComplete.
 * Place a BP child in the hub and author the Pathways array.
 */
UCLASS()
class ALCHEMIST_0_API ATutorialDirector : public AActor
{
    GENERATED_BODY()

public:
    ATutorialDirector();
    /** Called BY the sublevel's win-logic when the player has completed that triplet's tutorial. */
    UFUNCTION(BlueprintCallable, Category = "Tutorial") void NotifyTutorialComplete(ETriplet Triplet);
    /** Fires once all three triplet pathways are complete. GameMode binds this for the next beat. */
    UPROPERTY(BlueprintAssignable, Category = "Tutorial") FOnAllTripletsComplete OnAllTripletsComplete;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Per-triplet authoring. Set on the BP child placed in the hub. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial") TArray<FTutorialPathwayConfig> Pathways;
    /** Full-screen loading/fade widget shown during streaming. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial") TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
    /** Minimum time the loading screen stays up, even if the stream finishes sooner. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial") float LoadingScreenMinDuration = 1.5f;
    /** Implement on the BP child to enable that triplet's gameplay (e.g. activate Tito's AI). */
    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial") void OnTutorialGameplayBegin(ETriplet Triplet);

private:
    // --- flow steps ---
    UFUNCTION()
    void HandleEventTriggered(ALevelEventTrigger* Trigger, AActor* TriggerInstigator);
    void BeginLoadIn();
    UFUNCTION()
    void HandleSublevelLoaded();
    void HandleMinHoldElapsed();
    void TryEnterIntro();
    UFUNCTION()
    void HandleSceneComplete(UDialogueSequenceAsset* CompletedScene);
    void BeginLoadOut();
    void HandleLoadOutHoldElapsed();
    void FinishActivePathway();

    // --- helpers ---
    int32 FindPathwayIndexByEventId(FName EventId) const;
    void TeleportPlayerToTag(FName SpotTag);
    void ShowLoadingScreen();
    void HideLoadingScreen();
    UDialogueSubsystem* GetDialogueSubsystem() const;

    // --- runtime state ---
    UPROPERTY(Transient) TObjectPtr<UDialogueSubsystem> DialogueSubsystem;
    UPROPERTY(Transient) TObjectPtr<UUserWidget> LoadingScreenWidget;
    UPROPERTY(Transient) TObjectPtr<ULevelStreamingDynamic> ActiveStream;
    UPROPERTY(Transient) TWeakObjectPtr<ALevelEventTrigger> ActiveTrigger;
    TSet<ETriplet> CompletedTriplets;
    ETutorialDirectorState State = ETutorialDirectorState::Idle;
    int32 ActivePathwayIndex = INDEX_NONE;
    bool bMinHoldElapsed = false;
    bool bStreamLoaded = false;
    FTimerHandle LoadHoldTimerHandle;
};