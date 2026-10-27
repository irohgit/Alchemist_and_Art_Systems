#include "Enemies/EnemyBase.h"

#include "Characters/Omelette.h"
#include "Core/PhaseManager.h"
#include "Enemies/EnemyPoolComponent.h"
#include "Enemies/RangedAttackComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemIDs.h"
#include "Pickups/PickupBase.h"
#include "WaveManager.h"
#include "Utilities/ManagedTimerComponent.h"

static int32 GetEssenceUnitValue(EEssenceTier Tier)
{
    switch (Tier)
    {
    case EEssenceTier::Distilled: return 5;
    case EEssenceTier::Quintessence:  return 20;
    default:                     return 1;  // Crude
    }
}

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = false;
    Tags.Add(TEXT("Enemy"));
    StatusEffectComponent = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComponent"));
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    GetCharacterMovement()->bUseRVOAvoidance = true;
}
void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &AEnemyBase::HandleWaypointArrival);
    }
}
void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(ChaseTimer);
    GetWorldTimerManager().ClearTimer(FearMovementTimer);
    GetWorldTimerManager().ClearTimer(AttackTimer);
    GetWorldTimerManager().ClearTimer(ReturnTimer);   
    GetWorldTimerManager().ClearTimer(DestroyTimer); 
    Super::EndPlay(EndPlayReason);
}
void AEnemyBase::DropResource()
{
    UE_LOG(LogTemp, Warning, TEXT("DropResource called on %s"), *GetName());

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    UE_LOG(LogTemp, Warning, TEXT("PlayerPawn valid: %s"), PlayerPawn ? TEXT("YES") : TEXT("NO"));

    if (!PlayerPawn) return;

    UInventoryComponent* ResourceComp = PlayerPawn->FindComponentByClass<UInventoryComponent>();
    UE_LOG(LogTemp, Warning, TEXT("ResourceComp valid: %s"), ResourceComp ? TEXT("YES") : TEXT("NO"));

    if (!ResourceComp) return;

    // Gold
    if (GoldDropAmount > 0){ ResourceComp->AddGold(GoldDropAmount);}
    UE_LOG(LogTemp, Warning, TEXT("Gold Class valid: %s"), PickupActorClass_Gold ? TEXT("YES") : TEXT("NO"));

    // Essence — amount × tier value
    if (EssenceDropAmount > 0)
    {
        int32 TierValue = GetEssenceUnitValue(EssenceTier);
        ResourceComp->AddItem(ItemIDs::Essence, EssenceDropAmount * TierValue);
    }
    UE_LOG(LogTemp, Warning, TEXT("Essence Class valid: %s"), PickupActorClass_Essence ? TEXT("YES") : TEXT("NO"));

    // ── Spawn transforms ──────────────────────────
    FVector RandomOffset = FVector(
        FMath::RandRange(-40.f, 40.f),
        FMath::RandRange(-40.f, 40.f),
        0.f
    );

    FTransform GoldTransform(GetActorLocation());
    FTransform EssenceTransform(GetActorLocation() + RandomOffset);
    // Spawn cosmetic pickups
    if (PickupActorClass_Gold)
    {
        APickupBase* GoldPickup = GetWorld()->SpawnActorDeferred<APickupBase>(
            PickupActorClass_Gold, GoldTransform
        );
        if (GoldPickup)
        {
            GoldPickup->PickupType = EPickupType::Gold;
            GoldPickup->PickupValue = GoldDropAmount;
            UGameplayStatics::FinishSpawningActor(GoldPickup, GoldTransform);
            GoldPickup->SpawnDelay = 0.3f;
            GoldPickup->StartFly();
        }
    }
    if (PickupActorClass_Essence)
    {
        APickupBase* EssencePickup = GetWorld()->SpawnActorDeferred<APickupBase>
        (
            PickupActorClass_Essence, EssenceTransform
        );
        if (EssencePickup)
        {
            //Map Tier to pickup type
            switch (EssenceTier)
            {
                case EEssenceTier::Crude: EssencePickup->PickupType = EPickupType::Essence_Crude;        
                    break;
                case EEssenceTier::Distilled: EssencePickup->PickupType = EPickupType::Essence_Distilled;    
                    break;
                case EEssenceTier::Quintessence: EssencePickup->PickupType = EPickupType::Essence_Quintessence; 
                    break;
            }
            EssencePickup->PickupValue = EssenceDropAmount * GetEssenceUnitValue(EssenceTier);
            UGameplayStatics::FinishSpawningActor(EssencePickup,EssenceTransform);
            EssencePickup->SpawnDelay = 0.6f;
            EssencePickup->StartFly();
        }
    }
}
void AEnemyBase::DropOmelette()
{
    if (!IsValid(CarriedOmelette)) return;
    CarriedOmelette->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CarriedOmelette->OnDropped();
    CarriedOmelette = nullptr;
    bIsCarryingOmelette = false;
}
void AEnemyBase::DestroyEnemy()
{
    Destroy();
}
void AEnemyBase::HandleWaypointArrival(FAIRequestID RequestID,const FPathFollowingResult& Result)
{
    if (bIsDead) return;
    //if (ClaimedSlot != nullptr) return; // At attack slot — stop waypoint progression
    if (!Result.IsSuccess()) return;
    if (StatusEffectComponent->IsMovementLocked()) return;
    if (StatusEffectComponent->IsFeared()) return;

    OnWaypointReached(CurrentWaypointIndex);
    CurrentWaypointIndex++;
    if (!bIsStationary)
    {
        MoveToNextWaypoint();
    }
}
void AEnemyBase::HandlePhaseChanged(EGamePhase NewPhase)
{
    UE_LOG(LogTemp, Warning, TEXT("HandlePhaseChanged called on %s — Phase=%d Dead=%d Retrieving=%d Carrying=%d"),*GetName(),(int32)NewPhase,(int32)bIsDead,(int32)bIsRetrievingOmelette,(int32)bIsCarryingOmelette);

    if (NewPhase != EGamePhase::OmeletteCapture) return;
    if (bIsDead) return;
    if (bIsRetrievingOmelette) return;
    if (bIsCarryingOmelette) return;
    StartChasePlayer();
}
void AEnemyBase::MoveToNextWaypoint()
{
    /* This function 
    . (1) CHECKS
    . (2) ASSIGN NEXT WAYPOINT
    . (3) APPLY LATERAL OFFSET TO WAYPOINT POSITION, PRESSERVING FORMATION SHAPE DOWN FULL LANE
    . (4) MOVE ENEMY TO NEXT WAYPOINT  (4.1) SETUP AI MOVE STRUCTURE (4.2) CALL MOVETO WITH AI MOVE STRUCTURE
    */

    // (1)
    if (bMovementHeld) return;
    if (bIsDead) return;
    if (TargetActors.Num() == 0) return;
    if (CurrentWaypointIndex >= TargetActors.Num())
    {
        OnReachBoundary();
        return;
    }

    // (2) 
    AActor* NextWaypoint = TargetActors[CurrentWaypointIndex];
    if (!NextWaypoint) return;
    
    // (3)
    FVector TargetLocation = NextWaypoint->GetActorLocation();
    if (WaveManagerRef && WaveManagerRef->SpawnPoint)
    {
        const FVector LaneLateral = FRotationMatrix(WaveManagerRef->SpawnPoint->GetActorRotation()).GetScaledAxis(EAxis::Y);
        TargetLocation += LaneLateral * LateralOffset;
    }

    // (4) 
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        // (4.1) 
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(TargetLocation);
        MoveRequest.SetAcceptanceRadius(WaypointAcceptanceRadius);

        FNavPathSharedPtr NavPath;
        // (4.1) 
        AI->MoveTo(MoveRequest, &NavPath);
    }
}
void AEnemyBase::OnDeath_Implementation()
{
    GetWorldTimerManager().ClearTimer(ChaseTimer);
    bIsDead = true;
    StopAttacking();
    DropResource();
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->StopMovement();
    }
    if (WaveManagerRef)
    {
        WaveManagerRef->NotifyEnemyDied();
    }
    // If we were the assigned omelette retriever but never picked it up, tell PhaseManager so it can reassign before the omelette sits forever.
    if (bIsRetrievingOmelette && !bIsCarryingOmelette && IsValid(PhaseManagerRef))
    {
        PhaseManagerRef->OnActiveRetrieverLost(this);
    }
    // Defer destroy by one frame so overlap events finish cleanly
    SetActorEnableCollision(false);
    GetMesh()->SetVisibility(false);
    FTimerManager& WorldTimerManager = GetWorldTimerManager();
    if (bIsPooled) // Return to pool after brief delay so overlap events finish cleanly
    {
        WorldTimerManager.SetTimer(ReturnTimer,[this]() {if (!IsValid(this)) return;if (IsValid(WaveManagerRef) && IsValid(WaveManagerRef->EnemyPool)){WaveManagerRef->EnemyPool->ReturnToPool(this);}},0.1f,false);
    }
    else
    {
        WorldTimerManager.SetTimer(DestroyTimer, this,&AEnemyBase::DestroyEnemy,0.1f, false);
    }
}
void AEnemyBase::OnReachBoundary_Implementation()
{
    if (bIsCarryingOmelette)
    {
        if (IsValid(PhaseManagerRef))
        {
            PhaseManagerRef->TriggerGameOver();
        }
        return;
    }
}
void AEnemyBase::OnReturnToPool()
{
    if (bIsCarryingOmelette && IsValid(CarriedOmelette))
    {
        DropOmelette();
    }
    StopAttacking();
}
void AEnemyBase::OnWaypointReached_Implementation(int32 WaypointIndex)
{
    
}
void AEnemyBase::OnStatusApplied(EStatusType StatusType)
{
    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed * StatusEffectComponent->GetMovementMultiplier();

    if (StatusType == EStatusType::Fear)
    {
        GetWorldTimerManager().SetTimer(FearMovementTimer,
        [this]()
        {
            if (!IsValid(this)) return;
            if (bIsDead) return;
            if (!StatusEffectComponent->IsFeared()) return;
            if (CurrentWaypointIndex <= 0) return; // Guard Against Waypoint Index Out of Bounds Access

            AActor* PreviousWaypoint = TargetActors[CurrentWaypointIndex - 1];
            if (IsValid(PreviousWaypoint))
            {
                if (AAIController* AI = Cast<AAIController>(GetController()))
                {
                    AI->MoveToActor(PreviousWaypoint);
                }
            }
        }
        ,0.5f, true);
    }
}
void AEnemyBase::OnStatusExpired(EStatusType StatusType)
{
    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed * StatusEffectComponent->GetMovementMultiplier();

    if (StatusType == EStatusType::Fear)
    {   
        GetWorldTimerManager().ClearTimer(FearMovementTimer);
        MoveToNextWaypoint();
    }
}
void AEnemyBase::PickUpOmelette(AOmelette* Omelette)
{
    UE_LOG(LogTemp, Warning, TEXT("EnemyBase::PickUpOmelette called on %s — bIsRetrievingOmelette=%d"), *GetName(), (int32)bIsRetrievingOmelette);
    UE_LOG(LogTemp, Warning, TEXT("PickUpOmelette called on %s"), *GetName());
    if (!IsValid(Omelette)) return;
    if (CarriedOmelette) return;
    CarriedOmelette = Omelette;
    bIsCarryingOmelette = true;
    Omelette->OnPickedUp();
    Omelette->AttachToComponent(GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,FName("OmeletteSocket"));
    if (IsValid(WaveManagerRef))  // Swap to escape waypoints — head back to spawn
    {TargetActors = WaveManagerRef->EscapeWaypointActors;}
    bIsRetrievingOmelette = true;
    CurrentWaypointIndex = 0;
    bMovementHeld = false;
    MoveToNextWaypoint();
}
void AEnemyBase::StartAttacking()
{
    GetWorldTimerManager().SetTimer(AttackTimer,
        [this]()
        {
         if (!IsValid(this)) return;
         if (bIsDead) return;
         OnEnemyAttacking.Broadcast(MeleeDamage);   
        }
        ,1.0f,true);
}
void AEnemyBase::StartChasePlayer()
{
    // Start repeating chase — refreshes move target as player moves
    FTimerManager& WorldTimerManager = GetWorldTimerManager();
    WorldTimerManager.SetTimer(ChaseTimer, [this]()
    {
        if (!IsValid(this)) return;
        if (bIsDead) return;
        if (bIsRetrievingOmelette) return;
        if (bIsCarryingOmelette) return;

        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (!IsValid(PlayerPawn)) return;

        float AcceptanceRadius = ActorHasTag(FName("Ranged")) ? RangedAcceptanceRadius : MeleeAcceptanceRadius;

        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalActor(PlayerPawn);
            MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
            FNavPathSharedPtr NavPath;
            AI->MoveTo(MoveRequest, &NavPath);
        }
    }, 0.5f, true);
}
void AEnemyBase::StartOmeletteRetrieve(AOmelette* Omelette)
{
    UE_LOG(LogTemp, Warning, TEXT("StartOmeletteRetrieve: %s assigned as retriever"), *GetName());

    if (!IsValid(Omelette)) return;
    if (bIsDead) return;

    bIsRetrievingOmelette = true;
    bIsStationary = false;

    // Move directly to Omelette via NavMesh — bypass waypoint system
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalActor(Omelette);
        MoveRequest.SetAcceptanceRadius(80.f);
        FNavPathSharedPtr NavPath;
        AI->MoveTo(MoveRequest, &NavPath);
    }
}
void AEnemyBase::StopAttacking()
{
    GetWorldTimerManager().ClearTimer(AttackTimer);
}
void AEnemyBase::TakeHit(int32 HitDamage, bool bIsSupercharged)
{
    if (!CanBeDamaged()) return;
    if (bIsDead) return;

    bLastHitWasSupercharged = bIsSupercharged;
    HitPoints -= HitDamage;
    if (HitPoints <= 0)
    {
        OnDeath();
    }
}
float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,AController* EventInstigator, AActor* DamageCauser)
{
    Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    int32 Damage = FMath::RoundToInt(DamageAmount);
    TakeHit(Damage, false);
    return DamageAmount;
}

//Helpers
FVector AEnemyBase::GetGroundLocationAtActor(AActor* TargetActor) const
{
    if (!TargetActor) return FVector::ZeroVector;

    FVector TraceStart = TargetActor->GetActorLocation();
    FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, 1000.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(TargetActor);

    if (GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd,
        ECC_Visibility, Params))
    {
        return HitResult.ImpactPoint;
    }

    return TraceStart;
}
AEnemyBase* URangedAttackComponent::GetOwnerAsEnemy() const
{
    return Cast<AEnemyBase>(GetOwner());
}



