// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Structures.h"
#include "DialogueWidgetBase.h"
#include "DialogueManagerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AAD_V01_API UDialogueManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDialogueManagerComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue") TObjectPtr<UDataTable> DialogueTable;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue") TSubclassOf<UDialogueWidgetBase> DialogueWidgetClass;

	UFUNCTION(BlueprintCallable, Category="Dialogue") void StartDialogue();
	UFUNCTION(BlueprintCallable, Category="Dialogue") void StopDialogue();
	UFUNCTION(BlueprintCallable, Category="Dialogue") void NextLine();
	UFUNCTION(BlueprintCallable, Category="Dialogue") void StartAtRowName(FName RowName);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void CacheRowNames();
	void PlayCurrentLine();
	void AdvanceAuto();

	UPROPERTY(Transient)TObjectPtr<UDialogueWidgetBase> DialogueWidget;
	UPROPERTY(Transient)TArray<FName> RowNames;
	UPROPERTY(Transient)int32 CurrentIndex = 0;
	
	FTimerHandle AdvanceTimer;
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
