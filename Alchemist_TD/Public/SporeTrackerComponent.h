#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SporeTrackerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSporeThresholdReached, int32, WaveNumber, int32, SporeCount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALCHEMIST_0_API USporeTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USporeTrackerComponent();

	UFUNCTION(BlueprintCallable, Category = "Alchemist|Spore")
	void AddSporeEntry(int32 WaveNumber);

	UFUNCTION(BlueprintPure, Category = "Alchemist|Spore")
	int32 GetSporeCount(int32 WaveNumber) const;

	UFUNCTION(BlueprintCallable, Category = "Alchemist|Spore")
	void ResetLedger();

	UPROPERTY(BlueprintAssignable)
	FOnSporeThresholdReached OnSporeThresholdReached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Spore")
	int32 SporeThreshold = 3;

private:
	TMap<int32, int32> SporeLedger;
};