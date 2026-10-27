#include "Characters/VillagerNPC.h"

AVillagerNPC::AVillagerNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	// Mesh and skeleton assigned in Blueprint child per villager —
	// each villager has a unique skeleton so we do not set it here.

	// AI controller class set in Blueprint child defaults.
	// AutoPossessAI set to PlacedInWorldOrSpawned in Blueprint child.
}

void AVillagerNPC::BeginPlay()
{
	Super::BeginPlay();
}