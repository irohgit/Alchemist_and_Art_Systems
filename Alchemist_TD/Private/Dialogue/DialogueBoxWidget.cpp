#include "Dialogue/DialogueBoxWidget.h"
#include "Components/TextBlock.h" 
#include "TimerManager.h"

bool UDialogueBoxWidget::IsTypewriterRevealComplete() const
{
    return RevealedCharacterCount >= FullDialogueLineText.ToString().Len();
}

void UDialogueBoxWidget::CompleteTypewriterReveal()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TypewriterRevealTimerHandle);
    }
    RevealedCharacterCount = FullDialogueLineText.ToString().Len();
    if (DialogueText)
    {
        DialogueText->SetText(FullDialogueLineText);
    }
    bIsPromptActive = true;
}
void UDialogueBoxWidget::RevealNextTypewriterCharacter()
{
    ++RevealedCharacterCount;
    const FString FullLineString = FullDialogueLineText.ToString();
    const int32 TotalCharacterCount = FullLineString.Len();
    const int32 ClampedRevealCount = FMath::Min(RevealedCharacterCount, TotalCharacterCount);
    if (DialogueText)
    {
        DialogueText->SetText(FText::FromString(FullLineString.Left(ClampedRevealCount)));
    }
    if (RevealedCharacterCount >= TotalCharacterCount)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(TypewriterRevealTimerHandle);
        }
        bIsPromptActive = true;
    }
}
void UDialogueBoxWidget::StartTypewriterReveal(const FText& DialogueLineText, EDialogueRevealPace RevealPace)
{
    // Cancel any reveal already in progress so lines never overlap.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TypewriterRevealTimerHandle);
    }
    
    FullDialogueLineText = DialogueLineText;
    RevealedCharacterCount = 0;
    bIsPromptActive = false;
    if (DialogueText)
    {
        DialogueText->SetText(FText::GetEmpty());
    }
    // Typewriter off → show the whole line immediately and bail.
    if (!bIsTypewriterEnabled)
    {
        CompleteTypewriterReveal();
        return;
    }
    const float TypeWriterSecondsPerCharacter = GetDialogueRevealSecondsPerCharacter(RevealPace);
    if (UWorld* World = GetWorld())
    {
        
        World->GetTimerManager().SetTimer(TypewriterRevealTimerHandle,this,&UDialogueBoxWidget::RevealNextTypewriterCharacter,TypeWriterSecondsPerCharacter,true);
    }
}

void UDialogueBoxWidget::NativeDestruct()
{
    // Clear the pending reveal so the timer can't fire into a torn-down widget (CLAUDE.md rule).
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TypewriterRevealTimerHandle);
    }
    Super::NativeDestruct();
}