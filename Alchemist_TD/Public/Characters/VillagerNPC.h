#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VillagerNPC.generated.h"

/**
 * AVillagerNPC
 *
 * Base class for all villager characters.
 * Used in both Village Hub (exploration, dialogue, quests)
 * and Combat Scene (gate defenders).
 *
 * OWNS:
 *   Identity, persistent health, alive state for run.
 *   AI controller support — set AIControllerClass in Blueprint child.
 *   Relationship state (future — village quest system).
 *
 * DOES NOT OWN:
 *   Combat slot logic — lives on AVillagerDefender.
 *   Village quest logic — will be added in village phase.
 *   Mesh and skeleton — assigned in Blueprint child per villager.
 *
 * PERSISTENCE:
 *   Health and bIsAliveForRun persist across scenes via SaveGame system (future).
 *   Blueprint children set starting Health in their defaults.
 */
UCLASS()
class ALCHEMIST_0_API AVillagerNPC : public ACharacter
{
	GENERATED_BODY()

public:

	AVillagerNPC();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villager") FString VillagerName = TEXT("Villager");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villager") FName VillagerID = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villager") TObjectPtr<UTexture2D> Portrait = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villager") float Health = 100.f; 	// HEALTH — persistent across run
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Villager") float MaxHealth = 100.f;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Villager") bool bIsAliveForRun = true;
	
	// BLUEPRINT HOOKS
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Villager") void OnVillagerTookDamage(float DamageAmount); 	// Called when villager takes damage — implement VFX and audio in Blueprint
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Villager") void OnVillagerDied();// Called when villager dies — implement death sequence in Blueprint

	//Obsolete 
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Villager") void OnSelectedAsDefenceTarget();

protected:

	virtual void BeginPlay() override;
};