// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/FracturedStarsGameInstance.h"

UFracturedStarsGameInstance::UFracturedStarsGameInstance()
{
	// Constructor
}

void UFracturedStarsGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameInstance::Init - Game Instance initialized"));

	// Future: Initialize save/load system
	// Future: Initialize session management
	// Future: Load meta-game state
}

void UFracturedStarsGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameInstance::Shutdown - Game Instance shutting down"));

	// Future: Save meta-game state
	// Future: Cleanup sessions

	Super::Shutdown();
}
