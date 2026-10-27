// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueRunner.h"

#include "DialogueManagerComponent.h"


// Sets default values
ADialogueRunner::ADialogueRunner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ADialogueRunner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADialogueRunner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADialogueRunner::CallDialogue()
{
	if (DialogueManager)
	{
		DialogueManager->StartDialogue();
	}
}
