#pragma once
#include "CoreMinimal.h"
#include "StatusEffect.generated.h"

UENUM(BlueprintType)
enum class EStatusType : uint8
{
	None        UMETA(DisplayName = "None"),
	Freeze      UMETA(DisplayName = "Freeze"),
	Fear        UMETA(DisplayName = "Fear"),
	Slow        UMETA(DisplayName = "Slow"),
	Charge      UMETA(DisplayName = "Charge"),
	Ignite      UMETA(DisplayName = "Ignite")
};

UENUM(BlueprintType)
enum class EStatusPriority : uint8
{
	Movement_Hard   = 0,
	Movement_Soft   = 1,
	Movement_Debuff = 2,
	Persistent      = 3
};

USTRUCT(BlueprintType)
struct FStatusEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) EStatusType StatusType = EStatusType::None;
	UPROPERTY(BlueprintReadWrite) EStatusPriority Priority = EStatusPriority::Persistent;

	UPROPERTY(BlueprintReadWrite) bool bIsPersistent = false;
	UPROPERTY(BlueprintReadWrite) bool bIsQueued = false;

	UPROPERTY(BlueprintReadWrite) float Duration = 0.0f;
	UPROPERTY(BlueprintReadWrite) float Magnitude = 1.0f;
};