#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

//Enums
UENUM(BlueprintType)
enum class EPickupType : uint8
{
	Gold                    UMETA(DisplayName = "Gold"),
	Essence_Crude           UMETA(DisplayName = "Essence - Crude"),
	Essence_Distilled       UMETA(DisplayName = "Essence - Distilled"),
	Essence_Quintessence    UMETA(DisplayName = "Essence - Quintessence")
};
//Structs
UCLASS()
class ALCHEMIST_0_API  APickupBase : public AActor
{
	GENERATED_BODY()

public:
	APickupBase(); //Constructor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup") EPickupType PickupType = EPickupType::Gold;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup") int32 PickupValue = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Pickup") bool bAutoFly = false;
	UPROPERTY(EditDefaultsOnly, Category = "Pickup") TSubclassOf<AActor> GoldActorClass;
	UPROPERTY(EditDefaultsOnly, Category = "Pickup") FVector2D HUDTargetScreenPosition = FVector2D(100.f, 50.f);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup") TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(EditDefaultsOnly, Category = "Pickup") float SpawnDelay = 0.0f;
	UFUNCTION(BlueprintCallable, Category = "Pickup") void StartFly();
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup") void OnStartFly();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup") TObjectPtr<UStaticMeshComponent> PickupMesh;
	UPROPERTY(EditDefaultsOnly, Category = "Pickup") float FlyDuration = 0.8f;
	UPROPERTY(EditDefaultsOnly, Category = "Pickup") float FlyHeight = 200.f;

private:
	FVector SpawnLocation;
	FVector TargetLocation;
	FTimerHandle FlyDelayTimerHandle; 
	float ElapsedTime = 0.f;
	bool bFlying = false;
};