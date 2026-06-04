// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FracturedStarsGameMode.generated.h"

/**
 * FracturedStars Game Mode
 * 
 * Purpose:
 * - Sets default controller and pawn classes for FracturedStars
 * - Handles player spawning and initialization
 * - Server-side game rules and session management
 * 
 * Responsibilities:
 * - Spawn AFracturedStarsPlayerController for joining players
 * - Spawn AUniverseCameraPawn for each player
 * - Future: Handle game session lifecycle
 * - Future: Manage server-side game rules
 */
UCLASS()
class FRACTUREDSTARS_API AFracturedStarsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFracturedStarsGameMode();

	// AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	// Future: Game session configuration
	// Future: Server-side rule enforcement
};
