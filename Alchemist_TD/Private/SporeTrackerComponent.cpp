#include "SporeTrackerComponent.h"

USporeTrackerComponent::USporeTrackerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USporeTrackerComponent::AddSporeEntry(int32 WaveNumber)
{
	int32& Count = SporeLedger.FindOrAdd(WaveNumber);
	Count++;

	if (Count >= SporeThreshold)
	{
		OnSporeThresholdReached.Broadcast(WaveNumber, Count);
	}
}

int32 USporeTrackerComponent::GetSporeCount(int32 WaveNumber) const
{
	if (const int32* Found = SporeLedger.Find(WaveNumber))
	{
		return *Found;
	}
	return 0;
}

void USporeTrackerComponent::ResetLedger()
{
	SporeLedger.Empty();
}