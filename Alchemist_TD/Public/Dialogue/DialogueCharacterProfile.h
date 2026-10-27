#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueCharacterProfile.generated.h"

class UTexture2D;

UCLASS(BlueprintType)
class ALCHEMIST_0_API UDialogueCharacterProfile : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character") FName CharacterInternalID = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character") FText CharacterDisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character") TObjectPtr<UTexture2D> DefaultPortraitTexture;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character") TObjectPtr<UTexture2D> DramaticModePortraitTexture;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portraits") TMap<FName, TObjectPtr<UTexture2D>> ExpressionPortraitTextures;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portraits") TMap<FName, TObjectPtr<UTexture2D>> DramaticPortraitTextures;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style") FLinearColor CharacterThemeColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style") float PortraitHeightScale = 1.0f;
};