#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue/DialogueLineTypes.h"  
#include "DialogueBoxWidget.generated.h"

class UTextBlock;    

/**
 * Single-line dialogue display with a typewriter reveals
 * Ownership: Reveal behavior
 */
UCLASS()
class ALCHEMIST_0_API UDialogueBoxWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue") void StartTypewriterReveal(const FText& DialogueLineText, EDialogueRevealPace RevealPace);
    UFUNCTION(BlueprintCallable, Category = "Dialogue") void CompleteTypewriterReveal();
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue") bool IsTypewriterRevealComplete() const;

protected:
    virtual void NativeDestruct() override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") bool bIsTypewriterEnabled = true;
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue") bool bIsPromptActive = false;

private:
    void RevealNextTypewriterCharacter();
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> DialogueText;
    UPROPERTY() FText FullDialogueLineText;
    int32 RevealedCharacterCount = 0;
    FTimerHandle TypewriterRevealTimerHandle;
};