#include "Enemies/RangedAttackComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Enemies/EnemyBase.h"

URangedAttackComponent::URangedAttackComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URangedAttackComponent::BeginPlay()
{
    Super::BeginPlay();
    BuildAbilitySet();
    BurstCountRemaining = BurstCount;

    if (!PlayerActor)
    {
        PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }
}

void URangedAttackComponent::BuildAbilitySet()
{
    AvailableAbilities.Empty();
    AvailableAbilities.Add(EMageAbility::CircleAoE);

    if (MageTier == EMageTier::Blue || MageTier == EMageTier::Red)
    {
        AvailableAbilities.Add(EMageAbility::PillarStrike);

    }

    if (MageTier == EMageTier::Red)
    {
        AvailableAbilities.Add(EMageAbility::GroundProjectile);
    }

}

void URangedAttackComponent::StartAttackCycle()
{
    if (bIsAttacking) return;
    bIsAttacking = true;
    BurstCountRemaining = BurstCount;
    ExecuteNextAbility();
}

void URangedAttackComponent::StopAttackCycle()
{
    bIsAttacking = false;
    GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(WindUpTimerHandle);
}

void URangedAttackComponent::SetAttackMode(EAttackMode NewMode)
{
    AttackMode = NewMode;
}

void URangedAttackComponent::SetMageTier(EMageTier NewTier)
{
    MageTier = NewTier;
    BuildAbilitySet();
}

EMageAbility URangedAttackComponent::SelectNextAbility()
{
    if (AvailableAbilities.Num() == 0)
        return EMageAbility::CircleAoE;

    switch (AttackMode)
    {
    case EAttackMode::Cycle:
        {
            EMageAbility Next = AvailableAbilities[CurrentAbilityIndex];
            CurrentAbilityIndex = (CurrentAbilityIndex + 1) % AvailableAbilities.Num();
            return Next;
        }
    case EAttackMode::Random:
        {
            int32 RandomIndex = FMath::RandRange(0, AvailableAbilities.Num() - 1);
            return AvailableAbilities[RandomIndex];
        }
    case EAttackMode::Burst:
        {
            if (BurstCountRemaining == BurstCount)
            {
                int32 RandomIndex = FMath::RandRange(0, AvailableAbilities.Num() - 1);
                return AvailableAbilities[RandomIndex];
            }

            float RepeatRoll = FMath::FRandRange(0.0f, 1.0f);
            float ChanceToUse = GetRepeatChanceForAbility(LastFiredAbility);

            if (RepeatRoll <= ChanceToUse)
            {
                return LastFiredAbility;
            }

            int32 RandomIndex = FMath::RandRange(0, AvailableAbilities.Num() - 1);
            return AvailableAbilities[RandomIndex];
        }
    default:
        return EMageAbility::CircleAoE;
    }
}

void URangedAttackComponent::ExecuteNextAbility()
{
    if (!bIsAttacking) return;
    EMageAbility NextAbility = SelectNextAbility();
    LastFiredAbility = NextAbility;
    StartWindUp(NextAbility);
}

void URangedAttackComponent::StartWindUp(EMageAbility Ability)
{
    // Lock player elevation state at wind-up start
    if (Ability == EMageAbility::GroundProjectile)
    {
        AEnemyBase* OwnerEnemy = GetOwnerAsEnemy();
        if (OwnerEnemy && PlayerActor)
        {
            FVector PlayerLoc = GetPlayerLocation();
            FVector GroundAtPlayer = OwnerEnemy->GetGroundLocationAtActor(PlayerActor);
            bPlayerElevatedAtWindUp = (PlayerLoc.Z - GroundAtPlayer.Z) > GroundProjectileElevationThreshold;
        }
    }

    float WindUpDuration = GetWindUpForAbility(Ability);

    GetWorld()->GetTimerManager().SetTimer(
        WindUpTimerHandle,
        [this, Ability]()
        {
            FireAbility(Ability);
        },
        WindUpDuration,
        false);
}

void URangedAttackComponent::FireAbility(EMageAbility Ability)
{
    switch (Ability)
    {
    case EMageAbility::CircleAoE:
        CastCircleAoE();
        break;
    case EMageAbility::PillarStrike:
        CastPillarStrike();
        break;
    case EMageAbility::GroundProjectile:
        CastGroundProjectile();
        break;
    }

    float NextDelay = 0.0f;

    if (AttackMode == EAttackMode::Burst)
    {
        BurstCountRemaining--;

        if (BurstCountRemaining > 0)
        {
            NextDelay = BurstDelay;
        }
        else
        {
            NextDelay = GetCooldownForAbility(Ability);
            BurstCountRemaining = BurstCount;
        }
    }
    else
    {
        NextDelay = GetCooldownForAbility(Ability);
    }

    GetWorld()->GetTimerManager().SetTimer(
        AttackTimerHandle,
        this,
        &URangedAttackComponent::ExecuteNextAbility,
        NextDelay,
        false);
}

void URangedAttackComponent::CastCircleAoE()
{
    // (1) Null check — player and meteor class must exist
    if (!PlayerActor || !MeteorActorClass) return;

    // (2) Cast to EnemyBase to access ground trace and rotation functions
    AEnemyBase* OwnerEnemy = GetOwnerAsEnemy();
    if (!OwnerEnemy) return;

    // (3) Get ground location at player position via line trace
    FVector GroundLocation = OwnerEnemy->GetGroundLocationAtActor(PlayerActor);

    // (4) Face Mage toward player before casting
    FacePlayer();

    // (5) Spawn meteor actor at ground level at player position
    GetWorld()->SpawnActor<AActor>(MeteorActorClass, GroundLocation, FRotator::ZeroRotator);
}

