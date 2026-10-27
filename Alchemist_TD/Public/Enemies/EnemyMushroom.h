#pragma once
#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "EnemyMushroom.generated.h"

UCLASS()
class ALCHEMIST_0_API AEnemyMushroom : public AEnemyBase
{
	GENERATED_BODY()

public:
	AEnemyMushroom();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mushroom")
	int32 SporeThresholdIndex = 7;

protected:
	virtual void OnWaypointReached_Implementation(int32 WaypointIndex) override;
};