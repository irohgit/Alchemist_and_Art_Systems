#pragma once
#include "CoreMinimal.h"
#include "MageTypes.generated.h"

UENUM(BlueprintType)
enum class EMageTier : uint8
{
	Grey    UMETA(DisplayName = "Grey"),
	Blue    UMETA(DisplayName = "Blue"),
	Red     UMETA(DisplayName = "Red")
};

UENUM(BlueprintType)
enum class EMageAbility : uint8
{
	CircleAoE       UMETA(DisplayName = "CircleAoE"),
	PillarStrike    UMETA(DisplayName = "PillarStrike"),
	GroundProjectile   UMETA(DisplayName = "GroundProjectile")
};

UENUM(BlueprintType)
enum class EAttackMode : uint8
{
	Cycle   UMETA(DisplayName = "Cycle"),
	Random  UMETA(DisplayName = "Random"),
	Burst   UMETA(DisplayName = "Burst")
};