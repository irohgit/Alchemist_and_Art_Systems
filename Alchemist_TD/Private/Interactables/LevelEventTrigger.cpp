#include "Interactables/LevelEventTrigger.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ALevelEventTrigger::ALevelEventTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    SphereTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("SphereTrigger"));
    SetRootComponent(SphereTrigger);
    SphereTrigger->InitSphereRadius(SphereRadius);

    BoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTrigger"));
    BoxTrigger->SetupAttachment(SphereTrigger);
    BoxTrigger->InitBoxExtent(BoxExtent);

    // Shared query setup. Per-shape enable/disable is decided in ApplyTriggerShape().
    // Overlap the Pawn channel only; player identity is still verified in the handler.
    for (UShapeComponent* Shape : { static_cast<UShapeComponent*>(SphereTrigger),
                                    static_cast<UShapeComponent*>(BoxTrigger) })
    {
        Shape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Shape->SetCollisionObjectType(ECC_WorldDynamic);
        Shape->SetCollisionResponseToAllChannels(ECR_Ignore);
        Shape->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
        Shape->SetHiddenInGame(true);
    }
}

void ALevelEventTrigger::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyTriggerShape();
}

void ALevelEventTrigger::BeginPlay()
{
    Super::BeginPlay();
    ApplyTriggerShape();

    if (UShapeComponent* ActiveTrigger = GetActiveTriggerComponent())
    {
        ActiveTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALevelEventTrigger::HandleTriggerOverlap);
    }
}

void ALevelEventTrigger::ApplyTriggerShape()
{
    if (!SphereTrigger || !BoxTrigger) return;
    SphereTrigger->InitSphereRadius(SphereRadius);
    BoxTrigger->InitBoxExtent(BoxExtent);
    const bool bUseSphere = (TriggerShape == ELevelEventTriggerShape::Sphere);
    SphereTrigger->SetCollisionEnabled(bUseSphere ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    SphereTrigger->SetGenerateOverlapEvents(bUseSphere);
    SphereTrigger->SetVisibility(bUseSphere);
    BoxTrigger->SetCollisionEnabled(!bUseSphere ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    BoxTrigger->SetGenerateOverlapEvents(!bUseSphere);
    BoxTrigger->SetVisibility(!bUseSphere);
}

UShapeComponent* ALevelEventTrigger::GetActiveTriggerComponent() const
{
    return (TriggerShape == ELevelEventTriggerShape::Sphere)
        ? static_cast<UShapeComponent*>(SphereTrigger)
        : static_cast<UShapeComponent*>(BoxTrigger);
}

void ALevelEventTrigger::HandleTriggerOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* OtherActor,
    UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
    // Only the player triggers level events.
    if (OtherActor != UGameplayStatics::GetPlayerPawn(this, 0)) return;
    if (!bRepeatable && bHasFired) return;
    bHasFired = true;

    OnTriggered.Broadcast(this, OtherActor);
}

void ALevelEventTrigger::ConsumeTrigger()
{
    if (UShapeComponent* ActiveTrigger = GetActiveTriggerComponent())
    {
        ActiveTrigger->SetGenerateOverlapEvents(false);
        ActiveTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ALevelEventTrigger::RearmTrigger()
{
    bHasFired = false;
    ApplyTriggerShape();
    if (UShapeComponent* ActiveTrigger = GetActiveTriggerComponent())
    {
        ActiveTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        ActiveTrigger->SetGenerateOverlapEvents(true);
    }
}