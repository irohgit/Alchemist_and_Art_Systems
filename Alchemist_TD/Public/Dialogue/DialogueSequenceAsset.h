#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Dialogue/DialogueLineTypes.h"
#include "DialogueSequenceAsset.generated.h"

class UDialogueCharacterProfile;

UCLASS (BlueprintType)
class ALCHEMIST_0_API UDialogueSequenceAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue") TObjectPtr<UDialogueCharacterProfile> LeftCharacter;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue") TObjectPtr<UDialogueCharacterProfile> RightCharacter;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue") TArray<FDialogueLine> Lines;
	
};