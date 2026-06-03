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
 * Resource/commodity types for economy system (Sprint 3: renamed to Goods)
 * Used in production, consumption, and market trading
 */
UENUM(BlueprintType)
enum class EGoodType : uint8
{
	Food			UMETA(DisplayName = "Food"),
	Water			UMETA(DisplayName = "Water"),
	Fuel			UMETA(DisplayName = "Fuel"),
	Ore				UMETA(DisplayName = "Ore"),
	RefinedMetals	UMETA(DisplayName = "Refined Metals"),
	Medicine		UMETA(DisplayName = "Medicine"),
	Machinery		UMETA(DisplayName = "Machinery"),
	Electronics		UMETA(DisplayName = "Electronics"),
	Weapons			UMETA(DisplayName = "Weapons"),
	ConsumerGoods	UMETA(DisplayName = "Consumer Goods"),
	IndustrialParts	UMETA(DisplayName = "Industrial Parts"),
	AdvancedComponents UMETA(DisplayName = "Advanced Components"),
	ResearchMaterials UMETA(DisplayName = "Research Materials"),
	Contraband		UMETA(DisplayName = "Contraband")		// Illegal/restricted
};

/**
 * Good category for economy rules
 */
UENUM(BlueprintType)
enum class EGoodCategory : uint8
{
	Survival		UMETA(DisplayName = "Survival"),		// Food, Water, Medicine
	Industrial		UMETA(DisplayName = "Industrial"),		// Ore, Machinery, Parts
	HighTech		UMETA(DisplayName = "High Tech"),		// Electronics, Advanced Components
	Military		UMETA(DisplayName = "Military"),		// Weapons, restricted tech
	Luxury			UMETA(DisplayName = "Luxury"),			// Consumer goods, entertainment
	Illegal			UMETA(DisplayName = "Illegal")			// Contraband, smuggled goods
};

/**
 * Good definition
 * Static properties of each tradeable good
 */
USTRUCT(BlueprintType)
struct FGoodDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	EGoodType GoodType = EGoodType::Food;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	EGoodCategory Category = EGoodCategory::Survival;

	// Base price in credits (market prices fluctuate around this)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float BasePrice = 10.0f;

	// Is this good essential for survival? (food, water, medicine, fuel)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	bool bIsEssential = false;

	// Is this good legal by default? (false = contraband)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	bool bIsLegal = true;

	// Cargo space per unit (for future hauling mechanics)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float CargoUnitSize = 1.0f;

	FGoodDefinition()
		: GoodType(EGoodType::Food)
		, Category(EGoodCategory::Survival)
		, BasePrice(10.0f)
		, bIsEssential(false)
		, bIsLegal(true)
		, CargoUnitSize(1.0f)
	{}
};

// Sprint 2 compatibility: Keep EResourceType as alias for now
typedef EGoodType EResourceType;

/**
 * Resource profile entry
 * Quantity of a specific resource type
 */
USTRUCT(BlueprintType)
struct FResourceEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	EGoodType ResourceType = EGoodType::Food;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 Quantity = 0;

	FResourceEntry()
		: ResourceType(EGoodType::Food)
		, Quantity(0)
	{}

	FResourceEntry(EGoodType InType, int32 InQuantity)
		: ResourceType(InType)
		, Quantity(InQuantity)
	{}
};

/**
 * Market good entry
 * Tracks inventory, pricing, and shortage/surplus for a single good at one location
 */
USTRUCT(BlueprintType)
struct FMarketGoodEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	EGoodType GoodType = EGoodType::Food;

	// Current inventory stock (units available for sale)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 Stock = 0;

	// Current market price in credits per unit
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float CurrentPrice = 10.0f;

	// Production per simulation cycle (for producers)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 ProductionRate = 0;

	// Consumption per simulation cycle (for consumers)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 ConsumptionRate = 0;

	// Target stock level for healthy operation
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 TargetStock = 100;

	// Shortage flag (stock below critical threshold)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	bool bIsShortage = false;

	// Surplus flag (stock above target)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	bool bIsSurplus = false;

	FMarketGoodEntry()
		: GoodType(EGoodType::Food)
		, Stock(0)
		, CurrentPrice(10.0f)
		, ProductionRate(0)
		, ConsumptionRate(0)
		, TargetStock(100)
		, bIsShortage(false)
		, bIsSurplus(false)
	{}

	FMarketGoodEntry(EGoodType InType, int32 InStock, float InPrice)
		: GoodType(InType)
		, Stock(InStock)
		, CurrentPrice(InPrice)
		, ProductionRate(0)
		, ConsumptionRate(0)
		, TargetStock(100)
		, bIsShortage(false)
		, bIsSurplus(false)
	{}
};

