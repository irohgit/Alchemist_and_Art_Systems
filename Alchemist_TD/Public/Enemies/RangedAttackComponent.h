#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "alchemist_0/Public/Enemies/MageTypes.h"
#include "Projectiles/GroundProjectileBase.h"
#include "RangedAttackComponent.generated.h"
class AEnemyBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALCHEMIST_0_API URangedAttackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URangedAttackComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") EMageTier MageTier = EMageTier::Grey;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") EAttackMode AttackMode = EAttackMode::Cycle;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float BurstDelay = 0.4f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float CircleCooldown = 4.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float CircleWindUp = 1.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float PillarCooldown = 3.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float PillarProjectileSpeed = 2500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float PillarWindUp = 0.8f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float RepeatChance = 0.4f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float GroundProjectileCooldown = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float GroundProjectileRepeatChance = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float GroundProjectileWindUp = 0.6f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") int32 BurstCount = 2;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") TSubclassOf<AActor> MeteorActorClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") TSubclassOf<AActor> PillarProjectileClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") TSubclassOf<AGroundProjectileBase> GroundProjectileActorClass;
    UPROPERTY(BlueprintReadWrite, Category = "Alchemist|Mage") TObjectPtr<AActor> PlayerActor;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float GroundProjectileElevationThreshold = 200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float GroundProjectileHomingProximity = 600.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Mage") float GroundProjectileSpeed = 800.0f;


    UFUNCTION(BlueprintCallable, Category = "Alchemist|Mage") void StartAttackCycle();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Mage") void StopAttackCycle();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Mage") void SetAttackMode(EAttackMode NewMode);
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Mage") void SetMageTier(EMageTier NewTier);

    
protected:
    virtual void BeginPlay() override;

private:

    AEnemyBase* GetOwnerAsEnemy() const;
    TArray<EMageAbility> AvailableAbilities;
    int32 CurrentAbilityIndex = 0;
    int32 BurstCountRemaining = 0;
    bool bIsAttacking = false;
    EMageAbility LastFiredAbility = EMageAbility::CircleAoE;

    FTimerHandle AttackTimerHandle;
    FTimerHandle WindUpTimerHandle;

    void BuildAbilitySet();
    void ExecuteNextAbility();
    EMageAbility SelectNextAbility();
    void FireAbility(EMageAbility Ability);
    void StartWindUp(EMageAbility Ability);

    void CastCircleAoE();
    void CastPillarStrike();
    void CastGroundProjectile();

    FVector GetPlayerLocation() const;
    float GetCooldownForAbility(EMageAbility Ability) const;
    float GetWindUpForAbility(EMageAbility Ability) const;
    float GetRepeatChanceForAbility(EMageAbility Ability) const;
    bool bPlayerElevatedAtWindUp = false;
    // Helper Functions
    void FacePlayer();
    FRotator GetProjectileSpawnRotation() const;
};