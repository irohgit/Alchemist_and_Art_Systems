#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundProjectileBase.generated.h"

UCLASS()
class ALCHEMIST_0_API AGroundProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AGroundProjectileBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Projectile")
	bool bIsHoming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Projectile")
	float HomingProximityThreshold = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Projectile")
	float ProjectileSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Projectile")
	AActor* PlayerRef = nullptr;

protected:
	virtual void BeginPlay() override;
};