// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiFPSGameMode.h"
#include "MultiFPSCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMultiFPSGameMode::AMultiFPSGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
