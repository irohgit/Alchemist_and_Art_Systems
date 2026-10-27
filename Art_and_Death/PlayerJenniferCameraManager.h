// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "PlayerJenniferCameraManager.generated.h"

/**
 * Basic First Person Camera Manager.
 * Limits min/max look pitch.
 */
UCLASS()
class AAD_V01_API APlayerJenniferCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	APlayerJenniferCameraManager();
};
