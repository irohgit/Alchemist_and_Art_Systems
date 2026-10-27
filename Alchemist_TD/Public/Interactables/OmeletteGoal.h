#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OmeletteGoal.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOmeletteReturned);
/**
 * AOmeletteStump
 *
 * Level-placed collision volume. When the player walks into the volume while
 * carrying the omelette, broadcasts OnOmeletteReturned. PhaseManager subscribes
 * to flip bOmeletteSaved and trigger the save sequence.
 *
 * Designer notes:
 *   - Drop one of these into the level near the village/safe zone.
 *   - Position the BoxComponent's extent so the player can naturally enter it.
 *   - Assign this actor on BP_PhaseManager's OmeletteStump property.
 *
 * Step 1: skeleton + volume only. Overlap detection and broadcast wired in step 2.
 */

UCLASS()

class ALCHEMIST_0_API AOmeletteGoal : public AActor
{
	GENERATED_BODY()
public:
	AOmeletteGoal();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Alchemist|OmeletteGoal") TObjectPtr<UBoxComponent> OmeletteGoalVolume;
	UPROPERTY(BlueprintAssignable, Category="Alchemist|OmeletteGoal") FOnOmeletteReturned OnOmeletteReturned;
protected:
	virtual void BeginPlay() override;
	UFUNCTION() void OnOmeletteGoalVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
};