#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerResourceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResourcesUpdated);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALCHEMIST_0_API UPlayerResourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerResourceComponent();

	UPROPERTY(BlueprintReadOnly, Category = "Resources") int32 Gold = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Resources") int32 Essence = 0;
	UPROPERTY(BlueprintAssignable, Category = "Resources") FOnResourcesUpdated OnResourcesUpdated;
	UFUNCTION(BlueprintCallable, Category = "Resources") void AddGold(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Resources") void AddEssence(int32 Amount);
};