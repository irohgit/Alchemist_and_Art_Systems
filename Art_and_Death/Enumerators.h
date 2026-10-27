#pragma once

#include "Enumerators.generated.h"

UENUM(BlueprintType)
enum class EFaction : uint8
{
	Friendly UMETA(DisplayName = "Friendly"),
	Enemy UMETA(DisplayName = "Enemy"),
	Neutral UMETA(DisplayName = "Neutral")
};