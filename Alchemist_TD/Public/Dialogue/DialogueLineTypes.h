#pragma once

#include "CoreMinimal.h"
#include "DialogueLineTypes.generated.h"

class UDialogueCharacterProfile;
class UTexture2D;
class USoundBase;

UENUM(BlueprintType)
enum class EDialogueRevealPace : uint8
{
	VerySlow    UMETA(DisplayName = "Very Slow (0.065)"),
	Slow        UMETA(DisplayName = "Slow (0.050)"),
	Normal      UMETA(DisplayName = "Normal (0.035)"),
	Fast        UMETA(DisplayName = "Fast (0.020)"),
	VeryFast    UMETA(DisplayName = "Very Fast (0.010)")
};
inline float GetDialogueRevealSecondsPerCharacter(EDialogueRevealPace RevealPace)
{
	switch (RevealPace)
	{
	case EDialogueRevealPace::VerySlow: return 0.065f;
	case EDialogueRevealPace::Slow:     return 0.050f;
	case EDialogueRevealPace::Fast:     return 0.020f;
	case EDialogueRevealPace::VeryFast: return 0.010f;
	case EDialogueRevealPace::Normal:
	default:                            return 0.035f;
	}
}
UENUM(BlueprintType)
enum class EDialogueLayoutMode : uint8
{
	TwoPortraitConversation     UMETA(DisplayName = "Two-Portrait Conversation"),
	SingleCharacterDramatic     UMETA(DisplayName = "Single Character Dramatic"),
	SpecialMoment               UMETA(DisplayName = "Special Moment")
};

UENUM(BlueprintType)
enum class EDialoguePlaybackState : uint8
{
	Idle,
	TypewriterRevealing,
	AwaitingPlayerAdvance
};

USTRUCT(BlueprintType)
struct ALCHEMIST_0_API FDialogueLine
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") EDialogueRevealPace RevealPace = EDialogueRevealPace::Normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") bool bIsSkippable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") FName SpeakerLeftExpressionKey = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") FName SpeakerRightExpressionKey = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (EditCondition = "LayoutMode == EDialogueLayoutMode::SpecialMoment")) FName DramaticPortraitKey = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") EDialogueLayoutMode LayoutMode = EDialogueLayoutMode::TwoPortraitConversation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") TObjectPtr<USoundBase> VoiceOverSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") TObjectPtr<UDialogueCharacterProfile> SpeakingCharacterProfile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue") TObjectPtr<UTexture2D> DialogueBackground;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = true)) FText SpokenLineText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (EditCondition = "LayoutMode == EDialogueLayoutMode::TwoPortraitConversation")) TObjectPtr<UDialogueCharacterProfile> ListeningCharacterProfile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (EditCondition = "LayoutMode == EDialogueLayoutMode::SpecialMoment")) TObjectPtr<UTexture2D> SharedIllustrationTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (EditCondition = "LayoutMode == EDialogueLayoutMode::SpecialMoment")) FText SharedIllustrationLabel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (EditCondition = "LayoutMode == EDialogueLayoutMode::SpecialMoment")) float SharedIllustrationPortraitScale = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (EditCondition = "LayoutMode == EDialogueLayoutMode::SingleCharacterDramatic")) TObjectPtr<UTexture2D> SingleCharacterDramaticTexture;
};