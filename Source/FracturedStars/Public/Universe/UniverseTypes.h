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
 * Celestial body types
 * Stars, planets, moons, and other space objects
 */
UENUM(BlueprintType)
enum class ECelestialBodyType : uint8
{
	Star			UMETA(DisplayName = "Star"),
	Planet			UMETA(DisplayName = "Planet"),
	Moon			UMETA(DisplayName = "Moon"),
	AsteroidField	UMETA(DisplayName = "Asteroid Field"),
	GasGiant		UMETA(DisplayName = "Gas Giant"),
	IceGiant		UMETA(DisplayName = "Ice Giant"),
	DwarfPlanet		UMETA(DisplayName = "Dwarf Planet"),
	Anomaly			UMETA(DisplayName = "Anomaly")
};

/**
 * Location types for player-accessible places
 * Stations, colonies, outposts, and facilities
 */
UENUM(BlueprintType)
enum class ELocationType : uint8
{
	Station			UMETA(DisplayName = "Space Station"),		// General purpose station
	TradeHub		UMETA(DisplayName = "Trade Hub"),			// Major commerce center
	MiningColony	UMETA(DisplayName = "Mining Colony"),		// Resource extraction
	ResearchFacility UMETA(DisplayName = "Research Facility"),	// Science and development
	MilitaryBase	UMETA(DisplayName = "Military Base"),		// Armed forces installation
	PirateOutpost	UMETA(DisplayName = "Pirate Outpost"),		// Criminal hideout
	AbandonedFacility UMETA(DisplayName = "Abandoned Facility"),// Derelict structure
	Shipyard		UMETA(DisplayName = "Shipyard"),			// Ship construction
	RefuelingDepot	UMETA(DisplayName = "Refueling Depot"),		// Fuel and supplies
	Colony			UMETA(DisplayName = "Colony")				// Civilian settlement
};

/**
 * Resource/commodity types for economy system
 * Used in production and consumption profiles
 */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Food			UMETA(DisplayName = "Food"),
	Water			UMETA(DisplayName = "Water"),
	Fuel			UMETA(DisplayName = "Fuel"),
	Ore				UMETA(DisplayName = "Ore"),
	RareMetals		UMETA(DisplayName = "Rare Metals"),
	Medicine		UMETA(DisplayName = "Medicine"),
	Machinery		UMETA(DisplayName = "Machinery"),
	Electronics		UMETA(DisplayName = "Electronics"),
	Weapons			UMETA(DisplayName = "Weapons"),
	Luxuries		UMETA(DisplayName = "Luxuries"),
	IndustrialParts	UMETA(DisplayName = "Industrial Parts"),
	ChemicalCompounds UMETA(DisplayName = "Chemical Compounds")
};

/**
 * Resource profile entry
 * Quantity of a specific resource type
 */
USTRUCT(BlueprintType)
struct FResourceEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	EResourceType ResourceType = EResourceType::Food;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 Quantity = 0;

	FResourceEntry()
		: ResourceType(EResourceType::Food)
		, Quantity(0)
	{}

	FResourceEntry(EResourceType InType, int32 InQuantity)
		: ResourceType(InType)
		, Quantity(InQuantity)
	{}
};

/**
 * Celestial body data
 * Represents stars, planets, moons, and other space objects
 */
USTRUCT(BlueprintType)
struct FCelestialBodyData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Celestial")
	int32 BodyId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Celestial")
	FString BodyName;

	UPROPERTY(BlueprintReadOnly, Category = "Celestial")
	ECelestialBodyType BodyType = ECelestialBodyType::Planet;

	// Parent body ID (-1 for stars, planet ID for moons)
	UPROPERTY(BlueprintReadOnly, Category = "Celestial")
	int32 ParentBodyId = -1;

	// Optional resource richness (0.0 - 1.0)
	UPROPERTY(BlueprintReadOnly, Category = "Celestial")
	float ResourceRichness = 0.5f;

	FCelestialBodyData()
		: BodyId(-1)
		, BodyType(ECelestialBodyType::Planet)
		, ParentBodyId(-1)
		, ResourceRichness(0.5f)
	{}
};

/**
 * Location data
 * Represents player-accessible stations, colonies, and facilities
 */
USTRUCT(BlueprintType)
struct FLocationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Location")
	int32 LocationId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Location")
	FString LocationName;

	UPROPERTY(BlueprintReadOnly, Category = "Location")
	ELocationType LocationType = ELocationType::Station;

	// Owning faction ID (-1 = independent/unowned)
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	int32 OwningFactionId = -1;

	// Population estimate
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	int32 Population = 0;

	// Security rating (0.0 = no security, 1.0 = maximum security)
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	float SecurityRating = 0.5f;

	// Economy hooks - resources this location produces
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	TArray<FResourceEntry> Produces;

	// Economy hooks - resources this location consumes
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	TArray<FResourceEntry> Consumes;

	// Future: inventory storage
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	TArray<FResourceEntry> Inventory;

	FLocationData()
		: LocationId(-1)
		, LocationType(ELocationType::Station)
		, OwningFactionId(-1)
		, Population(0)
		, SecurityRating(0.5f)
	{}
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

	// Celestial bodies in this system (stars, planets, moons, etc.)
	UPROPERTY(BlueprintReadOnly, Category = "Content")
	TArray<FCelestialBodyData> CelestialBodies;

	// Locations in this system (stations, colonies, outposts, etc.)
	UPROPERTY(BlueprintReadOnly, Category = "Content")
	TArray<FLocationData> Locations;

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
