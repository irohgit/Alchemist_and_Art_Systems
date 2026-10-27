#include "Pickups/PickupBase.h"
#include "Components/StaticMeshComponent.h"

APickupBase::APickupBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(RootComponent);

}

void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	// 1. Capture spawn location at the moment the actor exists in the world
	SpawnLocation = GetActorLocation();
	// 2. Get the player controller so we can convert screen space to world space
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC) //(NC)_Nullcheck
	{
	// 3. Deproject the 2D screen position (pixels) into a 3D world position and direction
		FVector WorldLocation; FVector WorldDirection;
		PC->DeprojectScreenPositionToWorld(HUDTargetScreenPosition.X,HUDTargetScreenPosition.Y,WorldLocation,WorldDirection);
	// 4. Project 500 units along the direction ray to get a reachable world point
		TargetLocation = WorldLocation + WorldDirection * 500.f;
	}
	else
	{
	// 5. Fallback if no player controller found — fly straight up
		TargetLocation = SpawnLocation + FVector(0.f, 0.f, FlyHeight);
	}
	// 6. Only start flying if explicitly triggered — bAutoFly gate prevents BeginPlay firing in editor
	if (bAutoFly)
	{
		bFlying = true;
	}
}
void APickupBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cancel any pending fly-delay timer so it can never fire against a destroyed pickup.
	GetWorldTimerManager().ClearTimer(FlyDelayTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bFlying) return;

	ElapsedTime += DeltaTime;
	float Alpha = FMath::Clamp(ElapsedTime / FlyDuration, 0.f, 1.f);

	// Ease out — feels snappier than linear
	float EasedAlpha = 1.f - FMath::Square(1.f - Alpha);

	SetActorLocation(FMath::Lerp(SpawnLocation, TargetLocation, EasedAlpha));

	if (Alpha >= 1.f)
	{
		bFlying = false;
		Destroy();
	}
}
void APickupBase::StartFly()
{
	// 1. Check if a delay is requested before the pickup starts flying
	if (SpawnDelay > 0.f)
	{
		// 2. Member handle — so EndPlay can cancel this if the world tears down mid-delay
		GetWorldTimerManager().SetTimer(FlyDelayTimerHandle, [this]()
		{
			if (!IsValid(this)) return;  // necessary but NOT sufficient — the real fix is the EndPlay clear
			// 3. Notify Blueprint to activate Niagara effect
			OnStartFly();
			// 4. Start the fly animation in Tick
			bFlying = true;
		}, SpawnDelay, false); // false = fire once only
	}
	else
	{
		// 5. No delay — fire immediately
		OnStartFly();
		bFlying = true;
	}
}