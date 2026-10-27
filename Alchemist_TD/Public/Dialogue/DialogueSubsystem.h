#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DialogueSubsystem.generated.h"

class UDialogueSequenceAsset;
class UDialogueHUDWidget;
class APlayerController;

/** Fires when a scene started via PlayScene has fully finished. Carries the scene that ended. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueSceneComplete, UDialogueSequenceAsset*, CompletedScene);

/**
 * Generic, game-wide owner of dialogue playback. Survives level loads (GameInstance scope).
 * Owns one WBP_DialogueHUD instance, disables player input during a scene, and reports completion.
 * Knows nothing about tutorials — any system calls PlayScene and awaits OnSceneComplete.
 */
UCLASS()
class ALCHEMIST_0_API UDialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** Subscribe to learn when the current scene finishes. */
    UPROPERTY(BlueprintAssignable, Category = "Dialogue") FOnDialogueSceneComplete OnSceneComplete;

    /** Create/reuse the HUD widget, disable player input, play the scene, report on finish. */
    UFUNCTION(BlueprintCallable, Category = "Dialogue") void PlayScene(UDialogueSequenceAsset* SceneToPlay);

    /** Tell the subsystem which widget class to spawn. Call once (e.g. GameMode BeginPlay). */
    UFUNCTION(BlueprintCallable, Category = "Dialogue") void SetDialogueHUDWidgetClass(TSubclassOf<UDialogueHUDWidget> WidgetClass);
    UFUNCTION(BlueprintPure, Category = "Dialogue") bool IsPlayingScene() const { return bIsPlayingScene; }

private:
    UFUNCTION()
    void HandleSequenceComplete();

    UDialogueHUDWidget* ResolveDialogueHUDWidget();
    APlayerController* GetLocalPlayerController() const;
    void SetPlayerInputEnabled(bool bShouldEnableInput);

    UPROPERTY(Transient) TObjectPtr<UDialogueHUDWidget> DialogueHUDWidget;
    UPROPERTY(Transient) TSubclassOf<UDialogueHUDWidget> DialogueHUDWidgetClass;
    UPROPERTY(Transient) TObjectPtr<UDialogueSequenceAsset> ActiveScene;

    bool bIsPlayingScene = false;
};