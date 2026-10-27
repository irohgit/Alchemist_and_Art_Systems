#include "Enemies/EnemyMage.h"

AEnemyMage::AEnemyMage()
{
	PrimaryActorTick.bCanEverTick = false;

	RangedAttackComponent = CreateDefaultSubobject<URangedAttackComponent>(
		TEXT("RangedAttackComponent"));
}

void AEnemyMage::OnWaypointReached_Implementation(int32 WaypointIndex)
{
	

	Super::OnWaypointReached_Implementation(WaypointIndex);

	if (!TargetActors.IsValidIndex(WaypointIndex)) return;

	AActor* Waypoint = TargetActors[WaypointIndex];
	if (!Waypoint) return;


	if (Waypoint->ActorHasTag(FName("RangedAttack")))
	{


		if (AAIController* AI = Cast<AAIController>(GetController()))
		{
			bIsStationary = true;
			AI->StopMovement();

		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red,
	TEXT("AIController cast failed"));
			}
		}
		if (RangedAttackComponent)
		{
			RangedAttackComponent->StartAttackCycle();
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red,
	TEXT("RangedAttackComponent null"));
			}

		}
	}
}