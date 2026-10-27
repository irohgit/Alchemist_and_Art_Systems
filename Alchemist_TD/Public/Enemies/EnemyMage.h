#pragma once
#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Enemies/RangedAttackComponent.h"
#include "EnemyMage.generated.h"

UCLASS()
class ALCHEMIST_0_API AEnemyMage : public AEnemyBase
{
	GENERATED_BODY()

public:
	AEnemyMage();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Mage") TObjectPtr<URangedAttackComponent> RangedAttackComponent;

protected:
	virtual void OnWaypointReached_Implementation(int32 WaypointIndex) override;
};