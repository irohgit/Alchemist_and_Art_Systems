// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPCDataAsset.h"
#include "GameFramework/Character.h"
#include "Structures.h"
#include "NPCBaseCharacter.generated.h"

class UNPCDataAsset;

UCLASS()
class AAD_V01_API ANPCBaseCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	ANPCBaseCharacter();
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category="Data Asset") const TObjectPtr<UNPCDataAsset> NPCDataAssetConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Structure") FEnemy EnemyStructure;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void ApplyConfig(const UNPCDataAsset* InConfig);
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
