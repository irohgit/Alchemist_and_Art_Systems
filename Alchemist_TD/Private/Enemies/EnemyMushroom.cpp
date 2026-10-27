#include "Enemies/EnemyMushroom.h"

AEnemyMushroom::AEnemyMushroom()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyMushroom::OnWaypointReached_Implementation(int32 WaypointIndex)
{
	Super::OnWaypointReached_Implementation(WaypointIndex);

	if (WaypointIndex >= SporeThresholdIndex)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green,
	FString::Printf(TEXT("Spore Planted - Waypoint %d"), WaypointIndex));
		}

	}
}