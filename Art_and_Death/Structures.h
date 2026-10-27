#pragma once

#include "CoreMinimal.h"
#include "Enumerators.h"
#include "Engine/DataTable.h"
#include "Structures.generated.h"

USTRUCT(BlueprintType)
struct FEnemy
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float WalkSpeed = 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxWalkSpeed = 500.0f;
	
};

USTRUCT(BlueprintType)
struct FDialogueLine : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText SpeakerName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DialogueLine;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration;
};