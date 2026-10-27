#include "Enemies/EnemyCapturer.h"
#include "Characters/Omelette.h"
#include "Core/PhaseManager.h"
#include "Components/CapsuleComponent.h"
#include "Characters/VillagerDefender.h"
#include "Characters/VillagerNPC.h"
#include "WaveManager.h"
#include "Algo/Reverse.h"

/*
 * InitialiseCapturer(); - Reverses the forward waypoint array so capturer walks from spawn to Omelette.Called by Blueprint after spawn via OnEnterOmeletteCapture hook.
 * OnReachBoundary_Implementation(); - Fires when CurrentWaypointIndex >= TargetActors.Num(). Capturer has reached Omelette — pick up.
 * PickUpOmelette(); - Attaches Omelette to socket, begins arc escape movement.
 * BeginEscapeMovement(); - Stores start and end positions for arc lerp. Disables NavMesh movement — Tick drives position directly.
 * Tick - Drives arc escape movement when bIsEscaping is true Arcs up then sags back down — weight gag.
 * OnDeath_Implementation(); - Capturer killed — revive one random dead villager after delay, then reset to Lane Defence.
 */ 
ACapturer::ACapturer()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACapturer::BeginPlay()
{
    Super::BeginPlay();
}
void ACapturer::InitialiseCapturer(const TArray<AActor*>& ApproachWaypoints,const TArray<AActor*>& EscapeWaypoints,AOmelette* Omelette,APhaseManager* PhaseManager)
{

    if (!Omelette || !PhaseManager) return;
    SetCanBeDamaged(false); // In PickUpOmelette Set to True
    OmeletteRef = Omelette;
    PhaseManagerRef = PhaseManager;
    StoredEscapeWaypoints = EscapeWaypoints;

    // Store approach waypoints reversed — used if we need fallback
    TArray<AActor*> Reversed = ApproachWaypoints;
    Algo::Reverse(Reversed);
    TargetActors = Reversed;

    CurrentWaypointIndex = 0;
    bMovementHeld = false;

    // Move directly to Omelette — overlap sphere fires PickUpOmelette on arrival
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalActor(OmeletteRef);
        MoveRequest.SetAcceptanceRadius(80.f);
        FNavPathSharedPtr NavPath;
        AI->MoveTo(MoveRequest, &NavPath);
    }
    // Capturer is fully initialised and active — notify systems
    if (IsValid(PhaseManagerRef))
    {
        PhaseManagerRef->OnCapturerSpawned.Broadcast();
    }
}
void ACapturer::OnReachBoundary_Implementation()
{
    if (bIsDead) return;
    if (bHasOmelette)
    {
        // Reached spawn with Omelette — trigger game over
        if (PhaseManagerRef)
        {
            PhaseManagerRef->TriggerGameOver();
        }
        return;
    }
    PickUpOmelette(OmeletteRef);
}
void ACapturer::PickUpOmelette(AOmelette* Omelette)
{
    if (!Omelette) return;
    if (bHasOmelette) return;

    bHasOmelette = true;
    SetCanBeDamaged(true); // In intialized is false
    OmeletteRef = Omelette;

    // Attach Omelette to socket
    Omelette->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        OmeletteSocketName
    );

    OnOmelettePickedUp();

    // Swap to escape waypoints and head back to spawn
    TargetActors = StoredEscapeWaypoints;
    CurrentWaypointIndex = 0;
    bMovementHeld = false;
    MoveToNextWaypoint();
}
void ACapturer::BeginEscapeMovement()
{
    if (!WaveManagerRef) return;
    if (!WaveManagerRef->SpawnPoint) return;

    EscapeStartLocation = GetActorLocation();
    EscapeEndLocation = WaveManagerRef->SpawnPoint->GetActorLocation();
    EscapeAlpha = 0.f;
    bIsEscaping = true;

    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->StopMovement();
    }

    GetCharacterMovement()->DisableMovement();
    OnEscapeBegin();
}
void ACapturer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
void ACapturer::OnDeath_Implementation()
{
    bIsEscaping = false;
    bIsDead = true;
    if (bHasOmelette && OmeletteRef)
    {
        OmeletteRef->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        OmeletteRef->OnDropped();
        bHasOmelette = false;
    }
    // Notify PhaseManager to handle revive and reset — don't use timer on self
    if (PhaseManagerRef)
    {
        PhaseManagerRef->HandleCapturerKilled();
    }
    GetWorldTimerManager().ClearTimer(ChaseTimer);
    bIsDead = true;
    // Explicit cleanup — actor-level disable wasn't sticking
    SetActorEnableCollision(false);
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    SetActorHiddenInGame(true);   // hides ALL components including mesh
    GetMesh()->SetVisibility(false);  // keep for safety
}
void ACapturer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(ReviveDelayTimer);
    Super::EndPlay(EndPlayReason);
}