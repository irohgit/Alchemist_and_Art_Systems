#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "StatusEffectComponent.h"
#include "EnemyBase.generated.h"

class AOmelette;
class AWaveManager;
class UEnemyPoolComponent;
class APhaseManager;
class UManagedTimerComponent;

UENUM(BlueprintType)
enum class EEnemyTier : uint8
{
    Standard    UMETA(DisplayName = "Standard"), Elite       UMETA(DisplayName = "Elite")
};
UENUM(BlueprintType)
enum class EEssenceTier : uint8
{
    Crude        UMETA(DisplayName = "Crude"),         // 1 unit
    Distilled    UMETA(DisplayName = "Distilled"),     // 5 units
    Quintessence UMETA(DisplayName = "Quintessence")  // 20 units
};

UCLASS()
class ALCHEMIST_0_API AEnemyBase : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructor
    AEnemyBase();

    //DELEGATES
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusApplied, EStatusType, StatusType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusExpired, EStatusType, StatusType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyAttacking, int32, DamageValue);
    UFUNCTION(Category = "Alchemist|Enemy|StatusEffects") void OnStatusApplied(EStatusType StatusType);
    UFUNCTION(Category = "Alchemist|Enemy|StatusEffects") void OnStatusExpired(EStatusType StatusType);
    UPROPERTY(BlueprintAssignable, Category = "Alchemist|Enemy|Combat") FOnEnemyAttacking OnEnemyAttacking; 
    
    // --- Combat ---
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy|CombatManager") bool bIsDead = false;
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy|CombatManager") bool bLastHitWasSupercharged = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Enemy|Combat") int32 MeleeDamage = 1;
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy|Combat") void StartAttacking();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy|Combat") void StopAttacking();
    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStatusEffectComponent> StatusEffectComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Enemy") TObjectPtr<UManagedTimerComponent> ManagedTimer;
    
    // --- Drops ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Enemy|Drops") EEssenceTier EssenceTier = EEssenceTier::Crude;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Enemy|Drops") int32 EssenceDropAmount = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Enemy|Drops") int32 GoldDropAmount = 1;
    UPROPERTY(EditDefaultsOnly, Category = "Alchemist|Enemy|Drops") TSubclassOf<AActor> PickupActorClass_Essence;
    UPROPERTY(EditDefaultsOnly, Category = "Alchemist|Enemy|Drops") TSubclassOf<AActor> PickupActorClass_Gold;

    // --- Omelette ---
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy") bool bIsCarryingOmelette = false; // True when omelette is physically attached to this enemy
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy") bool bIsRetrievingOmelette = false;    // True when this enemy is assigned as Omelette retriever
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy") TObjectPtr<AOmelette> CarriedOmelette = nullptr;
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy") void StartChasePlayer();
    
    // --- Pool ---
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy") bool bIsPooled = false;
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy") bool bIsInUse = false;

    // --- Public API ---
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual void OnReturnToPool();
    FTimerHandle ChaseTimer;
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy") FVector GetGroundLocationAtActor(AActor* TargetActor) const;
 
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy") void DropOmelette();
    UFUNCTION() void HandlePhaseChanged(EGamePhase NewPhase);
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy") void MoveToNextWaypoint();
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy") void StartOmeletteRetrieve(AOmelette* Omelette);    // Called by PhaseManager when this enemy is selected to retrieve Omelette

    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy") virtual void PickUpOmelette(AOmelette* Omelette);
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Enemy") virtual void TakeHit(int32 HitDamage, bool bIsSupercharged = false);
    
    // --- Stats ---
    UPROPERTY(BlueprintReadWrite, Category = "Alchemist|Enemy|Stats") bool bIsStationary = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Enemy|Stats") int32 HitPoints = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Enemy|Stats") float BaseMovementSpeed = 300.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Enemy|Stats") float MeleeAcceptanceRadius = 80.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Enemy|Stats") float RangedAcceptanceRadius = 600.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Enemy|Stats") EEnemyTier EnemyTier = EEnemyTier::Standard;

    // --- STATUS EFFECTS ---

    
    
    // --- Waypoint System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemist|Enemy|Movement") TArray<TObjectPtr<AActor>> TargetActors;
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy|Movement") int32 CurrentWaypointIndex = 0;
    UPROPERTY(BlueprintReadWrite, Category = "Alchemist|Enemy|Movement") float LateralOffset = 0.f;
    
    // --- Wave ---
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Enemy|Wave") bool bMovementHeld = true;
    UPROPERTY(BlueprintReadWrite, Category = "Alchemist|Enemy|Wave") TObjectPtr<AWaveManager> WaveManagerRef = nullptr;
    UPROPERTY(BlueprintReadWrite, Category = "Alchemist|Enemy") TObjectPtr<APhaseManager> PhaseManagerRef = nullptr;
  
    
protected:
    virtual void BeginPlay() override;

    virtual void OnDeath_Implementation();
    virtual void OnReachBoundary_Implementation();
    virtual void OnWaypointReached_Implementation(int32 WaypointIndex);

    UFUNCTION(BlueprintNativeEvent) void OnDeath();
    UFUNCTION(BlueprintNativeEvent) void OnReachBoundary(); 
    UFUNCTION(BlueprintNativeEvent) void OnWaypointReached(int32 WaypointIndex);
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
private:
    void HandleWaypointArrival(FAIRequestID RequestID, const FPathFollowingResult& Result);
    void DropResource();
    void DestroyEnemy();
    float WaypointAcceptanceRadius = 50.0f;
    FTimerHandle FearMovementTimer;
    FTimerHandle AttackTimer;
    FTimerHandle ReturnTimer;
    FTimerHandle DestroyTimer;
    
};