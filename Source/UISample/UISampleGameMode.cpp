// Copyright Epic Games, Inc. All Rights Reserved.

#include "UISampleGameMode.h"
#include "UISampleCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUISampleGameMode::AUISampleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
