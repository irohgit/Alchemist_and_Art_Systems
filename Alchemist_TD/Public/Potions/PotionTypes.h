#pragma once

#include "CoreMinimal.h"
#include "PotionTypes.generated.h"

UENUM(BlueprintType)
enum class EPotionType : uint8
{
    Fire        UMETA(DisplayName = "Fire"),
    Ice         UMETA(DisplayName = "Ice"),
    Lightning   UMETA(DisplayName = "Lightning"),
    Poison      UMETA(DisplayName = "Poison"),
    Fear        UMETA(DisplayName = "Fear"),
    Space       UMETA(DisplayName = "Space")
};

UENUM(BlueprintType)
enum class EPotionAssignment : uint8
{
    None            UMETA(DisplayName = "None"),
    LeftTrigger     UMETA(DisplayName = "Left Trigger"),
    RightTrigger    UMETA(DisplayName = "Right Trigger")
};

UENUM(BlueprintType)
enum class EThrowBehaviourType : uint8
{
    Standard    UMETA(DisplayName = "Standard"),
    Pierce      UMETA(DisplayName = "Pierce"),
    Instant     UMETA(DisplayName = "Instant"),
    SplineRoll  UMETA(DisplayName = "Spline Roll"),
    NodeSpawn   UMETA(DisplayName = "Node Spawn")
};

UENUM(BlueprintType)
enum class EModifierType : uint8
{
    None                    UMETA(DisplayName = "None"),
    // Fire
    Fire_CakeBomb           UMETA(DisplayName = "Fire — Cake Bomb"),
    Fire_Cluster            UMETA(DisplayName = "Fire — Cluster"),
    Fire_Contagion          UMETA(DisplayName = "Fire — Contagion"),
    // Ice
    Ice_Freeze              UMETA(DisplayName = "Ice — Freeze"),
    Ice_Lattice             UMETA(DisplayName = "Ice — Lattice"),
    Ice_Snowball            UMETA(DisplayName = "Ice — Snowball"),
    // Lightning
    Lightning_Bolt          UMETA(DisplayName = "Lightning — Bolt"),
    Lightning_Bounce        UMETA(DisplayName = "Lightning — Bounce"),
    Lightning_Conductor     UMETA(DisplayName = "Lightning — Conductor"),
    // Poison
    Poison_TBD_1            UMETA(DisplayName = "Poison — TBD 1"),
    Poison_TBD_2            UMETA(DisplayName = "Poison — TBD 2"),
    Poison_TBD_3            UMETA(DisplayName = "Poison — TBD 3"),
    // Fear
    Fear_TBD_1              UMETA(DisplayName = "Fear — TBD 1"),
    Fear_TBD_2              UMETA(DisplayName = "Fear — TBD 2"),
    Fear_TBD_3              UMETA(DisplayName = "Fear — TBD 3"),
    // Space
    Space_TBD_1             UMETA(DisplayName = "Space — TBD 1"),
    Space_TBD_2             UMETA(DisplayName = "Space — TBD 2"),
    Space_TBD_3             UMETA(DisplayName = "Space — TBD 3")
};

USTRUCT(BlueprintType)
struct FPotionModifierData
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier") EModifierType ModifierType = EModifierType::None;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier") TSubclassOf<AActor> ProjectileClassOverride;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier") EThrowBehaviourType ThrowBehaviourOverride = EThrowBehaviourType::Standard;
};