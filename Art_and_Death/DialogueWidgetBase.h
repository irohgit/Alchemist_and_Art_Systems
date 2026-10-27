// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidgetBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AAD_V01_API UDialogueWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetDialogueText(const FText& Speaker, const FString& Line);
	virtual void SetDialogueText_Implementation(const FText& Speaker, const FString& Line){}
};
