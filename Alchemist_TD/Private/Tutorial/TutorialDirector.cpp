#include "Tutorial/TutorialDirector.h"
#include "Tutorial/TutorialTypes.h"
#include "Interactables/LevelEventTrigger.h"
#include "Dialogue/DialogueSubsystem.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

ATutorialDirector::ATutorialDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}
void ATutorialDirector::BeginPlay()
{
    Super::BeginPlay();

    DialogueSubsystem = GetDialogueSubsystem();
    if (DialogueSubsystem)
    {
        DialogueSubsystem->OnSceneComplete.AddDynamic(this, &ATutorialDirector::HandleSceneComplete);
    }

    // Bind every level-event trigger in the world. We route by EventId in HandleEventTriggered.
    TArray<AActor*> Triggers;
    UGameplayStatics::GetAllActorsOfClass(this, ALevelEventTrigger::StaticClass(), Triggers);
    for (AActor* TriggerActor : Triggers)
    {
        if (ALevelEventTrigger* Trigger = Cast<ALevelEventTrigger>(TriggerActor))
        {
            Trigger->OnTriggered.AddDynamic(this, &ATutorialDirector::HandleEventTriggered);
        }
    }
}
void ATutorialDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(LoadHoldTimerHandle);
    if (DialogueSubsystem)
    {
        DialogueSubsystem->OnSceneComplete.RemoveDynamic(this, &ATutorialDirector::HandleSceneComplete);
    }
    Super::EndPlay(EndPlayReason);
}

int32 ATutorialDirector::FindPathwayIndexByEventId(FName EventId) const
{
    for (int32 Index = 0; Index < Pathways.Num(); ++Index)
    {
        if (Pathways[Index].EventId == EventId) return Index;
    }
    return INDEX_NONE;
}

