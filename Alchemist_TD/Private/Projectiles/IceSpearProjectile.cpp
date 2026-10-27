#include "Projectiles/IceSpearProjectile.h"
#include "StatusEffectComponent.h"
#include "StatusEffect.h"

void AIceSpearProjectile::HandleEnemyContact(AActor* Enemy, const FVector& ContactLocation)
{
	if (!IsValid(Enemy))
	{
		DetonateAndDestroy(ContactLocation, TArray<AActor*>());
		return;
	}

	// Single-target only — the first enemy contacted. NOT AOE. Honours the damage floor.
	TArray<AActor*> SingleTarget;
	SingleTarget.Add(Enemy);
	ApplyDamageToTargets(SingleTarget);

	// Freeze the speared enemy.
	if (UStatusEffectComponent* StatusComp = Enemy->FindComponentByClass<UStatusEffectComponent>())
	{
		FStatusEffect Freeze;
		Freeze.StatusType = EStatusType::Freeze;
		Freeze.Priority   = EStatusPriority::Movement_Hard;
		Freeze.Duration   = FreezeDuration;
		Freeze.Magnitude  = 1.0f;
		StatusComp->ApplyStatus(Freeze);
	}

	OnEnemySpeared(Enemy, ContactLocation);
	DetonateAndDestroy(ContactLocation, SingleTarget);
}
void AIceSpearProjectile::HandleWorldHit(const FVector& ImpactLocation)
{
	// Spear missed and struck geometry — no damage, just shatter and clean up.
	DetonateAndDestroy(ImpactLocation, TArray<AActor*>());
}