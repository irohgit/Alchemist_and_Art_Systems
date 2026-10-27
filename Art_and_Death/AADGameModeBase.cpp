// Fill out your copyright notice in the Description page of Project Settings.


#include "AADGameModeBase.h"
#include "PlayerJenniferCharacter.h"

AAADGameModeBase::AAADGameModeBase()
{
	//Set Default Character
	DefaultPawnClass = APlayerJenniferCharacter::StaticClass();
}


void AAADGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}
