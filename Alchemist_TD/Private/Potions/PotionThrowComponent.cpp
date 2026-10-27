#include "Potions/PotionThrowComponent.h"
#include "Potions/PotionDataAsset.h"
#include "Projectiles/ProjectileBase.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemIDs.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Camera/PlayerCameraManager.h"

UPotionThrowComponent::UPotionThrowComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPotionThrowComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedInventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!CachedInventory)
	{
		UE_LOG(LogTemp, Error, TEXT("UPotionThrowComponent — no UInventoryComponent found on owner. Throws will be blocked."));
	}
}

// -----------------------------------------------------------------------------
// ExecuteThrow
// Entry point — called from Blueprint on trigger press.
// Resolves potion type → validates inventory → spawns → decrements.
// -----------------------------------------------------------------------------
void UPotionThrowComponent::ExecuteThrow(EPotionAssignment TriggerSide,EPotionType LeftTriggerPotion,EPotionType RightTriggerPotion)
{
	// 1. Entry guard
	if (TriggerSide == EPotionAssignment::None) return;

	// 2. Resolve potion type from trigger side
	EPotionType PotionType = (TriggerSide == EPotionAssignment::LeftTrigger)
		? LeftTriggerPotion
		: RightTriggerPotion;

	// 3. Resolve ItemID
	FName ItemID = GetItemIDForPotionType(PotionType);
	if (ItemID == NAME_None) return;

	// 4. Inventory check
	if (!CachedInventory || !CachedInventory->HasItem(ItemID))
	{
		return;
	}

	// 5. Spawn — decision layer resolves class and modifier
	SpawnPotion(PotionType,TriggerSide);

	// 6. Decrement — only after successful spawn call
	CachedInventory->RemoveItem(ItemID, 1);

	// 7. Stat counter — only the success path increments
	PotionsThrownThisRun++;
	
}

// -----------------------------------------------------------------------------
// SpawnPotion
// Decision layer — resolves projectile class from DataAsset and active modifier.
// Hands off to ExecuteSpawn with final resolved data.
// -----------------------------------------------------------------------------
void UPotionThrowComponent::SpawnPotion(EPotionType PotionType,EPotionAssignment TriggerSide)
{
	// 1. Look up DataAsset
	TObjectPtr<UPotionDataAsset>* DataAssetPtr = PotionDataAssets.Find(PotionType);
	if (!DataAssetPtr || !*DataAssetPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnPotion — no DataAsset for PotionType %d"), (int32)PotionType);
		return;
	}
	TObjectPtr<UPotionDataAsset> DataAsset = *DataAssetPtr;

	// 2. Start with base projectile class
	TSubclassOf<AActor> ProjectileClass = DataAsset->BaseProjectileClass;
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnPotion — BaseProjectileClass is null on DataAsset for PotionType %d"), (int32)PotionType);
		return;
	}

	// 3. Check active modifier — may override projectile class
	EModifierType* ActiveModifier = ActiveModifiers.Find(PotionType);
	if (ActiveModifier && *ActiveModifier != EModifierType::None)
	{
		int32 ModifierIndex = GetModifierIndex(*ActiveModifier);
		if (ModifierIndex >= 0 && ModifierIndex < DataAsset->Modifiers.Num())
		{
			const FPotionModifierData& ModifierData = DataAsset->Modifiers[ModifierIndex];
			if (ModifierData.ProjectileClassOverride)
			{
				ProjectileClass = ModifierData.ProjectileClassOverride;
			}
		}
	}

	// 4. Hand off to execution layer
	ExecuteSpawn(DataAsset, ProjectileClass,TriggerSide);
}

// -----------------------------------------------------------------------------
// ExecuteSpawn
// Execution layer — deferred spawn, initialise DataAsset, apply velocity.
// Uses SpawnActorDeferred so InitialiseProjectile fires before BeginPlay.
// -----------------------------------------------------------------------------
void UPotionThrowComponent::ExecuteSpawn(UPotionDataAsset* DataAsset, TSubclassOf<AActor> ProjectileClass, EPotionAssignment TriggerSide)
{
	if (!DataAsset || !ProjectileClass) return;

	// 1. Owner as Pawn — SandboxCharacter_Mover inherits APawn not ACharacter
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	// 2. Camera direction
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!CameraManager) return;

	const FRotator CameraRotation = CameraManager->GetCameraRotation();
	const FVector ForwardVector = CameraRotation.Vector();

	// 3. Spawn in the middle to be consistent
	const FVector CharacterLocation = OwnerPawn->GetActorLocation();
	const FVector SpawnLocation = CharacterLocation 
		+ ForwardVector * 60.f 
		+ FVector(0.f, 0.f, 60.f);

	// 4. Spawn transform
	const FTransform SpawnTransform(CameraRotation, SpawnLocation);

	// 5. Deferred spawn
	AProjectileBase* Projectile = GetWorld()->SpawnActorDeferred<AProjectileBase>(
		ProjectileClass,
		SpawnTransform,
		OwnerPawn,
		OwnerPawn,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (!Projectile)
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteSpawn — SpawnActorDeferred returned null for class %s"), *ProjectileClass->GetName());
		return;
	}

	// 6. Initialize before BeginPlay
	Projectile->InitialiseProjectile(DataAsset, false);

	// 7. Ignore owner collision
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Projectile->GetRootComponent()))
	{
		RootPrim->IgnoreActorWhenMoving(OwnerPawn, true);
	}

	// 8. Finish spawn — BeginPlay fires here
	UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);

	// 9. Launch velocity
	const FVector LaunchVelocity =
		(ForwardVector * DataAsset->ProjectileVelocity) +
		(FVector::UpVector * DataAsset->ArcHeight * DataAsset->ProjectileVelocity);

	if (Projectile->ProjectileMovement)
	{
		Projectile->ProjectileMovement->Velocity = LaunchVelocity;
		Projectile->ProjectileMovement->Activate(true);
	}
}

