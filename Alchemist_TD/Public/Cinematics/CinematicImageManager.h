#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CinematicImageManager.generated.h"

class UAudioComponent;
class UCameraComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USoundBase;
class UStaticMeshComponent;
class UTexture2D;

USTRUCT(BlueprintType) struct FCinematicImageEntry
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic") TObjectPtr<UTexture2D> Image = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic") TObjectPtr<USoundBase> VoiceOver = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic") float DisplayDuration = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic") float ExtraHoldAfterVoiceOver = 0.4f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCinematicComplete);

UCLASS()
class ALCHEMIST_0_API ACinematicImageManager : public AActor
{
	GENERATED_BODY()
public:
	ACinematicImageManager();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cinematic") bool bAutoPlayOnBeginPlay = true;
	
	UPROPERTY(BlueprintAssignable, Category = "Cinematic") FOnCinematicComplete OnCinematicComplete;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cinematic") FName TextureParameterName = TEXT("Image");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cinematic") TArray<FCinematicImageEntry> CinematicImageSequence;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cinematic") TObjectPtr<UMaterialInterface> BaseMaterial;

	UFUNCTION(BlueprintCallable, Category = "Cinematic") void PlayCinematicImageSequence();
	UFUNCTION(BlueprintCallable, Category = "Cinematic") void SkipToEnd();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic") TObjectPtr<UStaticMeshComponent> CinematicImagePlane;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic") TObjectPtr<UCameraComponent> CinematicCamera;

private:
	int32 CurrentIndex = -1;
	FTimerHandle AdvanceTimer; //Member handle - cleared in Endplay
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> PlaneMID;
	UPROPERTY(Transient) TObjectPtr<UAudioComponent> CurrentVoiceOver;

	void AdvanceCinematicImageSequence();
	void ShowCinematicImageEntry(int32 Index);
	void FinishCinematicImageSequence();
	
};