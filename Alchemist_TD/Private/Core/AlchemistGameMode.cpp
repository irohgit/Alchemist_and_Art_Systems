#include "Core/AlchemistGameMode.h"
#include "WaveManager.h"
#include "Core/PhaseManager.h"
#include "Core/VillagerDefenderManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Player/AlchemistPlayer.h"

AAlchemistGameMode::AAlchemistGameMode()
{
    CurrentWaveState = EWaveState::LaneDefence;
    CurrentWaveIndex = 0;
}

void AAlchemistGameMode::BeginPlay()
{
    Super::BeginPlay();
    RunStartTime = GetWorld()->GetTimeSeconds();
    // Find managers placed in the level
    WaveManager = Cast<AWaveManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaveManager::StaticClass()));
    PhaseManager = Cast<APhaseManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APhaseManager::StaticClass()));
    VillagerDefenderManager = Cast<AVillagerDefenderManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AVillagerDefenderManager::StaticClass()));
    if (!WaveManager)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("GameMode: WaveManager not found in level"));
        return;
    }
    if (!PhaseManager)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("GameMode: PhaseManager not found in level"));
        return;
    }
    if (!VillagerDefenderManager)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("GameMode: VillagerDefenderManager not found in level"));
    }

    // Bind to Manager delegates
    if (PhaseManager && WaveManager)
    {
        WaveManager->OnWaveComplete.AddDynamic(this, &AAlchemistGameMode::OnWaveCompleted);
        WaveManager->OnAllWavesComplete.AddDynamic(this, &AAlchemistGameMode::OnAllWavesCompleted);
        PhaseManager->OnCapturerSpawned.AddDynamic(this, &AAlchemistGameMode::OnCapturerSpawned);
        PhaseManager->OnCapturerDied.AddDynamic(this, &AAlchemistGameMode::OnCapturerDied);
        PhaseManager->OnGameOverSignal.AddDynamic(this, &AAlchemistGameMode::OnGameOverHandler);
    }
    if (AAlchemistPlayer* Player = Cast<AAlchemistPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
    {
        Player->OnPlayerPickedUpOmelette.AddDynamic(PhaseManager, &APhaseManager::OnPlayerPickedUpOmeletteHandler);
    }
    // Start wave 1
    StartNextWave();
}
void AAlchemistGameMode::BeginBetweenWaveSequence(bool bOmeletteSaved)
{
    CurrentWaveState = EWaveState::WaveComplete;

    if (bOmeletteSaved && VillagerDefenderManager)
    {
        VillagerDefenderManager->ReviveRandomVillager();
    }

    GetWorldTimerManager().SetTimer(
        BetweenWaveTimer,
        [this]()
        {
            if (!IsValid(this)) return;
            CurrentWaveState = EWaveState::BetweenWaves;
            StartNextWave();
        },
        BetweenWaveDelay,
        false);
}
void AAlchemistGameMode::OnAllWavesCompleted()
{
    CurrentWaveState = EWaveState::VillageExploration;
    // TODO: Open village level
    HandleRunEnded(ERunOutcome::Victory);
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("All waves complete — village phase"));
}
void AAlchemistGameMode::OnCapturerDied()
{
    if (WaveManager) WaveManager->NotifyCapturerDied();
}
void AAlchemistGameMode::OnCapturerSpawned()
{
    if (WaveManager) WaveManager->NotifyCapturerSpawned();
}
void AAlchemistGameMode::OnGameOverHandler()
{
    HandleRunEnded(ERunOutcome::Defeat);
}
void AAlchemistGameMode::OnWaveCompleted(int32 WaveNumber)
{
    if (bRunEnded) return;
    bool bOmeletteSaved = PhaseManager ? PhaseManager->bOmeletteSaved : false;
    BeginBetweenWaveSequence(bOmeletteSaved);
}
void AAlchemistGameMode::HandleRunEnded(ERunOutcome Outcome)
{
    if (bRunEnded) return;                  // idempotent — guard against double-trigger
    bRunEnded = true;
    RunOutcome = Outcome;
    RunEndTime = GetWorld()->GetTimeSeconds();
    CurrentWaveState = EWaveState::RunComplete;
    OnRunEnded.Broadcast(Outcome);
    UGameplayStatics::SetGamePaused(GetWorld(), true);
    ShowRunCompleteScreen();
}
void AAlchemistGameMode::ShowRunCompleteScreen()
{
    if (!RunCompleteWidgetClass)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
            TEXT("GameMode: RunCompleteWidgetClass not assigned on BP_AlchemistGameMode"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    RunCompleteWidget = CreateWidget<UUserWidget>(PC, RunCompleteWidgetClass);
    if (!RunCompleteWidget) return;

    RunCompleteWidget->AddToViewport();
    PC->bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(RunCompleteWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(InputMode);
}
void AAlchemistGameMode::StartNextWave()
{
    UE_LOG(LogTemp, Warning, TEXT("GameMode::StartNextWave called — CurrentWaveIndex=%d"), CurrentWaveIndex);
    if (!WaveManager) return;
    if (bRunEnded) return;
    CurrentWaveState = EWaveState::LaneDefence;
    CurrentWaveIndex++;
    if (IsValid(PhaseManager))
    {
        PhaseManager->ResetToLaneDefence();
    }
    WaveManager->StartNextWave();
}