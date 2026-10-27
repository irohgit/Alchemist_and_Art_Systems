// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCBaseCharacter.h"
#include "Enumerators.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ANPCBaseCharacter::ANPCBaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(55.f,96.0f);
	GetCapsuleComponent()->SetCapsuleSize(34.0f,96.0f);
}

// Called when the game starts or when spawned
void ANPCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(NPCDataAssetConfig))
	{
		ApplyConfig(NPCDataAssetConfig);
	}
}

// Called every frame
void ANPCBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ANPCBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANPCBaseCharacter::ApplyConfig(const UNPCDataAsset* InConfig)
{
	//Apply Faction
	if (IsValid(InConfig))
	{
		switch (InConfig->Faction)
		{
		case EFaction::Enemy:
			GetCharacterMovement()->MaxWalkSpeed = InConfig->Enemy.MaxWalkSpeed;
		}
	}
}