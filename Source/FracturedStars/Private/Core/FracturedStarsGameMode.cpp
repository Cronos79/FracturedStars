// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/FracturedStarsGameMode.h"
#include "Player/FracturedStarsPlayerController.h"
#include "Player/UniverseCameraPawn.h"
#include "Universe/UniverseSubsystem.h"
#include "Visualization/SystemActor.h"

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

void AFracturedStarsGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::BeginPlay - Initializing galaxy visualization"));

	// Sprint 8: Spawn galaxy visualization
	SpawnGalaxyVisualization();
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

void AFracturedStarsGameMode::SpawnGalaxyVisualization()
{
	UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - Starting"));

	// Get universe subsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - No GameInstance!"));
		return;
	}

	UUniverseSubsystem* UniverseSubsystem = GameInstance->GetSubsystem<UUniverseSubsystem>();
	if (!UniverseSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - No UniverseSubsystem!"));
		return;
	}

	// Generate universe if it hasn't been generated yet
	if (!UniverseSubsystem->IsUniverseGenerated())
	{
		UE_LOG(LogTemp, Warning, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - Universe not generated, generating now..."));

		// Use default config (500 systems, 5 factions)
		FUniverseConfig Config;
		Config.SystemCount = 500;
		Config.FactionCount = 5;
		Config.Seed = FMath::Rand(); // Random seed each time, or use a fixed value for reproducible universes

		if (!UniverseSubsystem->GenerateUniverse(Config))
		{
			UE_LOG(LogTemp, Error, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - Failed to generate universe!"));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - Universe generated successfully"));
	}

	const FUniverseData& Universe = UniverseSubsystem->GetUniverseData();
	UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - Found %d systems"), Universe.Systems.Num());

	// Spawn ASystemActor for each system
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - No World!"));
		return;
	}

	// Scale factor for positioning systems in 3D space
	// Systems are stored in large galactic coordinates (100,000 unit radius)
	// Scale to 10,000 unit radius for balanced spacing (0.10 = 100,000 * 0.10 = 10,000)
	const float PositionScale = 0.25f;

	int32 SpawnedCount = 0;
	int32 FailedCount = 0;
	for (const FStarSystemData& System : Universe.Systems)
	{
		// Use system coordinates directly (already in FVector format)
		FVector WorldPosition = System.Coordinates * PositionScale;

		// Spawn system actor
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(*FString::Printf(TEXT("System_%d"), System.SystemId));
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASystemActor* SystemActor = World->SpawnActor<ASystemActor>(
			ASystemActor::StaticClass(),
			WorldPosition,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (SystemActor)
		{
			SystemActor->SystemId = System.SystemId;
			SystemActor->SystemName = System.SystemName;
			SpawnedCount++;

			// Log first few spawns to verify
			if (SpawnedCount <= 5)
			{
				UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - Spawned %s at %s"), 
					*SystemActor->GetName(), *WorldPosition.ToString());
			}
		}
		else
		{
			FailedCount++;
			UE_LOG(LogTemp, Warning, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - Failed to spawn actor for system %d at %s"), 
				System.SystemId, *WorldPosition.ToString());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FracturedStarsGameMode::SpawnGalaxyVisualization - Complete (spawned %d, failed %d, total %d systems)"), 
		SpawnedCount, FailedCount, Universe.Systems.Num());
}
