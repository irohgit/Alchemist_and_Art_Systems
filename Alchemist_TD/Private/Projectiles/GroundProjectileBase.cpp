#include "Projectiles/GroundProjectileBase.h"
#include "Kismet/GameplayStatics.h"

AGroundProjectileBase::AGroundProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGroundProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (!PlayerRef)
	{
		PlayerRef = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}
}