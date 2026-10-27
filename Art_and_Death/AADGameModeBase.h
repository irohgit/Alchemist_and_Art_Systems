// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AADGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class AAD_V01_API AAADGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	AAADGameModeBase();
	virtual void BeginPlay() override;
	
};
