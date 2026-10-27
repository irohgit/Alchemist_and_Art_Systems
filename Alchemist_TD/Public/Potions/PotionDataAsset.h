#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Potions/PotionTypes.h"
#include "PotionDataAsset.generated.h"

UCLASS(BlueprintType)
class ALCHEMIST_0_API UPotionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat") int32 BaseDamage = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion") int32 MaxCarryCount = 6;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat") float AOERadius = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat") float PushBackForce = 500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw") float ProjectileVelocity = 1500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw") float ArcHeight = 0.2f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw") EThrowBehaviourType ThrowBehaviourType = EThrowBehaviourType::Standard;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion") EPotionType PotionType = EPotionType::Fire;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throw") TSubclassOf<AActor> BaseProjectileClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifiers") TArray<FPotionModifierData> Modifiers;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion") FString PotionName = "Fire";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion") TObjectPtr<UTexture2D> PotionIcon = nullptr;
};