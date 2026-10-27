#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataAsset.generated.h"

/**
 * EItemCategory
 * 
 * Identifies what kind of item this is. Used by UI and systems
 * to filter, display, and route items correctly.
 * 
 * Never add behaviour per-category inside UInventoryComponent —
 * category is for UI and filtering only.
 */
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Potion              UMETA(DisplayName = "Potion"),
	Essence             UMETA(DisplayName = "Essence"),
	Gold                UMETA(DisplayName = "Gold"),
	RawMaterial         UMETA(DisplayName = "Raw Material"),
	Intermediate        UMETA(DisplayName = "Intermediate"),
	BossMaterial        UMETA(DisplayName = "Boss Material"),
	SpecialThrowable    UMETA(DisplayName = "Special Throwable"),
	VillagerCollectible UMETA(DisplayName = "Villager Collectible")
};

/**
 * UItemDataAsset
 * 
 * One asset per item type. Defines display data and carry limits.
 * Does NOT define behaviour — behaviour lives in the system that uses the item.
 * 
 * Matches the pattern of UPotionDataAsset — one asset, all data driven.
 * 
 * To add a new item:
 *   1. Add its FName constant to ItemIDs.h
 *   2. Create a DA_Item_[Name] asset from this class
 *   3. Set ItemID to match the ItemIDs.h constant exactly
 *   4. Set MaxCarryCount (0 = uncapped, used for Gold)
 */
UCLASS(BlueprintType)
class ALCHEMIST_0_API UItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// Must match the corresponding constant in ItemIDs.h exactly.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	// Display name shown in UI.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FString ItemName;

	// Icon shown in HUD and merchant UI.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> ItemIcon = nullptr;

	// Category used by UI for filtering and display routing.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemCategory Category = EItemCategory::Potion;

	// Maximum stack the player can carry. 0 = uncapped (Gold only).
	// Enforced by UInventoryComponent on AddItem.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 MaxCarryCount = 10;

	// Optional short description for merchant UI and Research Log tooltips.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FString Description;

	// If true, this item survives when the player returns to the village after a run.
	// If false, count resets to zero on ResetRunItems().
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bPersistsToVillage = false;
};
