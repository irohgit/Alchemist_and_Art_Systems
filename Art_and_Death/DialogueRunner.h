// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueRunner.generated.h"

class UDialogueManagerComponent;

UCLASS()
class AAD_V01_API ADialogueRunner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADialogueRunner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Dialogue")
	UDialogueManagerComponent* DialogueManager;

	UFUNCTION(BlueprintCallable, Category="Dialogue") void CallDialogue();
};
