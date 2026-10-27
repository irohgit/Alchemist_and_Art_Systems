#include "Utilities/ManagedTimerComponent.h"

UManagedTimerComponent::UManagedTimerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UManagedTimerComponent::SetManagedTimer(FTimerHandle& Handle,
	TFunction<void()> Callback, float Rate, bool bLoop, float FirstDelay)
{
	GetWorld()->GetTimerManager().SetTimer(
	   Handle,
	   [Callback]() { Callback(); },
	   Rate,
	   bLoop,
	   FirstDelay
   );
	RegisteredTimers.AddUnique(&Handle);
	RegisteredTimers.AddUnique(&Handle);
}

void UManagedTimerComponent::ClearManagedTimer(FTimerHandle& Handle)
{
	GetWorld()->GetTimerManager().ClearTimer(Handle);
	RegisteredTimers.Remove(&Handle);
}

void UManagedTimerComponent::ClearAllTimers()
{
	for (FTimerHandle* Handle : RegisteredTimers)
	{
		if (Handle)
			GetWorld()->GetTimerManager().ClearTimer(*Handle);
	}
	RegisteredTimers.Empty();
}

void UManagedTimerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAllTimers();
	Super::EndPlay(EndPlayReason);
}