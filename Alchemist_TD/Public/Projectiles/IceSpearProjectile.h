#pragma once

#include "CoreMinimal.h"
#include "Projectiles/ProjectileBase.h"
#include "IceSpearProjectile.generated.h"
/**
 * Ice Spear — the Ice_Freeze modifier (Wave 4 narrative unlock).
 * Modifier rule (D-040): changes TARGET RELATIONSHIP only — base Ice is AOE;
 * the spear strikes the FIRST enemy it contacts, single-target.
 * Damage floor (D-026): still deals base potion damage to that enemy.
 * Identity: applies Freeze to the speared enemy.
 */

UCLASS()
class ALCHEMIST_0_API AIceSpearProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ice Spear")
	float FreezeDuration = 2.0f;

	// VFX/audio hook — fires when the spear strikes an enemy.
	UFUNCTION(BlueprintImplementableEvent, Category = "Ice Spear")
	void OnEnemySpeared(AActor* SpearedEnemy, const FVector& ContactLocation);

protected:
	virtual void HandleEnemyContact(AActor* Enemy, const FVector& ContactLocation) override;
	virtual void HandleWorldHit(const FVector& ImpactLocation) override;
};