// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuController.generated.h"

/**
 * 
 */
UCLASS()
class AAD_V01_API AMenuController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI") TSubclassOf<UUserWidget> MainMenuWidgetEditorObject = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI") TObjectPtr<UUserWidget>   MainMenuWidget;
	UFUNCTION(BlueprintCallable, Category="GameManagement")			                         void BP_StartGame();
};
