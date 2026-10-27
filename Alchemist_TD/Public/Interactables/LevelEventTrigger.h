#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelEventTrigger.generated.h"

class USphereComponent;
class UBoxComponent;
class UShapeComponent;
class ALevelEventTrigger;

/** Which collision primitive an ALevelEventTrigger uses. Dropdown = mutually exclusive. */
UENUM(BlueprintType)
enum class ELevelEventTriggerShape : uint8
{
    Sphere,
    Box
};

/** Fired when the player overlaps this trigger. Passes the trigger itself so one handler can route many. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelEventTriggered, ALevelEventTrigger*, Trigger, AActor*, TriggerInstigator);

/**
 * Generic placed trigger volume. Makes NO decisions — it only announces "the player entered me".
 * Listeners decide what happens: the TutorialDirector for orchestrated flows, or an NPC's own BP
 * for self-contained beats (e.g. a repeatable startle). Pick Sphere or Box via TriggerShape.
 */
UCLASS()
class ALCHEMIST_0_API ALevelEventTrigger : public AActor
{
    GENERATED_BODY()

public:
    ALevelEventTrigger();

    /** Broadcast on player overlap (honours bRepeatable / consumed state). */
    UPROPERTY(BlueprintAssignable, Category = "Level Event")
    FOnLevelEventTriggered OnTriggered;

    /** Identifier so a listener can tell which event this is (e.g. "Tito_Intro"). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Event")
    FName EventId;

    /** false = fires once then ignores further overlaps. true = fires every entry. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Event")
    bool bRepeatable = false;

    /** Hint to the director: disable this trigger once its flow completes. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Event")
    bool bConsumeOnComplete = true;

    /** Disable the trigger so it can no longer fire. Director calls this after a one-shot flow. */
    UFUNCTION(BlueprintCallable, Category = "Level Event")
    void ConsumeTrigger();

    /** Re-enable a consumed trigger and clear its fired state. */
    UFUNCTION(BlueprintCallable, Category = "Level Event")
    void RearmTrigger();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Event|Shape")
    ELevelEventTriggerShape TriggerShape = ELevelEventTriggerShape::Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Event|Shape",
        meta = (EditCondition = "TriggerShape == ELevelEventTriggerShape::Sphere", EditConditionHides))
    float SphereRadius = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Event|Shape",
        meta = (EditCondition = "TriggerShape == ELevelEventTriggerShape::Box", EditConditionHides))
    FVector BoxExtent = FVector(200.f, 200.f, 200.f);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Event|Shape")
    TObjectPtr<USphereComponent> SphereTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Event|Shape")
    TObjectPtr<UBoxComponent> BoxTrigger;

private:
    void ApplyTriggerShape();
    UShapeComponent* GetActiveTriggerComponent() const;

    UFUNCTION()
    void HandleTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    bool bHasFired = false;
};