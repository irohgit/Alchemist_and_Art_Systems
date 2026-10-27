#include "Dialogue/DialogueHUDWidget.h"

void UDialogueHUDWidget::NotifyDialogueSequenceComplete()
{
	OnDialogueSequenceComplete.Broadcast();
}