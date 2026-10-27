#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/ItemDataAsset.h"
#include "InventoryComponent.generated.h"

/**
 * FOnInventoryChanged
 * 
 * Broadcasts whenever any item count changes.
 * ItemID identifies which item changed — HUD uses this to refresh
 * only the relevant slot rather than redrawing everything.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, FName, ItemID);

/**
 * UInventoryComponent
 * 
 * Single unified component tracking quantity of every item the player owns.
 * 
 * OWNS:
 *   Item counts, add/remove operations, OnInventoryChanged delegate, carry limits.
 * 
 * DOES NOT OWN:
 *   Item visual data, item behaviour, UI display logic, crafting logic, merchant logic.
 *   Those systems read from this component — they do not live inside it.
 * 
 * SUPERSEDES:
 *   UPlayerResourceComponent (Gold + single Essence — remove from character at migration).
 *   Scattered PotionInventory TMap on SandboxCharacter_Mover.
 * 
 * GOLD NOTE:
 *   Gold is stored as int64 to handle late Endless Mode with multipliers at
 *   2000-3000 enemies per wave. All other items use int32. int64 is native
 *   on 64-bit hardware — zero performance cost.
 * 
 * STARTING INVENTORY:
 *   Set Blueprint defaults on this component directly for prototype.
 *   Migrate to UMissionStartData DataAsset at Alpha start when values are known.
 *   (Logged as deliberate migration point — do not forget.)
 * 
 * SERIALISATION:
 *   Not implemented for MVP. Save game system is post-launch.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALCHEMIST_0_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInventoryComponent();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// DELEGATE
	// Bind to this in HUD widgets on BeginPlay.
	// Broadcasts ItemID of the changed item — not a full inventory refresh.
	// -------------------------------------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	// -------------------------------------------------------------------------
	// DATA ASSETS
	// Populate in Blueprint — one entry per item type.
	// Key must match the FName constant in ItemIDs.h exactly.
	// Used to look up MaxCarryCount on AddItem.
	// -------------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, TObjectPtr<UItemDataAsset>> ItemDataAssets;

	// -------------------------------------------------------------------------
	// DEFAULT ITEMS
	// Set starting inventory here in Blueprint defaults for prototype phase.
	// Loaded into ItemCounts on BeginPlay.
	// Example: Key = "Potion_Fire", Value = 100
	// Migrate to UMissionStartData DataAsset at Alpha start (deliberate migration point).
	// Gold default uses DefaultGold below — int64 cannot be a TMap value in Blueprint.
	// -------------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory|Defaults")
	TMap<FName, int32> DefaultItems;

	// Starting gold. Set in Blueprint defaults. Loaded on BeginPlay.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory|Defaults")
	int64 DefaultGold = 0;

	// -------------------------------------------------------------------------
	// CORE INTERFACE
	// -------------------------------------------------------------------------

	/**
	 * AddItem
	 * Adds Amount to ItemID count. Clamps to MaxCarryCount (0 = uncapped).
	 * Gold (ItemIDs::Gold) is always uncapped — stored as int64 internally,
	 * routed through AddGold. Do not call AddItem with Gold directly in C++ —
	 * use AddGold instead. Blueprint may call AddItem with Gold safely.
	 * Broadcasts OnInventoryChanged after every call regardless of clamp.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FName ItemID, int32 Amount);

	/**
	 * RemoveItem
	 * Removes Amount from ItemID count. Clamps at zero — count never goes negative.
	 * Returns true if removal succeeded (had sufficient quantity).
	 * Returns false if insufficient — count unchanged, delegate still broadcasts.
	 * Use HasItem or GetCount to pre-check before calling if silent failure
	 * is not acceptable in your calling context.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Amount);

	/**
	 * HasItem
	 * Returns true if ItemID count is greater than zero.
	 * Pure — no side effects, safe to call every frame.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID) const;

	/**
	 * GetCount
	 * Returns current count for ItemID. Returns 0 if item not in inventory.
	 * Pure — no side effects, safe to call every frame.
	 * For Gold use GetGold — this returns 0 for Gold (int64 not representable as int32).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetCount(FName ItemID) const;

	/**
	 * AddGold
	 * Gold-specific add. Accepts int64 for large Endless Mode accumulations.
	 * Gold is always uncapped. Broadcasts OnInventoryChanged after call.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddGold(int64 Amount);

	/**
	 * RemoveGold
	 * Gold-specific remove. Returns true if sufficient gold, false if not.
	 * Count never goes below zero. Broadcasts OnInventoryChanged after call.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveGold(int64 Amount);

	/**
	 * GetGold
	 * Returns current gold as int64.
	 * Pure — no side effects, safe to call every frame.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int64 GetGold() const;

	/**
	 * HasGold
	 * Returns true if current gold >= Amount.
	 * Pure — no side effects. Use this before merchant purchases.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool HasGold(int64 Amount) const;

	/**
	 * ResetRunItems
	 * Called when player returns to village after a run.
	 * Zeroes all items where bPersistsToVillage = false on their DataAsset.
	 * Gold, Intermediates, Boss Materials, Special Throwables, Villager Collectibles survive.
	 * Essence, Potions, Raws reset to zero.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ResetRunItems();
private:

	// Item counts for all non-Gold items. Key = ItemID FName from ItemIDs.h.
	TMap<FName, int32> ItemCounts;

	// Gold stored separately as int64.
	// Late Endless Mode with multipliers at 2000-3000 enemies per wave
	// overflows int32 at ~1193 waves. int64 ceiling is effectively unreachable.
	int64 GoldCount = 0;

	// Resolves MaxCarryCount for ItemID from ItemDataAssets map.
	// Returns 0 if no data asset found (treats as uncapped — fail safe).
	int32 GetMaxCarryCount(FName ItemID) const;
};
