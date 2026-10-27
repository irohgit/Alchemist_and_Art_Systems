// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerJenniferController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"

void APlayerJenniferController::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerJenniferController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// Only adds Input Mapping Context for local player controlelrs
	if (IsLocalPlayerController())
	{
		//Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext,0);
			}
			//

			/*
			*			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
			 * 
			 */
		}
	}
}
