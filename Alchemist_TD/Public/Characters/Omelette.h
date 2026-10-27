#pragma once
#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Omelette.generated.h"

class ACapturer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOmeletteDropped, AOmelette*, DroppedOmelette);


UCLASS()
class ALCHEMIST_0_API AOmelette : public AActor
{
	GENERATED_BODY()
public:
	AOmelette();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Omelette") TObjectPtr<USkeletalMeshComponent> OmeletteMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Omelette") TObjectPtr<USphereComponent> CaptureTrigger;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Omelette") float CaptureRadius = 150.f;
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Omelette") FOnOmeletteDropped OnOmeletteDropped;
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Omelette") void OnDropped();
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Omelette") void OnPickedUp();
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Omelette") void OnSavedAtGoalVolume(const FVector& SaveLocation);
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Omelette") bool bIsDropped = false;
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnCarrierOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	};