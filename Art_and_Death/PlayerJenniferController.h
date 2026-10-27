// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerJenniferController.generated.h"

class UInputMappingContext;
class UUserWidget;
/**
 *  Basic PlayerController Class for FirstPerson Character Jennifer
 *  Manages Input Mappings
 */
UCLASS(abstract)
class AAD_V01_API APlayerJenniferController : public APlayerController
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings") TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category="Player") FName PlayerPawnTag = FName("Player");
	
	/** Gameplay initialization **/
	virtual void BeginPlay() override;

	/** Input mapping context setup*/
	virtual void SetupInputComponent() override;
};
