#include "Player/PlayerResourceComponent.h"

UPlayerResourceComponent::UPlayerResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerResourceComponent::AddGold(int32 Amount)
{
	if (Amount <= 0) return;
	Gold += Amount;
	OnResourcesUpdated.Broadcast();
}

void UPlayerResourceComponent::AddEssence(int32 Amount)
{
	if (Amount <= 0) return;
	Essence += Amount;
	OnResourcesUpdated.Broadcast();
}