void URangedAttackComponent::CastPillarStrike()
{
    // (1) Null check — player and pillar class must exist
    if (!PlayerActor || !PillarProjectileClass) return;

    // (2) Cast to EnemyBase to access ground trace and rotation functions
    AEnemyBase* OwnerEnemy = GetOwnerAsEnemy();
    if (!OwnerEnemy) return;

    // (3) Get ground location at player position via line trace
    FVector GroundLocation = OwnerEnemy->GetGroundLocationAtActor(PlayerActor);
    FVector MageLoc = GetOwner()->GetActorLocation();

    // (4) Face Mage toward player before firing
    FacePlayer();

    // (5) Get spawn rotation from shared helper
    FRotator SpawnRotation = GetProjectileSpawnRotation();

    // (6) Spawn at Mage XY position locked to ground Z at player position
    FVector SpawnLocation = FVector(MageLoc.X, MageLoc.Y, GroundLocation.Z);

    // (7) Spawn projectile actor
    GetWorld()->SpawnActor<AActor>(PillarProjectileClass, SpawnLocation, SpawnRotation);

    // (LOG)
    UE_LOG(LogTemp, Warning, TEXT("Pillar Strike — projectile fired"));
}

void URangedAttackComponent::CastGroundProjectile()
{
    // (1) Null check — player and projectile class must exist
    if (!PlayerActor || !GroundProjectileActorClass) return;

    // (2) Cast to EnemyBase to access ground trace and rotation functions
    AEnemyBase* OwnerEnemy = GetOwnerAsEnemy();
    if (!OwnerEnemy) return;

    // (3) Face Mage toward player before firing
    FacePlayer();

    // (4) Get spawn rotation from shared helper
    FRotator SpawnRotation = GetProjectileSpawnRotation();

    // (5) Lock spawn location to ground Z at Mage XY position
    FVector MageLoc = GetOwner()->GetActorLocation();
    FVector MageGround = OwnerEnemy->GetGroundLocationAtActor(GetOwner());
    FVector SpawnLocation = FVector(MageLoc.X, MageLoc.Y, MageGround.Z);

    // (6) Spawn deferred — properties must be set before BeginPlay fires
    AGroundProjectileBase* Projectile = GetWorld()->SpawnActorDeferred<AGroundProjectileBase>(
        GroundProjectileActorClass,
        FTransform(SpawnRotation, SpawnLocation));

    if (!Projectile) return;

    // (7) Set properties using elevation state locked at wind-up start
    Projectile->bIsHoming = !bPlayerElevatedAtWindUp;
    Projectile->HomingProximityThreshold = GroundProjectileHomingProximity;
    Projectile->ProjectileSpeed = GroundProjectileSpeed;
    Projectile->PlayerRef = PlayerActor;

    // (8) Finish spawning — BeginPlay fires here with all properties set
    UGameplayStatics::FinishSpawningActor(
        Projectile,
        FTransform(SpawnRotation, SpawnLocation));
}

FVector URangedAttackComponent::GetPlayerLocation() const
{
    if (PlayerActor)
    {
        return PlayerActor->GetActorLocation();
    }
    return FVector::ZeroVector;
}

float URangedAttackComponent::GetCooldownForAbility(EMageAbility Ability) const
{
    switch (Ability)
    {
    case EMageAbility::CircleAoE:     return CircleCooldown;
    case EMageAbility::PillarStrike:  return PillarCooldown;
    case EMageAbility::GroundProjectile: return GroundProjectileCooldown;
    default:                          return CircleCooldown;
    }
}

float URangedAttackComponent::GetWindUpForAbility(EMageAbility Ability) const
{
    switch (Ability)
    {
    case EMageAbility::CircleAoE:     return CircleWindUp;
    case EMageAbility::PillarStrike:  return PillarWindUp;
    case EMageAbility::GroundProjectile: return GroundProjectileWindUp;
    default:                          return CircleWindUp;
    }
}

float URangedAttackComponent::GetRepeatChanceForAbility(EMageAbility Ability) const
{
    if (Ability == EMageAbility::GroundProjectile)
    {
        return GroundProjectileRepeatChance;
    }
    return RepeatChance;
}

void URangedAttackComponent::FacePlayer()
{
    AEnemyBase* OwnerEnemy = GetOwnerAsEnemy();
    if (!OwnerEnemy || !PlayerActor) return;

    FVector DirectionXY = FVector(
        PlayerActor->GetActorLocation().X - OwnerEnemy->GetActorLocation().X,
        PlayerActor->GetActorLocation().Y - OwnerEnemy->GetActorLocation().Y,
        0.0f).GetSafeNormal();

    OwnerEnemy->SetActorRotation(DirectionXY.Rotation());
}


// Helper Functions
FRotator URangedAttackComponent::GetProjectileSpawnRotation() const
{
    FVector MageLoc = GetOwner()->GetActorLocation();
    FVector PlayerLoc = GetPlayerLocation();

    FVector DirectionXY = FVector(
        PlayerLoc.X - MageLoc.X,
        PlayerLoc.Y - MageLoc.Y,
        0.0f).GetSafeNormal();

    return DirectionXY.Rotation();
}