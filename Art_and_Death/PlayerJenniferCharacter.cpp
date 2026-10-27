// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerJenniferCharacter.h"
#include "AAD_V01.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
APlayerJenniferCharacter::APlayerJenniferCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(55.f,96.0f);

	FirstPersonMesh = GetMesh();
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f,21.f,-90.f),FRotator(-90.0f,0.0f,90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation		  = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale       = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView        = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale			  = 0.6f;

	// Configure character comps?
	GetMesh()-> SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f,96.0f);

	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	
//
	
}

// Called when the game starts or when spawned
void APlayerJenniferCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlayerJenniferCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerJenniferCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up Action Bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Move
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerJenniferCharacter::MoveInput);
		//LookAim
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerJenniferCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerJenniferCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogAAD_V01, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APlayerJenniferCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();
	//pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);
}

void APlayerJenniferCharacter::LookInput(const FInputActionValue& Value)
{
	// Get the Vector 2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	//Pass the axis values to the look input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void APlayerJenniferCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController())
	{
		//pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void APlayerJenniferCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}