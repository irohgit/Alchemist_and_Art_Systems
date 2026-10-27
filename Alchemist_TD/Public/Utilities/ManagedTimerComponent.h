#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ManagedTimerComponent.generated.h"

UCLASS(ClassGroup=(Alchemist), meta=(BlueprintSpawnableComponent))
class ALCHEMIST_0_API UManagedTimerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UManagedTimerComponent();

	void SetManagedTimer(FTimerHandle& Handle, TFunction<void()> Callback, float Rate, bool bLoop, float FirstDelay);
	void ClearManagedTimer(FTimerHandle& Handle);
	void ClearAllTimers();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	TArray<FTimerHandle*> RegisteredTimers;
};