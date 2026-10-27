// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PlayerJenniferCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * A simple player-controllable first person character
 *  Implements a controllable camera set at the players head
 */ 
UCLASS(abstract)
class AAD_V01_API APlayerJenniferCharacter : public ACharacter
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true")) UCameraComponent* FirstPersonCameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true")) USkeletalMeshComponent* FirstPersonMesh;

		
	
public:
	// Sets default values for this character's properties & Constructor
	APlayerJenniferCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	FORCEINLINE USkeletalMeshComponent* GetFirstPersonMesh() const {return FirstPersonMesh;}
	FORCEINLINE UCameraComponent* GetFirstPersonCameraComponent() const {return FirstPersonCameraComponent;}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Input") UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category="Input") UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category="Input") UInputAction* MouseLookAction;


	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, Category="Input") virtual void DoMove(float Right, float Forward);
	UFUNCTION(BlueprintCallable, Category="Input") virtual void DoLook(float Yaw, float Pitch);

public:


	
};
