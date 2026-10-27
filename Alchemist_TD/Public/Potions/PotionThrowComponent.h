#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Potions/PotionTypes.h"
#include "PotionThrowComponent.generated.h"

class UPotionDataAsset;
class UInventoryComponent;

/**
 * UPotionThrowComponent
 *
 * Owns the full throw pipeline:
 *   ExecuteThrow (entry point from Blueprint input)
 *     → resolves potion type from trigger side
 *     → validates inventory
 *     → SpawnPotion (decision layer — resolves class and modifier)
 *       → ExecuteSpawn (execution layer — deferred spawn, initialise, velocity)
 *     → decrements inventory after successful spawn
 *
 * DOES NOT OWN:
 *   Inventory storage — lives on UInventoryComponent on the same owner.
 *   Potion data — lives on UPotionDataAsset, mapped in PotionDataAssets.
 *   VFX/audio — lives in Blueprint child projectile classes.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALCHEMIST_0_API UPotionThrowComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPotionThrowComponent();
	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// DATA — configure in Blueprint on the character
	// -------------------------------------------------------------------------
	// Which potion is equipped on each trigger side.
	UPROPERTY(BlueprintReadOnly, Category = "Potion") TMap<EPotionAssignment, EPotionType> EquippedPotions;
	// One DataAsset per potion type. Set in Blueprint defaults on the character.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion") TMap<EPotionType, TObjectPtr<UPotionDataAsset>> PotionDataAssets;

	// Active modifier per potion type. Updated by dpad input from Blueprint. NOTE: Defaults to None for all types — base potion behaviour.
	UPROPERTY(BlueprintReadWrite, Category = "Potion") TMap<EPotionType, EModifierType> ActiveModifiers;

	// Favourite modifier per potion (set in out-of-combat prep). NOTE: ActiveModifiers (above) is the CURRENT/equipped modifier used on throw.
	UPROPERTY(BlueprintReadWrite, Category = "Potion") TMap<EPotionType, EModifierType> FavouriteModifiers;
	// Cached at BeginPlay from owner. Do not set manually.
	UPROPERTY(Transient) TObjectPtr<UInventoryComponent> CachedInventory = nullptr;
	// Stat counter — incremented on every successful throw. Read by GameMode at run end.
	UPROPERTY(BlueprintReadOnly, Category = "Potion") int32 PotionsThrownThisRun = 0;

	
	// -------------------------------------------------------------------------
	// PUBLIC API — called from Blueprint input events
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Potion") void ExecuteThrow(EPotionAssignment TriggerSide, EPotionType LeftTriggerPotion, EPotionType RightTriggerPotion); // Entry point. Called from Blueprint on trigger press. Resolves potion type, checks inventory, spawns projectile, decrements inventory.
	UFUNCTION(BlueprintCallable, Category = "Potion") void SetTriggerPotion(EPotionAssignment TriggerSide, EPotionType PotionType); // Equip a potion onto a trigger. Sets its CurrentModifier to its Favourite (or Base).
	UFUNCTION(BlueprintCallable, Category = "Potion") void SetCurrentModifier(EPotionType PotionType, EModifierType Modifier); // Set a potion's CURRENT (sticky) modifier. Used by modifier-swap. Favourite untouched.
	UFUNCTION(BlueprintCallable, Category = "Potion") void SetFavouriteModifier(EPotionType PotionType, EModifierType Modifier);// Set a potion's FAVOURITE (prep) and apply it as Current immediately.
	// --- HUD / overlay getters ---
	UFUNCTION(BlueprintPure, Category = "Potion") EModifierType GetCurrentModifier(EPotionType PotionType) const;
	UFUNCTION(BlueprintPure, Category = "Potion") EModifierType GetFavouriteModifier(EPotionType PotionType) const;
	UFUNCTION(BlueprintPure, Category = "Potion") EPotionType GetEquippedPotion(EPotionAssignment TriggerSide) const;
	
	// -------------------------------------------------------------------------
	// BLUEPRINT UTILITIES — pure helpers exposed for HUD and UI
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Potion") EThrowBehaviourType GetResolvedThrowBehaviour(EModifierType CurrentModifier, UPotionDataAsset* CurrentDataAsset) const; // Returns the resolved throw behaviour for the current modifier — used by Blueprint for handler routing.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Potion") int32 GetModifierIndex(EModifierType ModifierType) const; // Returns modifier array index (0/1/2) for a given modifier type. -1 = no modifier.

	
	
private:

	// -------------------------------------------------------------------------
	// INTERNAL PIPELINE
	// -------------------------------------------------------------------------

	// Decision layer — resolves projectile class from DataAsset and active modifier.
	// Calls ExecuteSpawn with resolved data.
	void SpawnPotion(EPotionType PotionType,EPotionAssignment TriggerSide);

	// Execution layer — deferred spawn, initialise with DataAsset, apply velocity.
	void ExecuteSpawn(UPotionDataAsset* DataAsset, TSubclassOf<AActor> ProjectileClass, EPotionAssignment TriggerSide);

	// Maps EPotionType to ItemID FName for inventory lookup.
	FName GetItemIDForPotionType(EPotionType PotionType) const;
};