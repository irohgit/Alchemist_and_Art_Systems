#include "Dialogue/DialogueSubsystem.h"
#include "Dialogue/DialogueHUDWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"


APlayerController* UDialogueSubsystem::GetLocalPlayerController() const
{
    return UGameplayStatics::GetPlayerController(GetGameInstance(), 0);
}
UDialogueHUDWidget* UDialogueSubsystem::ResolveDialogueHUDWidget()
{
    if (DialogueHUDWidget) return DialogueHUDWidget;
    if (!DialogueHUDWidgetClass) return nullptr;

    APlayerController* PlayerController = GetLocalPlayerController();
    if (!PlayerController) return nullptr;

    DialogueHUDWidget = CreateWidget<UDialogueHUDWidget>(PlayerController, DialogueHUDWidgetClass);
    return DialogueHUDWidget;
}


void UDialogueSubsystem::HandleSequenceComplete()
{
    if (!bIsPlayingScene) return;
    UDialogueSequenceAsset* CompletedScene = ActiveScene;
    bIsPlayingScene = false;
    ActiveScene = nullptr;
    if (DialogueHUDWidget)
    {
        DialogueHUDWidget->OnDialogueSequenceComplete.RemoveDynamic(this, &UDialogueSubsystem::HandleSequenceComplete);
        DialogueHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
    SetPlayerInputEnabled(true);
    OnSceneComplete.Broadcast(CompletedScene);
}
void UDialogueSubsystem::PlayScene(UDialogueSequenceAsset* SceneToPlay)
{
    if (!SceneToPlay)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("PlayScene: null scene"));
        return;
    }
    if (bIsPlayingScene)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("PlayScene: a scene is already playing"));
        return;
    }
    UDialogueHUDWidget* Widget = ResolveDialogueHUDWidget();
    if (!Widget)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red,
            TEXT("PlayScene: no HUD widget. Call SetDialogueHUDWidgetClass first."));
        return;
    }
    ActiveScene = SceneToPlay;
    bIsPlayingScene = true;
    if (!Widget->IsInViewport())
    {
        Widget->AddToViewport();
    }
    Widget->SetVisibility(ESlateVisibility::Visible);
    SetPlayerInputEnabled(false);
    // Re-bind defensively (RemoveDynamic is a no-op if not bound) so we never double-subscribe.
    Widget->OnDialogueSequenceComplete.RemoveDynamic(this, &UDialogueSubsystem::HandleSequenceComplete);
    Widget->OnDialogueSequenceComplete.AddDynamic(this, &UDialogueSubsystem::HandleSequenceComplete);
    Widget->PlayDialogueSequence(SceneToPlay);
}
void UDialogueSubsystem::SetDialogueHUDWidgetClass(TSubclassOf<UDialogueHUDWidget> WidgetClass)
{
    DialogueHUDWidgetClass = WidgetClass;
}
void UDialogueSubsystem::SetPlayerInputEnabled(bool bShouldEnableInput)
{
    APlayerController* PlayerController = GetLocalPlayerController();
    if (!PlayerController) return;
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn) return;
    if (bShouldEnableInput) PlayerPawn->EnableInput(PlayerController);
    else                    PlayerPawn->DisableInput(PlayerController);
}