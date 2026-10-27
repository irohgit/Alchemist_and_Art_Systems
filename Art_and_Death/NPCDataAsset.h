// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Structures.h"
#include "NPCDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class AAD_V01_API UNPCDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly) EFaction Faction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FEnemy Enemy;
};
