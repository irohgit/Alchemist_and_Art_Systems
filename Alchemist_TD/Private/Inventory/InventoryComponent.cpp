#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemIDs.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// -----------------------------------------------------------------------------
// BeginPlay
// Loads DefaultItems into ItemCounts and DefaultGold into GoldCount.
// Runs once at game start. Does not broadcast — HUD should read counts
// directly on construct rather than waiting for a change event.
// -----------------------------------------------------------------------------
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	for (const TPair<FName, int32>& DefaultItem : DefaultItems)
	{
		if (DefaultItem.Value > 0)
		{
			AddItem(DefaultItem.Key, DefaultItem.Value);
		}
	}

	if (DefaultGold > 0)
	{
		AddGold(DefaultGold);
	}
}

// -----------------------------------------------------------------------------
// AddItem
// Routes Gold to AddGold to preserve int64 handling.
// All other items: look up MaxCarryCount, clamp, store, broadcast.
// -----------------------------------------------------------------------------
void UInventoryComponent::AddItem(FName ItemID, int32 Amount)
{
	if (ItemID == ItemIDs::Gold)
	{
		AddGold(static_cast<int64>(Amount));
		return;
	}

	if (Amount <= 0) return;

	const int32 MaxCount = GetMaxCarryCount(ItemID);
	const int32 Current  = ItemCounts.FindOrAdd(ItemID);

	if (MaxCount > 0)
	{
		// Capped item — clamp silently at MaxCarryCount.
		ItemCounts[ItemID] = FMath::Min(Current + Amount, MaxCount);
	}
	else
	{
		// Uncapped item (MaxCarryCount == 0 in DataAsset means no ceiling).
		ItemCounts[ItemID] = Current + Amount;
	}

	OnInventoryChanged.Broadcast(ItemID);
}

// -----------------------------------------------------------------------------
// RemoveItem
// Returns false and broadcasts if insufficient quantity.
// Count never goes below zero.
// -----------------------------------------------------------------------------
bool UInventoryComponent::RemoveItem(FName ItemID, int32 Amount)
{
	if (ItemID == ItemIDs::Gold)
	{
		return RemoveGold(static_cast<int64>(Amount));
	}

	if (Amount <= 0) return true;

	int32* Current = ItemCounts.Find(ItemID);
	if (!Current || *Current < Amount)
	{
		// Insufficient quantity — do not modify, still broadcast so HUD can react.
		OnInventoryChanged.Broadcast(ItemID);
		return false;
	}

	*Current -= Amount;
	OnInventoryChanged.Broadcast(ItemID);
	return true;
}

// -----------------------------------------------------------------------------
// HasItem
// Pure — no side effects.
// -----------------------------------------------------------------------------
bool UInventoryComponent::HasItem(FName ItemID) const
{
	if (ItemID == ItemIDs::Gold)
	{
		return GoldCount > 0;
	}

	const int32* Current = ItemCounts.Find(ItemID);
	return Current && *Current > 0;
}

// -----------------------------------------------------------------------------
// GetCount
// Returns 0 for Gold — callers must use GetGold for int64 accuracy.
// -----------------------------------------------------------------------------
int32 UInventoryComponent::GetCount(FName ItemID) const
{
	if (ItemID == ItemIDs::Gold)
	{
		// Gold is int64. Returning via int32 here would silently truncate.
		// Use GetGold() for Gold. Return 0 to surface the incorrect call.
		return 0;
	}

	const int32* Current = ItemCounts.Find(ItemID);
	return Current ? *Current : 0;
}

// -----------------------------------------------------------------------------
// AddGold
// int64 — handles late Endless Mode accumulation safely.
// -----------------------------------------------------------------------------
void UInventoryComponent::AddGold(int64 Amount)
{
	if (Amount <= 0) return;
	GoldCount += Amount;
	OnInventoryChanged.Broadcast(ItemIDs::Gold);
}

// -----------------------------------------------------------------------------
// RemoveGold
// Returns false if insufficient. Count never goes negative.
// -----------------------------------------------------------------------------
bool UInventoryComponent::RemoveGold(int64 Amount)
{
	if (Amount <= 0) return true;

	if (GoldCount < Amount)
	{
		OnInventoryChanged.Broadcast(ItemIDs::Gold);
		return false;
	}

	GoldCount -= Amount;
	OnInventoryChanged.Broadcast(ItemIDs::Gold);
	return true;
}

// -----------------------------------------------------------------------------
// GetGold
// Pure — returns int64 directly.
// -----------------------------------------------------------------------------
int64 UInventoryComponent::GetGold() const
{
	return GoldCount;
}

// -----------------------------------------------------------------------------
// HasGold
// Pure — returns true if current gold >= Amount.
// Use before every merchant purchase.
// -----------------------------------------------------------------------------
bool UInventoryComponent::HasGold(int64 Amount) const
{
	return GoldCount >= Amount;
}

// -----------------------------------------------------------------------------
// GetMaxCarryCount (private)
// Looks up the DataAsset for ItemID and returns MaxCarryCount.
// Returns 0 (uncapped) if no DataAsset found — fail safe, never blocks AddItem.
// -----------------------------------------------------------------------------
int32 UInventoryComponent::GetMaxCarryCount(FName ItemID) const
{
	const TObjectPtr<UItemDataAsset>* Asset = ItemDataAssets.Find(ItemID);
	if (!Asset || !(*Asset))
	{
		// No DataAsset registered — treat as uncapped.
		// This will not crash but will log a warning in debug builds.
		UE_LOG(LogTemp, Warning,
			TEXT("UInventoryComponent::GetMaxCarryCount — No DataAsset found for ItemID: %s. Treating as uncapped."),
			*ItemID.ToString());
		return 0;
	}

	return (*Asset)->MaxCarryCount;
}

void UInventoryComponent::ResetRunItems()
{
	for (auto& Pair : ItemCounts)
	{
		const TObjectPtr<UItemDataAsset>* Asset = ItemDataAssets.Find(Pair.Key);
		if (!Asset || !(*Asset) || !(*Asset)->bPersistsToVillage)
		{
			Pair.Value = 0;
		}
	}
	// Broadcast once per item that changed — HUD refreshes all non-persistent slots
	for (const auto& Pair : ItemCounts)
	{
		OnInventoryChanged.Broadcast(Pair.Key);
	}
}