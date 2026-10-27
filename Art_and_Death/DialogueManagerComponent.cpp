// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueManagerComponent.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Subtitles/SubtitlesAndClosedCaptionsDelegates.h"

// Sets default values for this component's properties
UDialogueManagerComponent::UDialogueManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UDialogueManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	//CacheRowNames();
	
}


void UDialogueManagerComponent::CacheRowNames()
{
	RowNames.Reset();
	if (DialogueTable)
	{
		RowNames = DialogueTable->GetRowNames();

		//Ensure deterministic order: 0001_Line ... 000X_Line
		RowNames.Sort([](const FName& A, const FName& B)
		{
			return A.FastLess(B);	
		});
		
	}
}


void UDialogueManagerComponent::StartDialogue()
{
	if (!DialogueTable || !DialogueWidgetClass) return;
	if (!DialogueWidget)
	{	
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			DialogueWidget = CreateWidget<UDialogueWidgetBase>(PC, DialogueWidgetClass);
			if (DialogueWidget) DialogueWidget->AddToViewport();
		}
	}
	CacheRowNames();
	CurrentIndex = 0;
	GetWorld()->GetTimerManager().ClearTimer(AdvanceTimer);
	PlayCurrentLine();
}

void UDialogueManagerComponent::StopDialogue()
{
	GetWorld()->GetTimerManager().ClearTimer(AdvanceTimer);
	if (DialogueWidget)
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
	}
}

void UDialogueManagerComponent::StartAtRowName(FName RowName)
{
	//if (GEngine)GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Red,TEXT("Started Dialogue"));
	if (!DialogueTable || !DialogueWidgetClass) return;
	//if (GEngine)GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Red,TEXT("Started Dialogue 123 "));
	if (!DialogueWidget)
	{	
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			DialogueWidget = CreateWidget<UDialogueWidgetBase>(PC, DialogueWidgetClass);
			if (DialogueWidget) DialogueWidget->AddToViewport();
			//if (GEngine)GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Red,TEXT("Started Dialogue Viewport"));
		}
	}
	if (!DialogueTable || RowNames.Num() == 0) CacheRowNames();
	int32 Found = RowNames.IndexOfByKey(RowName);
	CurrentIndex = (Found != INDEX_NONE) ? Found : 0;
	GetWorld()->GetTimerManager().ClearTimer(AdvanceTimer);
	PlayCurrentLine();
}

void UDialogueManagerComponent::PlayCurrentLine()
{
	//if (GEngine)GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Red,TEXT("Play current line start"));
	if (!DialogueTable || !DialogueWidget) return;
	if (!RowNames.IsValidIndex(CurrentIndex))
	{
		StopDialogue();
		return;
	}
	const FName RowName = RowNames[CurrentIndex];
	const FDialogueLine* Row = DialogueTable->FindRow<FDialogueLine>(RowName, TEXT("Dialogue"));
	if (!Row)
	{
		++CurrentIndex;
		PlayCurrentLine();
		return;
	}
	const FText Speaker = Row->SpeakerName;
	const FString Line = Row->DialogueLine;
	DialogueWidget->SetDialogueText(Speaker,Line);
	//if (GEngine)GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Red,TEXT("Set Line"));
	const float Delay = (Row->Duration>0.f) ? Row-> Duration : 2.0f;
	GetWorld()->GetTimerManager().SetTimer(
		AdvanceTimer,this, &UDialogueManagerComponent::AdvanceAuto, Delay, false);
}

void UDialogueManagerComponent::AdvanceAuto()
{
	++CurrentIndex;
	if (GEngine)GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Red,TEXT("ADVANCING"),true);
	PlayCurrentLine();
}

void UDialogueManagerComponent::NextLine()
{
	GetWorld()->GetTimerManager().ClearTimer(AdvanceTimer);
	AdvanceAuto();
}


// Called every frame
void UDialogueManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


