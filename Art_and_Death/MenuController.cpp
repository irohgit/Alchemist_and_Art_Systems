// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuController.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMenuController::BeginPlay()
{
	Super::BeginPlay();

	if (MainMenuWidgetEditorObject != nullptr)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetEditorObject);
		MainMenuWidget->AddToViewport(100);
		FInputModeUIOnly InputModeUIOnly;
		bShowMouseCursor = true;
		InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeUIOnly.SetWidgetToFocus(MainMenuWidget->TakeWidget());
		SetInputMode(InputModeUIOnly);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Please Set MainMenuWidgetEditorObject"));
	}
	
}

void AMenuController::BP_StartGame()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_1"), true);
}
