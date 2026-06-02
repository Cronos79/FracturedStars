// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UniverseTypes.generated.h"

/**
 * Region type classification for star systems
 * Determines security, faction presence, and gameplay opportunities
 */
UENUM(BlueprintType)
enum class ERegionType : uint8
{
	FactionCore		UMETA(DisplayName = "Faction Core"),		// High security, faction capital regions
	FactionFrontier	UMETA(DisplayName = "Faction Frontier"),	// Moderate security, outer faction territory
	Neutral			UMETA(DisplayName = "Neutral"),				// Independent, moderate risk
	Lawless			UMETA(DisplayName = "Lawless"),				// No law enforcement, high piracy risk
	Disputed		UMETA(DisplayName = "Disputed"),			// Contested territory, war zones
	Unknown			UMETA(DisplayName = "Unknown")				// Unexplored or placeholder
};

/**
 * Star system data structure
 * Represents a single star system in the universe
 */
USTRUCT(BlueprintType)
struct FStarSystemData
{
	GENERATED_BODY()

	// Unique system identifier
	UPROPERTY(BlueprintReadOnly, Category = "System")
	int32 SystemId = -1;

	// Generated system name
	UPROPERTY(BlueprintReadOnly, Category = "System")
	FString SystemName;

	// Galactic coordinates (3D space position)
	UPROPERTY(BlueprintReadOnly, Category = "System")
	FVector Coordinates = FVector::ZeroVector;

	// Region classification
	UPROPERTY(BlueprintReadOnly, Category = "System")
	ERegionType RegionType = ERegionType::Unknown;

	// Lawfulness rating (0.0 = lawless pirate haven, 1.0 = maximum security)
	UPROPERTY(BlueprintReadOnly, Category = "System")
	float Lawfulness = 0.5f;

	// Faction ID that controls this system (-1 = no faction)
	UPROPERTY(BlueprintReadOnly, Category = "System")
	int32 ControllingFactionId = -1;

	// Connected system IDs (bidirectional jump network)
	UPROPERTY(BlueprintReadOnly, Category = "System")
	TArray<int32> ConnectedSystemIds;

	// Distance to faction core (for region assignment logic)
	UPROPERTY(BlueprintReadOnly, Category = "System")
	int32 DistanceToFactionCore = -1;

	FStarSystemData()
		: SystemId(-1)
		, Coordinates(FVector::ZeroVector)
		, RegionType(ERegionType::Unknown)
		, Lawfulness(0.5f)
		, ControllingFactionId(-1)
		, DistanceToFactionCore(-1)
	{}
};

/**
 * Universe generation configuration
 * Parameters controlling universe generation
 */
USTRUCT(BlueprintType)
struct FUniverseConfig
{
	GENERATED_BODY()

	// Random seed for deterministic generation
	UPROPERTY(BlueprintReadWrite, Category = "Generation")
	int32 Seed = 12345;

	// Total number of star systems to generate
	UPROPERTY(BlueprintReadWrite, Category = "Generation")
	int32 SystemCount = 500;

	// Number of major factions
	UPROPERTY(BlueprintReadWrite, Category = "Factions")
	int32 FactionCount = 5;

	// Minimum jump distance between faction home systems
	UPROPERTY(BlueprintReadWrite, Category = "Factions")
	int32 MinFactionSeparation = 8;

	// Minimum number of lawless systems between factions
	UPROPERTY(BlueprintReadWrite, Category = "Factions")
	int32 MinLawlessBuffer = 2;

	// Average number of connections per system (2-5 recommended)
	UPROPERTY(BlueprintReadWrite, Category = "Network")
	int32 AvgConnectionsPerSystem = 3;

	// Galactic radius for system placement
	UPROPERTY(BlueprintReadWrite, Category = "Generation")
	float GalacticRadius = 100000.0f;

	FUniverseConfig()
		: Seed(12345)
		, SystemCount(500)
		, FactionCount(5)
		, MinFactionSeparation(8)
		, MinLawlessBuffer(2)
		, AvgConnectionsPerSystem(3)
		, GalacticRadius(100000.0f)
	{}
};

/**
 * Universe data container
 * Holds all generated universe data
 */
USTRUCT(BlueprintType)
struct FUniverseData
{
	GENERATED_BODY()

	// Configuration used for generation
	UPROPERTY(BlueprintReadOnly, Category = "Universe")
	FUniverseConfig Config;

	// All star systems in the universe
	UPROPERTY(BlueprintReadOnly, Category = "Universe")
	TArray<FStarSystemData> Systems;

	// Faction home system IDs
	UPROPERTY(BlueprintReadOnly, Category = "Universe")
	TArray<int32> FactionHomeSystems;

	// Generation timestamp (for debugging)
	UPROPERTY(BlueprintReadOnly, Category = "Universe")
	FDateTime GenerationTime;

	FUniverseData()
	{
		GenerationTime = FDateTime::Now();
	}
};
