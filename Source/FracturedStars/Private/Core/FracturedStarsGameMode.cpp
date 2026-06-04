// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/FracturedStarsGameMode.h"
#include "Player/FracturedStarsPlayerController.h"
#include "Player/UniverseCameraPawn.h"

AFracturedStarsGameMode::AFracturedStarsGameMode()
{
	// Set default controller and pawn classes
	PlayerControllerClass = AFracturedStarsPlayerController::StaticClass();
	DefaultPawnClass = AUniverseCameraPawn::StaticClass();

	UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode: Configured with FracturedStarsPlayerController and UniverseCameraPawn"));
}

void AFracturedStarsGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::InitGame - Map: %s, Options: %s"), *MapName, *Options);

	// Future: Parse server options
	// Future: Initialize server-side game state
}

void AFracturedStarsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::PostLogin - Player %s joined"), *NewPlayer->GetName());

		// The controller/pawn spawning is handled by the base class
		// Future: Trigger player creation flow (UI or auto-create)
		// Future: Notify player subsystem of new connection
	}
}

void AFracturedStarsGameMode::Logout(AController* Exiting)
{
	if (Exiting)
	{
		UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::Logout - Player %s leaving"), *Exiting->GetName());

		// Future: Save player state
		// Future: Cleanup player subsystem entries
	}

	Super::Logout(Exiting);
}
