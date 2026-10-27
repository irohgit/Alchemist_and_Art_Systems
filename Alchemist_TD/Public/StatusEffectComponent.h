#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusEffect.h"
#include "StatusEffectComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusApplied, EStatusType, StatusType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusExpired, EStatusType, StatusType);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALCHEMIST_0_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UStatusEffectComponent();
	UFUNCTION(BlueprintCallable) void ApplyStatus(FStatusEffect NewStatus);
	UFUNCTION(BlueprintCallable) void RemoveStatus(EStatusType StatusType);
	UFUNCTION(BlueprintPure) bool HasStatus(EStatusType StatusType) const;
	UFUNCTION(BlueprintPure) bool IsMovementLocked() const; // Returns true if enemy movement is hard locked
	UFUNCTION(BlueprintPure) bool IsFeared() const; // Returns true if enemy should move backward
	UFUNCTION(BlueprintPure) float GetMovementMultiplier() const; //Accounts for all active movement statuses
	UFUNCTION(BlueprintPure) FStatusEffect GetStatus(EStatusType StatusType) const;
	UPROPERTY(BlueprintAssignable) FOnStatusApplied OnStatusApplied;
	UPROPERTY(BlueprintAssignable) FOnStatusExpired OnStatusExpired;
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	// Active statuses - max one per EStatusType
	// Persistent statuses have no duration
	TMap<EStatusType, FStatusEffect> ActiveStatuses;
	// Queue for movement statuses waiting behind Freeze
	TArray<FStatusEffect> MovementQueue;

	void TickDurations(float DeltaTime);
	void ProcessMovementQueue();
	//void ApplyMovementStatus(FStatusEffect& Status);
	void HandleIgniteTick(float DeltaTime);

	// Ignite Dot Accumulator
	float IgniteTickAccumulator = 0.0f;
	float IgniteTickRate = 0.5f; // ie damage every 0.5 seconds
	
};