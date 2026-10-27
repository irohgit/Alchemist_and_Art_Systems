// Fill out your copyright notice in the Description page of Project Settings.


#include "CinematicGameModeBase.h"
#include "GameFramework/SpectatorPawn.h"

ACinematicGameModeBase::ACinematicGameModeBase()
{
	DefaultPawnClass = ASpectatorPawn::StaticClass();
}