void ATutorialDirector::BeginLoadIn()
{
    if (!Pathways.IsValidIndex(ActivePathwayIndex)) return;
    const FTutorialPathwayConfig& Pathway = Pathways[ActivePathwayIndex];

    State = ETutorialDirectorState::LoadingIn;
    bMinHoldElapsed = false;
    bStreamLoaded = false;

    ShowLoadingScreen();

    // Minimum hold so the fade always covers >= LoadingScreenMinDuration.
    GetWorldTimerManager().SetTimer(LoadHoldTimerHandle, this,
        &ATutorialDirector::HandleMinHoldElapsed, LoadingScreenMinDuration, false);

    // Async stream-in. Reveal waits for BOTH this callback and the min-hold timer.
    bool bStreamRequested = false;
    ActiveStream = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
        this, Pathway.Sublevel, FVector::ZeroVector, FRotator::ZeroRotator, bStreamRequested);

    if (ActiveStream && bStreamRequested)
    {
        ActiveStream->OnLevelLoaded.AddDynamic(this, &ATutorialDirector::HandleSublevelLoaded);
    }
    else
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            TEXT("TutorialDirector: failed to request sublevel stream"));
        // Don't hard-block: let the min-hold timer fall through with bStreamLoaded forced true.
        bStreamLoaded = true;
    }
}
void ATutorialDirector::BeginLoadOut()
{
    State = ETutorialDirectorState::LoadingOut;

    const FTutorialPathwayConfig& Pathway = Pathways[ActivePathwayIndex];

    ShowLoadingScreen();
    TeleportPlayerToTag(Pathway.TutorialPlayerReturnLocationTag);

    if (ActiveStream)
    {
        // 5.7: request async unload of the dynamic instance.
        ActiveStream->SetShouldBeVisible(false);
        ActiveStream->SetShouldBeLoaded(false);
        ActiveStream = nullptr;
    }

    // Hold the screen for the same fade duration on the way back out.
    GetWorldTimerManager().SetTimer(LoadHoldTimerHandle, this,
        &ATutorialDirector::HandleLoadOutHoldElapsed, LoadingScreenMinDuration, false);
}
void ATutorialDirector::FinishActivePathway()
{
    if (Pathways.IsValidIndex(ActivePathwayIndex))
    {
        const FTutorialPathwayConfig& Pathway = Pathways[ActivePathwayIndex];
        CompletedTriplets.Add(Pathway.Triplet);

        if (ALevelEventTrigger* Trigger = ActiveTrigger.Get())
        {
            if (Trigger->bConsumeOnComplete) Trigger->ConsumeTrigger();
        }
    }

    ActivePathwayIndex = INDEX_NONE;
    ActiveTrigger = nullptr;
    State = ETutorialDirectorState::Idle;

    if (CompletedTriplets.Num() >= Pathways.Num() && Pathways.Num() > 0)
    {
        OnAllTripletsComplete.Broadcast();
    }
}
void ATutorialDirector::HandleEventTriggered(ALevelEventTrigger* Trigger, AActor* /*TriggerInstigator*/)
{
    if (State != ETutorialDirectorState::Idle) return;   // already running a pathway
    if (!Trigger) return;

    const int32 PathwayIndex = FindPathwayIndexByEventId(Trigger->EventId);
    if (PathwayIndex == INDEX_NONE) return;              // not a pathway we manage

    if (CompletedTriplets.Contains(Pathways[PathwayIndex].Triplet)) return;  // done-set guard

    ActivePathwayIndex = PathwayIndex;
    ActiveTrigger = Trigger;
    BeginLoadIn();
}
void ATutorialDirector::HandleLoadOutHoldElapsed()
{
    HideLoadingScreen();
    FinishActivePathway();
}
void ATutorialDirector::HandleMinHoldElapsed()
{
    bMinHoldElapsed = true;
    TryEnterIntro();
}
void ATutorialDirector::HandleSceneComplete(UDialogueSequenceAsset* /*CompletedScene*/)
{
    switch (State)
    {
    case ETutorialDirectorState::IntroCutscene:
        State = ETutorialDirectorState::Gameplay;
        OnTutorialGameplayBegin(Pathways[ActivePathwayIndex].Triplet);
        break;

    case ETutorialDirectorState::OutroCutscene:
        BeginLoadOut();
        break;

    default:
        break;   // a scene we don't own (e.g. unrelated dialogue) — ignore
    }
}
void ATutorialDirector::HandleSublevelLoaded()
{
    bStreamLoaded = true;
    TryEnterIntro();
}
void ATutorialDirector::HideLoadingScreen()
{
    if (LoadingScreenWidget) LoadingScreenWidget->SetVisibility(ESlateVisibility::Collapsed);
}
void ATutorialDirector::NotifyTutorialComplete(ETriplet Triplet)
{
    if (State != ETutorialDirectorState::Gameplay) return;
    if (!Pathways.IsValidIndex(ActivePathwayIndex)) return;
    if (Pathways[ActivePathwayIndex].Triplet != Triplet) return;   // mismatched signal — ignore

    UDialogueSequenceAsset* PostScene = Pathways[ActivePathwayIndex].PostScene;
    if (PostScene && DialogueSubsystem)
    {
        State = ETutorialDirectorState::OutroCutscene;
        DialogueSubsystem->PlayScene(PostScene);
    }
    else
    {
        BeginLoadOut();   // no outro authored — go straight to return + unload
    }
}
void ATutorialDirector::ShowLoadingScreen()
{
    if (!LoadingScreenWidgetClass) return;

    if (!LoadingScreenWidget)
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
        if (!PlayerController) return;
        LoadingScreenWidget = CreateWidget<UUserWidget>(PlayerController, LoadingScreenWidgetClass);
    }

    if (LoadingScreenWidget && !LoadingScreenWidget->IsInViewport())
    {
        LoadingScreenWidget->AddToViewport(1000);   // high Z so it covers the HUD
    }
    if (LoadingScreenWidget) LoadingScreenWidget->SetVisibility(ESlateVisibility::Visible);
}
void ATutorialDirector::TeleportPlayerToTag(FName SpotTag)
{
    if (SpotTag.IsNone()) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn) return;

    TArray<AActor*> Spots;
    UGameplayStatics::GetAllActorsWithTag(this, SpotTag, Spots);
    if (Spots.Num() == 0 || !Spots[0]) return;

    PlayerPawn->SetActorLocationAndRotation(Spots[0]->GetActorLocation(), Spots[0]->GetActorRotation());
}
void ATutorialDirector::TryEnterIntro()
{
    if (State != ETutorialDirectorState::LoadingIn) return;
    if (!bMinHoldElapsed || !bStreamLoaded) return;      // wait for the slower of the two

    const FTutorialPathwayConfig& Pathway = Pathways[ActivePathwayIndex];

    TeleportPlayerToTag(Pathway.TutorialPlayerStartLocationTag);
    HideLoadingScreen();

    UDialogueSequenceAsset* IntroScene = Pathway.IntroScene;
    if (IntroScene && DialogueSubsystem)
    {
        State = ETutorialDirectorState::IntroCutscene;
        DialogueSubsystem->PlayScene(IntroScene);
    }
    else
    {
        State = ETutorialDirectorState::Gameplay;
        OnTutorialGameplayBegin(Pathway.Triplet);
    }
}

UDialogueSubsystem* ATutorialDirector::GetDialogueSubsystem() const
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        return GameInstance->GetSubsystem<UDialogueSubsystem>();
    }
    return nullptr;
}