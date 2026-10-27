#include "Player/AlchemistPlayer.h"
#include "Characters/Omelette.h"

AAlchemistPlayer::AAlchemistPlayer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAlchemistPlayer::PickUpOmelette(AOmelette* Omelette)
{
	if (!IsValid(Omelette)) return;
	if (bIsCarryingOmelette) return;

	CarriedOmelette = Omelette;
	bIsCarryingOmelette = true;

	Omelette->OnPickedUp();

	USkeletalMeshComponent* PlayerMesh = FindComponentByClass<USkeletalMeshComponent>();
	if (!PlayerMesh) return;

	Omelette->AttachToComponent(PlayerMesh,FAttachmentTransformRules::SnapToTargetNotIncludingScale,OmeletteCarrySocketName);
	UE_LOG(LogTemp, Error, TEXT("Omelette Pickedup"));

	OnOmelettePickedUp();           // Blueprint hook — VFX, animation etc
	OnPlayerPickedUpOmelette.Broadcast(); // Systems delegate — PhaseManager reacts
}

void AAlchemistPlayer::DropOmelette()
{
	if (!bIsCarryingOmelette) return;
	if (!IsValid(CarriedOmelette)) return;

	CarriedOmelette->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CarriedOmelette->OnDropped();
	CarriedOmelette = nullptr;
	bIsCarryingOmelette = false;

	OnOmeletteDropped();
}