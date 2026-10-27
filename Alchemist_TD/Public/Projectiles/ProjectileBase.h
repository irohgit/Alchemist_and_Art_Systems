#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class URotatingMovementComponent;
class UPotionDataAsset;

/**
 * AProjectileBase
 *
 * C++ base class for all alchemist potion projectiles.
 * All potion Blueprint projectiles inherit from this — directly or via BP_Proj_Potion.
 *
 * OWNS:
 *   Sphere collision, projectile movement, rotating movement.
 *   AOE damage pipeline on world hit.
 *   Self-destruction after impact.
 *
 * DOES NOT OWN:
 *   Mesh, VFX, audio — those live in Blueprint child classes.
 *   PotionMesh is NOT created here. Add it in BP_Proj_Potion via the
 *   Blueprint Components panel. This avoids C++/Blueprint attachment conflicts.
 *
 * INITIALISATION:
 *   PotionThrowComponent calls InitialiseProjectile via SpawnActorDeferred
 *   before BeginPlay fires. DataAsset must be valid before BeginPlay.
 *
 * DAMAGE PATTERN:
 *   OnSphereHit fires on WorldStatic contact — AOE sphere overlap at impact point.
 *   TakeDamage called on each valid enemy in radius. Enemy manages its own lifecycle.
 *
 * BLUEPRINT HOOKS:
 *   OnProjectileImpact — implement in BP child for VFX and audio on any impact.
 */
UCLASS()
class ALCHEMIST_0_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:

	AProjectileBase();

	// -------------------------------------------------------------------------
	// COMPONENTS — created in C++ constructor
	// Do NOT recreate these in Blueprint.
	// -------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile") TObjectPtr<USphereComponent> SphereCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile") TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile") TObjectPtr<URotatingMovementComponent> RotatingMovement;

	// -------------------------------------------------------------------------
	// DATA — set via InitialiseProjectile before BeginPlay
	// -------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = "Projectile") TObjectPtr<UPotionDataAsset> PotionDataAsset = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Projectile") bool bIsSupercharged = false;

	// -------------------------------------------------------------------------
	// INITIALISATION
	// Called by PotionThrowComponent via SpawnActorDeferred before BeginPlay.
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitialiseProjectile(UPotionDataAsset* InDataAsset, bool bInIsSupercharged = false);

	// -------------------------------------------------------------------------
	// BLUEPRINT HOOK
	// Implement in BP child for VFX and audio.
	// Called after damage is applied and before self-destruction.
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnProjectileImpact(const FVector& ImpactLocation, const TArray<AActor*>& HitActors);


protected:

	virtual void BeginPlay() override;
	// -------------------------------------------------------------------------
	// IMPACT SEAMS — override in subclasses to change targeting.
	// Base behaviour is AOE at the impact point (unchanged).
	// -------------------------------------------------------------------------
	virtual void HandleWorldHit(const FVector& ImpactLocation);
	virtual void HandleEnemyContact(AActor* Enemy, const FVector& ContactLocation);

	// Damage Helpers
	void ApplyDamageToTargets(const TArray<AActor*>& Targets);
	void DetectAndDamage(const FVector& ImpactLocation);
	void DetonateAndDestroy(const FVector& ImpactLocation, const TArray<AActor*>& HitActors);
	TArray<AActor*> CollectValidTargets(const FVector& ImpactLocation) const;

	// Prevents double detonation if multiple overlaps fire the same frame.
	bool bHasExploded = false;

private:

	// Bound to SphereCollision OnComponentHit — fires on WorldStatic contact.
	UFUNCTION()
	void OnSphereHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	// Bound to SphereCollision OnComponentBeginOverlap — fires on Enemy overlap.
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};