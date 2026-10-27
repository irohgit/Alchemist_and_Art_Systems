#pragma once

#include "CoreMinimal.h"

/**
 * ItemIDs
 * 
 * Canonical FName constants for every item in the inventory system.
 * Always reference these constants — never use raw FName strings in code.
 * 
 * Categories mirror EItemCategory in InventoryComponent.h:
 *   Potions | Essence | Gold | RawMaterial | Intermediate | BossMaterial | SpecialThrowable | VillagerCollectible
 * 
 * Adding a new item:
 *   1. Add a constant here in the correct category block
 *   2. Create a UItemDataAsset for it and set MaxCarryCount
 *   3. Register it in Blueprint starting inventory defaults if needed
 *   No code changes required anywhere else.
 */

namespace ItemIDs
{
	// -------------------------------------------------------------------------
	// POTIONS
	// Six core throwable potion types. MaxCarryCount set per DA_Item_[Name].
	// -------------------------------------------------------------------------
	static const FName Fire         = TEXT("Potion_Fire");
	static const FName Ice          = TEXT("Potion_Ice");
	static const FName Lightning    = TEXT("Potion_Lightning");
	static const FName Poison       = TEXT("Potion_Poison");
	static const FName Fear         = TEXT("Potion_Fear");
	static const FName Space        = TEXT("Potion_Space");

	// -------------------------------------------------------------------------
	// ESSENCE
	// Single generic essence pool. Three pickup tiers (Crude/Distilled/Quintessence)
	// all add to this same FName — tier determines AddItem Amount only.
	// D-059: Single generic Essence, typed system deferred post-launch.
	// D-060: Crude = 1, Distilled = 5, Quintessence = 20.
	// -------------------------------------------------------------------------
	static const FName Essence      = TEXT("Essence");

	// -------------------------------------------------------------------------
	// GOLD
	// Universal currency. Uncapped — stored as int64 in inventory component.
	// Spent at Goblin Merchant, plot upgrades, and synthesis rerolls.
	// -------------------------------------------------------------------------
	static const FName Gold         = TEXT("Gold");

	// -------------------------------------------------------------------------
	// RAW MATERIALS
	// Produced by farm plot creatures on timers. Unlocks after Wave 3 (D-065).
	// Primary use: synthesis into intermediates. Secondary: sell to merchant for gold.
	// -------------------------------------------------------------------------
	static const FName Honey        = TEXT("Raw_Honey");         // Bee plot — Fire Cake Bomb chain
	static const FName IceBall      = TEXT("Raw_IceBall");       // Arctic Penguin plot — Ice Snowball chain
	static const FName Jelly        = TEXT("Raw_Jelly");         // Electric Jellyfish plot — Lightning Bounce chain

	// -------------------------------------------------------------------------
	// INTERMEDIATES
	// Produced at Synthesis Station: Raw + Essence = Intermediate.
	// Consumable modifier use — one use per throw at the bench.
	// Permanently unlocked modifier requires no intermediate (bench uses Essence only).
	// -------------------------------------------------------------------------
	static const FName ViscousHoney = TEXT("Inter_ViscousHoney");  // Honey + Essence → Cake Bomb use
	static const FName GlacialPearl = TEXT("Inter_GlacialPearl");  // Ice Ball + Essence → Snowball use
	static const FName SparkJelly   = TEXT("Inter_SparkJelly");     // Jelly + Essence → Bounce use

	// -------------------------------------------------------------------------
	// BOSS MATERIALS
	// Dropped by bosses. 100% drop if matching modifier used on boss this run.
	// 15-20% baseline drop regardless. Combine with Intermediate at Synthesis
	// Station for permanent modifier unlock (D-061).
	// -------------------------------------------------------------------------
	static const FName ConfettiCore = TEXT("Boss_ConfettiCore");  // Cake Bomb permanent unlock
	static const FName FrostFang    = TEXT("Boss_FrostFang");     // Snowball permanent unlock
	static const FName ArcShard     = TEXT("Boss_ArcShard");      // Bounce permanent unlock

	// -------------------------------------------------------------------------
	// SPECIAL THROWABLES
	// Relationship-unlocked combat items outside the core potion system.
	// Purchased at Goblin Merchant or gifted via NPC quest. Consumable per run.
	// -------------------------------------------------------------------------
	static const FName OctopusPotion = TEXT("Special_OctopusPotion"); // Fisherman NPC — deploys temporary creature

	// -------------------------------------------------------------------------
	// VILLAGER COLLECTIBLES
	// Quest items. No combat use. Do not appear in combat HUD.
	// -------------------------------------------------------------------------
	// Add villager quest items here as quests are written.
	// Example: static const FName EdricLure = TEXT("Collect_EdricLure");
}