/**
 * Market state for a location
 * Tracks all tradeable goods and last simulation update
 */
USTRUCT(BlueprintType)
struct FMarketState
{
	GENERATED_BODY()

	// All goods traded at this market
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TArray<FMarketGoodEntry> Goods;

	// Last simulation time (game seconds since universe creation)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	double LastUpdateTime = 0.0;

	// Is this market currently simulated in real-time? (player in-system)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	bool bIsActiveSimulation = false;

	FMarketState()
		: LastUpdateTime(0.0)
		, bIsActiveSimulation(false)
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

	// Sprint 2: Future inventory storage (deprecated - use Market.Goods instead)
	UPROPERTY(BlueprintReadOnly, Category = "Location")
	TArray<FResourceEntry> Inventory;

	// Sprint 3: Market state with live inventory, prices, and shortages
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	FMarketState Market;

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
 * Universe time tracking
 * Represents the current in-game date and time for the universe simulation
 */
USTRUCT(BlueprintType)
struct FUniverseTime
{
	GENERATED_BODY()

	// Calendar date
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 Year;

	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 Month; // 1-12 (January = 1)

	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 Day; // 1-31

	// Time of day
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 Hour; // 0-23

	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 Minute; // 0-59

	UPROPERTY(BlueprintReadOnly, Category = "Time")
	int32 Second; // 0-59

	// Total elapsed time since start (in seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	double TotalElapsedSeconds;

	FUniverseTime()
		: Year(2094)
		, Month(7)
		, Day(8)
		, Hour(0)
		, Minute(0)
		, Second(0)
		, TotalElapsedSeconds(0.0)
	{}

	FUniverseTime(int32 InYear, int32 InMonth, int32 InDay, int32 InHour = 0, int32 InMinute = 0, int32 InSecond = 0)
		: Year(InYear)
		, Month(InMonth)
		, Day(InDay)
		, Hour(InHour)
		, Minute(InMinute)
		, Second(InSecond)
		, TotalElapsedSeconds(0.0)
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

	// Sprint 3: Economy simulation tick rate (seconds) for active systems
	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	float EconomyTickRate = 5.0f;

	// Sprint 3: Catch-up simulation chunk size (hours of game time per call)
	UPROPERTY(BlueprintReadWrite, Category = "Economy")
	float BackgroundSimulationChunk = 24.0f;

	// Game time configuration
	UPROPERTY(BlueprintReadWrite, Category = "Time")
	int32 StartYear = 2094;

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	int32 StartMonth = 7; // July

	UPROPERTY(BlueprintReadWrite, Category = "Time")
	int32 StartDay = 8;

	// Time scale: 1 real second = TimeScale game seconds
	// Default: 24.0 (1 real hour = 1 game day)
	UPROPERTY(BlueprintReadWrite, Category = "Time")
	float TimeScale = 24.0f;

	FUniverseConfig()
		: Seed(12345)
		, SystemCount(500)
		, FactionCount(5)
		, MinFactionSeparation(8)
		, MinLawlessBuffer(2)
		, AvgConnectionsPerSystem(3)
		, GalacticRadius(100000.0f)
		, EconomyTickRate(5.0f)
		, BackgroundSimulationChunk(24.0f)
		, StartYear(2094)
		, StartMonth(7)
		, StartDay(8)
		, TimeScale(24.0f)
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

	// Game time tracking
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	FUniverseTime CurrentTime;

	// Sprint 3: Active system ID for real-time economy simulation (-1 = none)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 ActiveSystemId = -1;

	FUniverseData()
		: ActiveSystemId(-1)
	{
		GenerationTime = FDateTime::Now();
	}
};
