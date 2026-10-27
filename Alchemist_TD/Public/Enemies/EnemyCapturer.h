#pragma once
#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "EnemyCapturer.generated.h"

class AOmelette;
class APhaseManager;

UCLASS()
class ALCHEMIST_0_API ACapturer : public AEnemyBase
{
    GENERATED_BODY()

public:

    ACapturer();

    // -------------------------------------------------------------------------
    // CONFIGURATION — set in Blueprint defaults
    // -------------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Capturer") FName OmeletteSocketName = FName("OmeletteSocket");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Capturer") float EscapeArcHeight = 500.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Capturer") float EscapeMovementSpeed = 150.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alchemist|Capturer") TObjectPtr<URadialForceComponent> RadialForce = nullptr;
    // -------------------------------------------------------------------------
    // REFERENCES — set by Blueprint after spawn via InitialiseCapturer
    // -------------------------------------------------------------------------
    UPROPERTY(BlueprintReadWrite, Category = "Alchemist|Capturer") AOmelette* OmeletteRef = nullptr;

    // -------------------------------------------------------------------------
    // STATE
    // -------------------------------------------------------------------------
    UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Capturer") bool bHasOmelette = false;

    // -------------------------------------------------------------------------
    // PUBLIC API
    // -------------------------------------------------------------------------

    // Called by Blueprint after spawn — sets reverse waypoints, stores refs
    UFUNCTION(BlueprintCallable, Category = "Alchemist|Capturer")
    void InitialiseCapturer
    (
        const TArray<AActor*>& ApproachWaypoints,  
        const TArray<AActor*>& EscapeWaypoints,   
        AOmelette* Omelette,
        APhaseManager* PhaseManager
    );

    // Called when capturer reaches Omelette
    virtual void PickUpOmelette(AOmelette* Omelette) override;

    // -------------------------------------------------------------------------
    // BLUEPRINT HOOKS
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Capturer")
    void OnOmelettePickedUp();

    UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Capturer")
    void OnEscapeBegin();

    UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Capturer")
    void OnCapturerEscaped();

protected:

    virtual void OnDeath_Implementation() override;
    virtual void OnReachBoundary_Implementation() override;
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:

    bool bIsEscaping = false;
    FVector EscapeStartLocation;
    FVector EscapeEndLocation;
    float EscapeAlpha = 0.f;

    void BeginEscapeMovement();
    FTimerHandle ReviveDelayTimer;
    

    UPROPERTY()
    TArray<TObjectPtr<AActor>> StoredEscapeWaypoints;
};