#include "Projectiles/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "Potions/PotionDataAsset.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// -------------------------------------------------------------------------
	// SPHERE COLLISION — root component
	// Handles both overlap (enemy) and hit (world geometry).
	// Collision profile "Potion" must be configured in Project Settings: //Mesh to Be Added in Children
	//   Block: WorldStatic
	//   Overlap: Enemy channel
	//   Ignore: Pawn, Potion
	// -------------------------------------------------------------------------
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;
	SphereCollision->SetSphereRadius(12.f);
	SphereCollision->SetCollisionProfileName(TEXT("Potion"));
	SphereCollision->SetNotifyRigidBodyCollision(true);

	SphereCollision->OnComponentHit.AddDynamic(this, &AProjectileBase::OnSphereHit);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnSphereOverlap);

	// -------------------------------------------------------------------------
	// PROJECTILE MOVEMENT
	// InitialSpeed = 0 — velocity set externally by PotionThrowComponent
	// after FinishSpawningActor.
	// -------------------------------------------------------------------------
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.f;

	// -------------------------------------------------------------------------
	// ROTATING MOVEMENT
	// Tumble rates set in Blueprint child class defaults.
	// -------------------------------------------------------------------------
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));

	// NOTE: PotionMesh is NOT created here.
	// Add it in BP_Proj_Potion via the Blueprint Components panel and attach
	// it to SphereCollision there. This avoids C++/Blueprint hierarchy conflicts
	// that prevent the mesh from rendering.
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// Safety net — destroy after 8 seconds if never hits anything.
	// Prevents orphaned actors in Endless Mode.
	// Override InitialLifeSpan in Blueprint child for projectiles with longer lifetimes (e.g. Snowball).
	SetLifeSpan(8.f);

	// TEMP DEBUG
	UE_LOG(LogTemp, Warning, TEXT("ProjectileBase BeginPlay fired on %s at location %s"), 
		*GetName(), 
		*GetActorLocation().ToString());
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 50.f, 16, FColor::Green, false, 8.f);
}

// -----------------------------------------------------------------------------
// ApplyDamageToTargets
// TakeDamage on each target. Double IsValid — earlier hit may have killed a target.
// Projectile never destroys enemies — enemy manages its own lifecycle.
// -----------------------------------------------------------------------------
void AProjectileBase::ApplyDamageToTargets(const TArray<AActor*>& Targets)
{
	if (!IsValid(PotionDataAsset)) return;

	const float DamageAmount = static_cast<float>(PotionDataAsset->BaseDamage);

	for (AActor* Target : Targets)
	{
		if (!IsValid(Target)) continue;

		Target->TakeDamage(
			DamageAmount,
			FDamageEvent(),
			nullptr,
			this
		);
	}
}
void AProjectileBase::InitialiseProjectile(UPotionDataAsset* InDataAsset, bool bInIsSupercharged)
{
	PotionDataAsset = InDataAsset;
	bIsSupercharged = bInIsSupercharged;
}

// -----------------------------------------------------------------------------
// OnSphereHit
// Fires when projectile hits WorldStatic (ground, walls).
// This is the primary detonation trigger for standard throws.
// -----------------------------------------------------------------------------
void AProjectileBase::OnSphereHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("OnSphereHit fired — OtherActor: %s"), *GetNameSafe(OtherActor));
    if (bHasExploded) return;
	bHasExploded = true;
	HandleWorldHit(Hit.ImpactPoint);

	/*
	if (bHasExploded) return;
	bHasExploded = true;

	DetectAndDamage(Hit.ImpactPoint);
	*/
}

// -----------------------------------------------------------------------------
// OnSphereOverlap
// Fires when projectile overlaps an enemy.
// Detonates at projectile location — enemy was inside AOE radius on contact.
// -----------------------------------------------------------------------------
void AProjectileBase::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bHasExploded) return;
	if (!IsValid(OtherActor) || OtherActor == this) return;
	if (!OtherActor->ActorHasTag(TEXT("Enemy"))) return;
	bHasExploded = true;
	HandleEnemyContact(OtherActor, GetActorLocation());
}
// -----------------------------------------------------------------------------
// CollectValidTargets
// Sphere overlap at ImpactLocation using AOERadius from DataAsset.
// Uses ECC_GameTraceChannel1 — must match your Enemy collision channel in
// Project Settings > Collision > Trace Channels.
// -----------------------------------------------------------------------------
TArray<AActor*> AProjectileBase::CollectValidTargets(const FVector& ImpactLocation) const
{
	TArray<AActor*> ValidTargets;

	if (!IsValid(PotionDataAsset)) return ValidTargets;

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(PotionDataAsset->AOERadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ImpactLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		CollisionShape,
		QueryParams
	);

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* Actor = Result.GetActor();
		if (IsValid(Actor) && Actor->ActorHasTag(TEXT("Enemy")))
		{
			ValidTargets.AddUnique(Actor); // AddUnique instead of Add
		}
	}

	return ValidTargets;
}
// -----------------------------------------------------------------------------
// DetectAndDamage
// Full pipeline — collect targets in AOE, apply damage, destroy self.
// -----------------------------------------------------------------------------
void AProjectileBase::DetectAndDamage(const FVector& ImpactLocation)
{
	TArray<AActor*> HitActors = CollectValidTargets(ImpactLocation);
	ApplyDamageToTargets(HitActors);
	DetonateAndDestroy(ImpactLocation, HitActors);
}
// -----------------------------------------------------------------------------
// DetonateAndDestroy
// Disable collision and movement, notify Blueprint for VFX/audio, destroy self.
// -----------------------------------------------------------------------------
void AProjectileBase::DetonateAndDestroy(const FVector& ImpactLocation, const TArray<AActor*>& HitActors)
{
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();

	OnProjectileImpact(ImpactLocation, HitActors);

	Destroy();
}

// Base behaviour 
void AProjectileBase::HandleWorldHit(const FVector& ImpactLocation)
{
	DetectAndDamage(ImpactLocation);
}
void AProjectileBase::HandleEnemyContact(AActor* /*Enemy*/, const FVector& ContactLocation)
{
	DetectAndDamage(ContactLocation);
}




