#include "Cinematics/CinematicImageManager.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Playercontroller.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"

ACinematicImageManager::ACinematicImageManager()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	CinematicImagePlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImagePlane"));
	CinematicImagePlane->SetupAttachment(Root);
	CinematicImagePlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CinematicCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CinematicCamera"));
	CinematicCamera->SetupAttachment(Root);
}

void ACinematicImageManager::BeginPlay()
{
	Super::BeginPlay();
	if (BaseMaterial && CinematicImagePlane)
	{
		PlaneMID = UMaterialInstanceDynamic::Create(BaseMaterial,this);
		CinematicImagePlane->SetMaterial(0,PlaneMID);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,10.f,FColor::Red,TEXT("CinematicImageManager: assign BaseMaterial + an CinematicImagePlane mesh."));
	}
	if (bAutoPlayOnBeginPlay)
	{
		PlayCinematicImageSequence();
	}
}
void ACinematicImageManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CurrentVoiceOver)
	{
		CurrentVoiceOver->Stop();
		CurrentVoiceOver = nullptr;
	}
	GetWorldTimerManager().ClearTimer(AdvanceTimer);
	Super::EndPlay(EndPlayReason);
}
void ACinematicImageManager::AdvanceCinematicImageSequence()
{
	CurrentIndex++;
	// Past the last entry → sequence is done
	if (!CinematicImageSequence.IsValidIndex(CurrentIndex))
	{
		FinishCinematicImageSequence();
		return;
	}
	ShowCinematicImageEntry(CurrentIndex);
}
void ACinematicImageManager::FinishCinematicImageSequence()
{
	OnCinematicComplete.Broadcast();
}
void ACinematicImageManager::PlayCinematicImageSequence()
{
	// Exit if no images to be played
	if (CinematicImageSequence.Num() == 0)
	{
		FinishCinematicImageSequence();
		return;
	}
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(),0))
	{
		PlayerController->SetViewTargetWithBlend(this,0.f);
	}
	//Reset Index to default -1
	CurrentIndex = -1;
	AdvanceCinematicImageSequence();
}
void ACinematicImageManager::ShowCinematicImageEntry(int32 Index)
{
	const FCinematicImageEntry& SCinematicImageEntry = CinematicImageSequence[Index];
	if (PlaneMID && SCinematicImageEntry.Image)
	{
		PlaneMID->SetTextureParameterValue(TextureParameterName, SCinematicImageEntry.Image);
	}
	if (CurrentVoiceOver)
	{
		CurrentVoiceOver->Stop();
		CurrentVoiceOver = nullptr;
	}
	float Hold = SCinematicImageEntry.DisplayDuration;
	if (SCinematicImageEntry.VoiceOver)
	{
		CurrentVoiceOver = UGameplayStatics::SpawnSound2D(this,SCinematicImageEntry.VoiceOver);
		const float VoiceOverLength = SCinematicImageEntry.VoiceOver->GetDuration();
		if (VoiceOverLength > 0.f && VoiceOverLength < INDEFINITELY_LOOPING_DURATION)
		{
			Hold = VoiceOverLength + SCinematicImageEntry.ExtraHoldAfterVoiceOver;
		}
	}
	Hold = FMath::Max(Hold,0.05f);
	GetWorldTimerManager().SetTimer(AdvanceTimer, this, &ACinematicImageManager::AdvanceCinematicImageSequence, Hold, false);
}
void ACinematicImageManager::SkipToEnd()
{
	if (CurrentVoiceOver)
	{
		CurrentVoiceOver->Stop();
		CurrentVoiceOver = nullptr;
	}
	GetWorldTimerManager().ClearTimer(AdvanceTimer);
	FinishCinematicImageSequence();
}

