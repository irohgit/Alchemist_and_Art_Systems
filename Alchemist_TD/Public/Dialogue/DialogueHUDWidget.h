#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueHUDWidget.generated.h"

class UDialogueSequenceAsset;

/** Broadcast by the widget when a sequence has fully finished playing. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueHUDSequenceComplete);

/**
 * C++ base for WBP_DialogueHUD. Reparent the existing widget Blueprint to this.
 * C++ -> BP: PlayDialogueSequence (implement with your existing PlaySequence logic).
 * BP -> C++: call NotifyDialogueSequenceComplete from the BP when the scene ends.
 */
UCLASS(Abstract, Blueprintable)
class ALCHEMIST_0_API UDialogueHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Implemented in WBP_DialogueHUD — kick off playback of this scene. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue") void PlayDialogueSequence(UDialogueSequenceAsset* SequenceToPlay);

	/** WBP_DialogueHUD calls this the moment the whole sequence has finished. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue") void NotifyDialogueSequenceComplete();

	/** The subsystem subscribes to this to learn the scene is over. */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue") FOnDialogueHUDSequenceComplete OnDialogueSequenceComplete;
};