// -----------------------------------------------------------------------------
// GetResolvedThrowBehaviour
// Returns the effective throw behaviour for the current modifier.
// Ice_Snowball overrides to SplineRoll. Everything else defers to DataAsset.
// -----------------------------------------------------------------------------
EThrowBehaviourType UPotionThrowComponent::GetResolvedThrowBehaviour(
	EModifierType CurrentModifier,
	UPotionDataAsset* CurrentDataAsset) const
{
	if (!CurrentDataAsset) return EThrowBehaviourType::Standard;

	if (CurrentModifier == EModifierType::Ice_Snowball)
		return EThrowBehaviourType::SplineRoll;

	return CurrentDataAsset->ThrowBehaviourType;
}

// -----------------------------------------------------------------------------
// GetModifierIndex
// Returns array index (0/1/2) for modifier lookup in DataAsset.Modifiers.
// Returns -1 for None or unrecognised values.
// -----------------------------------------------------------------------------
int32 UPotionThrowComponent::GetModifierIndex(EModifierType ModifierType) const
{
	switch (ModifierType)
	{
	case EModifierType::None: return -1;

	case EModifierType::Fire_CakeBomb:       return 0;
	case EModifierType::Ice_Freeze:          return 0;
	case EModifierType::Lightning_Bolt:      return 0;
	case EModifierType::Poison_TBD_1:        return 0;
	case EModifierType::Fear_TBD_1:          return 0;
	case EModifierType::Space_TBD_1:         return 0;

	case EModifierType::Fire_Cluster:        return 1;
	case EModifierType::Ice_Lattice:         return 1;
	case EModifierType::Lightning_Bounce:    return 1;
	case EModifierType::Poison_TBD_2:        return 1;
	case EModifierType::Fear_TBD_2:          return 1;
	case EModifierType::Space_TBD_2:         return 1;

	case EModifierType::Fire_Contagion:      return 2;
	case EModifierType::Ice_Snowball:        return 2;
	case EModifierType::Lightning_Conductor: return 2;
	case EModifierType::Poison_TBD_3:        return 2;
	case EModifierType::Fear_TBD_3:          return 2;
	case EModifierType::Space_TBD_3:         return 2;

	default: return -1;
	}
}

// -----------------------------------------------------------------------------
// GetItemIDForPotionType
// Maps EPotionType to FName ItemID for inventory lookup.
// -----------------------------------------------------------------------------
FName UPotionThrowComponent::GetItemIDForPotionType(EPotionType PotionType) const
{
	switch (PotionType)
	{
	case EPotionType::Fire:      return ItemIDs::Fire;
	case EPotionType::Ice:       return ItemIDs::Ice;
	case EPotionType::Lightning: return ItemIDs::Lightning;
	case EPotionType::Poison:    return ItemIDs::Poison;
	case EPotionType::Fear:      return ItemIDs::Fear;
	case EPotionType::Space:     return ItemIDs::Space;
	default:
		UE_LOG(LogTemp, Warning, TEXT("GetItemIDForPotionType — unhandled PotionType %d"), (int32)PotionType);
		return NAME_None;
	}
}
void UPotionThrowComponent::SetTriggerPotion(EPotionAssignment TriggerSide, EPotionType PotionType)
{
	if (TriggerSide == EPotionAssignment::None) return;
	EquippedPotions.Add(TriggerSide, PotionType);
	// Selecting a potion equips it with its favourite (Base if none set).
	const EModifierType Favourite = GetFavouriteModifier(PotionType);
	ActiveModifiers.Add(PotionType, Favourite);
	if (GEngine)
	{
		const FString PotionName = StaticEnum<EPotionType>()->GetDisplayNameTextByValue((int64)PotionType).ToString();
		const FString SideName   = StaticEnum<EPotionAssignment>()->GetDisplayNameTextByValue((int64)TriggerSide).ToString();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,FString::Printf(TEXT("%s = %s (favourite applied)"), *SideName, *PotionName));
	}
}

void UPotionThrowComponent::SetCurrentModifier(EPotionType PotionType, EModifierType Modifier)
{
	ActiveModifiers.Add(PotionType, Modifier);   // Favourite deliberately untouched

	if (GEngine)
	{
		const FString ModName = StaticEnum<EModifierType>()->GetDisplayNameTextByValue((int64)Modifier).ToString();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Modifier -> %s"), *ModName));
	}
}

void UPotionThrowComponent::SetFavouriteModifier(EPotionType PotionType, EModifierType Modifier)
{
	FavouriteModifiers.Add(PotionType, Modifier);
	ActiveModifiers.Add(PotionType, Modifier);   // apply immediately

	if (GEngine)
	{
		const FString ModName = StaticEnum<EModifierType>()->GetDisplayNameTextByValue((int64)Modifier).ToString();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Favourite set -> %s"), *ModName));
	}
}

EModifierType UPotionThrowComponent::GetCurrentModifier(EPotionType PotionType) const
{
	if (const EModifierType* Found = ActiveModifiers.Find(PotionType)) return *Found;
	return EModifierType::None;   // None = Base behaviour
}

EModifierType UPotionThrowComponent::GetFavouriteModifier(EPotionType PotionType) const
{
	if (const EModifierType* Found = FavouriteModifiers.Find(PotionType)) return *Found;
	return EModifierType::None;
}

EPotionType UPotionThrowComponent::GetEquippedPotion(EPotionAssignment TriggerSide) const
{
	if (const EPotionType* Found = EquippedPotions.Find(TriggerSide)) return *Found;
	return EPotionType::Fire;   // sensible default
}