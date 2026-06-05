// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Universe/UniverseTypes.h"
#include "ShipTypes.generated.h"

/**
 * Ship Framework & Component Economy
 * 
 * DESIGN PHILOSOPHY:
 * Ships are assemblies, not predefined objects.
 * 
 * Ship = Frame + Components
 * 
 * - Frames provide structure (slots, mass, base capacity)
 * - Components provide capability (engines, power, weapons, etc.)
 * - Components are economic goods (manufactured, traded, consumed)
 * - All ships (player, NPC, pirate, military) use the same architecture
 * 
 * Sprint 5.5 Foundation
 */

// ============================================================================
// ENUMS
// ============================================================================

/**
 * Component slot size categories
 * Determines what components can be installed in which slots
 */
UENUM(BlueprintType)
enum class ESlotSize : uint8
{
	Small       UMETA(DisplayName = "Small"),        // Starter ships, small craft
	Medium      UMETA(DisplayName = "Medium"),       // Standard freighters, escorts
	Large       UMETA(DisplayName = "Large"),        // Heavy freighters, cruisers
	Capital     UMETA(DisplayName = "Capital")       // Capital ships, carriers, battleships
};

/**
 * Component slot type/purpose
 * Defines what kind of component can be installed
 * 
 * DESIGN PHILOSOPHY:
 * - Core systems (Engine, PowerPlant, Weapon, Shield) have dedicated typed slots
 * - Utility slots are universal: cargo, fuel, mining, sensors, tractor, refinery, etc.
 * - Players make tradeoff choices: "Do I use my 2 utility slots for cargo+mining or both cargo?"
 */
UENUM(BlueprintType)
enum class ESlotType : uint8
{
	Engine          UMETA(DisplayName = "Engine"),           // Required - provides movement
	PowerPlant      UMETA(DisplayName = "Power Plant"),      // Required - provides power
	Shield          UMETA(DisplayName = "Shield"),           // Recommended - provides shields
	Weapon          UMETA(DisplayName = "Weapon"),           // Optional - combat capability
	Utility         UMETA(DisplayName = "Utility")           // Universal: cargo, fuel, mining, sensors, etc.
};

/**
 * Ship frame class/category
 * Defines the general purpose and scale of the ship
 */
UENUM(BlueprintType)
enum class EShipFrameClass : uint8
{
	// Small Ships (gate-dependent FTL)
	StarterMining      UMETA(DisplayName = "Starter Mining"),
	StarterTrade       UMETA(DisplayName = "Starter Trade"),
	StarterFighter     UMETA(DisplayName = "Starter Fighter"),

	// Medium Ships (primarily gate-dependent)
	MediumFreighter    UMETA(DisplayName = "Medium Freighter"),
	MediumMiner        UMETA(DisplayName = "Medium Miner"),
	Escort             UMETA(DisplayName = "Escort"),

	// Large Ships (can support independent FTL)
	HeavyFreighter     UMETA(DisplayName = "Heavy Freighter"),
	Cruiser            UMETA(DisplayName = "Cruiser"),
	IndustrialShip     UMETA(DisplayName = "Industrial Ship"),

	// Capital Ships (independent FTL, fleet jump capability)
	Carrier            UMETA(DisplayName = "Carrier"),
	Battleship         UMETA(DisplayName = "Battleship"),
	Dreadnought        UMETA(DisplayName = "Dreadnought")
};

/**
 * Component type categories
 * Defines what kind of capability a component provides
 */
UENUM(BlueprintType)
enum class EShipComponentType : uint8
{
	// Required Components
	Engine             UMETA(DisplayName = "Engine"),
	PowerPlant         UMETA(DisplayName = "Power Plant"),

	// Defense Components
	Shield             UMETA(DisplayName = "Shield Generator"),

	// Weapon Components
	Laser              UMETA(DisplayName = "Laser Weapon"),
	Cannon             UMETA(DisplayName = "Cannon"),
	MissileLauncher    UMETA(DisplayName = "Missile Launcher"),

	// Utility Components
	MiningLaser        UMETA(DisplayName = "Mining Laser"),
	CargoExpansion     UMETA(DisplayName = "Cargo Expansion"),
	FuelTank           UMETA(DisplayName = "Fuel Tank"),
	SensorArray        UMETA(DisplayName = "Sensor Array"),
	TractorBeam        UMETA(DisplayName = "Tractor Beam"),
	RefineryModule     UMETA(DisplayName = "Refinery Module"),
	SalvageEquipment   UMETA(DisplayName = "Salvage Equipment"),
	RepairDrone        UMETA(DisplayName = "Repair Drone")
};

/**
 * Component quality/generation tier
 * Higher tiers = better performance, higher cost
 */
UENUM(BlueprintType)
enum class EComponentTier : uint8
{
	TierI       UMETA(DisplayName = "Tier I"),      // Basic, common
	TierII      UMETA(DisplayName = "Tier II"),     // Improved
	TierIII     UMETA(DisplayName = "Tier III"),    // Advanced
	TierIV      UMETA(DisplayName = "Tier IV"),     // High-end
	TierV       UMETA(DisplayName = "Tier V")       // Elite
};

/**
 * FTL capability flags
 * Determines how a ship can perform faster-than-light travel
 */
UENUM(BlueprintType)
enum class EFTLCapability : uint8
{
	GateOnly           UMETA(DisplayName = "Gate Only"),          // Must use jump gates (small ships)
	GatePrimary        UMETA(DisplayName = "Gate Primary"),       // Primarily gates, limited independent (medium)
	Independent        UMETA(DisplayName = "Independent"),        // Can jump independently (large ships)
	FleetJump          UMETA(DisplayName = "Fleet Jump")          // Can generate jumps for escort fleet (capitals)
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Component slot definition
 * Defines a single slot on a ship frame
 */
USTRUCT(BlueprintType)
struct FComponentSlot
{
	GENERATED_BODY()

	/** Slot identifier (unique within a frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Slot")
	FName SlotId;

	/** Slot size (Small/Medium/Large/Capital) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Slot")
	ESlotSize SlotSize = ESlotSize::Small;

	/** Slot type (Engine/PowerPlant/Weapon/etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Slot")
	ESlotType SlotType = ESlotType::Utility;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Slot")
	FText SlotName;

	FComponentSlot()
		: SlotId(NAME_None)
		, SlotSize(ESlotSize::Small)
		, SlotType(ESlotType::Utility)
		, SlotName(FText::FromString("Unknown Slot"))
	{}

	FComponentSlot(FName InId, ESlotSize InSize, ESlotType InType, const FText& InName)
		: SlotId(InId)
		, SlotSize(InSize)
		, SlotType(InType)
		, SlotName(InName)
	{}
};

/**
 * Ship frame definition
 * Defines the structure and slot layout of a ship hull
 * 
 * DESIGN PHILOSOPHY:
 * Each faction/race has unique frame types with different specializations.
 * Example: Human Sparrow (speed-focused, fewer weapons, more fuel/range)
 *          Khral Scavenger (utility-focused, many utility slots, weaker combat)
 */
USTRUCT(BlueprintType)
struct FShipFrameDefinition
{
	GENERATED_BODY()

	/** Unique frame identifier (e.g., "Human_Sparrow", "Khral_Scavenger") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame")
	FName FrameId;

	/** Frame display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame")
	FText FrameName;

	/** Faction/race that produces this frame (e.g., "Human", "Khral", "Syndicate") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame")
	FName FactionId;

	/** Frame class/category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame")
	EShipFrameClass FrameClass = EShipFrameClass::StarterMining;

	/** Hull integrity points (future combat/damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|Stats")
	int32 HullIntegrity = 1000;

	/** Base mass in tons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|Stats")
	int32 BaseMass = 100;

	/** Base cargo capacity (before component modifiers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|Stats")
	int32 BaseCargoCapacity = 50;

	/** Base fuel capacity (before component modifiers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|Stats")
	int32 BaseFuelCapacity = 50;

	/** Base crew capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|Stats")
	int32 BaseCrewCapacity = 2;

	/** FTL travel capability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|FTL")
	EFTLCapability FTLCapability = EFTLCapability::GateOnly;

	/** Component slots available on this frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|Slots")
	TArray<FComponentSlot> Slots;

	/** Economic good type for this frame (for manufacturing/trading) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|Economy")
	EGoodType EconomicGoodType = EGoodType::AdvancedComponents;

	/** Base manufacturing cost in credits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame|Economy")
	int32 BaseManufacturingCost = 10000;

	FShipFrameDefinition()
		: FrameId(NAME_None)
		, FrameName(FText::FromString("Unknown Frame"))
		, FrameClass(EShipFrameClass::StarterMining)
		, HullIntegrity(1000)
		, BaseMass(100)
		, BaseCargoCapacity(50)
		, BaseFuelCapacity(50)
		, BaseCrewCapacity(2)
		, FTLCapability(EFTLCapability::GateOnly)
		, EconomicGoodType(EGoodType::AdvancedComponents)
		, BaseManufacturingCost(10000)
	{}
};

/**
 * Ship component definition
 * Defines a specific component that can be installed on a ship
 */
USTRUCT(BlueprintType)
struct FShipComponentDefinition
{
	GENERATED_BODY()

	/** Unique component identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	FName ComponentId;

	/** Component display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	FText ComponentName;

	/** Component type category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	EShipComponentType ComponentType = EShipComponentType::Engine;

	/** Component size (determines compatible slots) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	ESlotSize ComponentSize = ESlotSize::Small;

	/** Component quality tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	EComponentTier Tier = EComponentTier::TierI;

	// ========== Stats ==========

	/** Power generation (for power plants, positive value) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	int32 PowerGeneration = 0;

	/** Power consumption (for all other components, positive value) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	int32 PowerConsumption = 0;

	/** Speed/acceleration modifier (for engines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	float SpeedModifier = 0.0f;

	/** Shield capacity (for shield generators) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	int32 ShieldCapacity = 0;

	/** Shield recharge rate per second (for shield generators) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	int32 ShieldRechargeRate = 0;

	/** Weapon damage (for weapon components) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	int32 WeaponDamage = 0;

	/** Cargo capacity bonus (for cargo modules) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	int32 CargoBonus = 0;

	/** Fuel capacity bonus (for fuel modules) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	int32 FuelBonus = 0;

	/** Sensor range bonus (for sensor arrays) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	int32 SensorRange = 0;

	/** Mining efficiency (for mining lasers, 0.0-1.0 multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Stats")
	float MiningEfficiency = 0.0f;

	// ========== Condition ==========

	/** Maximum condition value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Condition")
	int32 MaxCondition = 100;

	// ========== Economy ==========

	/** Economic good type for this component (for manufacturing/trading) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Economy")
	EGoodType EconomicGoodType = EGoodType::AdvancedComponents;

	/** Base manufacturing cost in credits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Economy")
	int32 BaseManufacturingCost = 1000;

	/** Mass in tons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component|Economy")
	int32 Mass = 5;

	FShipComponentDefinition()
		: ComponentId(NAME_None)
		, ComponentName(FText::FromString("Unknown Component"))
		, ComponentType(EShipComponentType::Engine)
		, ComponentSize(ESlotSize::Small)
		, Tier(EComponentTier::TierI)
		, PowerGeneration(0)
		, PowerConsumption(0)
		, SpeedModifier(0.0f)
		, ShieldCapacity(0)
		, ShieldRechargeRate(0)
		, WeaponDamage(0)
		, CargoBonus(0)
		, FuelBonus(0)
		, SensorRange(0)
		, MiningEfficiency(0.0f)
		, MaxCondition(100)
		, EconomicGoodType(EGoodType::AdvancedComponents)
		, BaseManufacturingCost(1000)
		, Mass(5)
	{}
};

/**
 * Installed component instance
 * Represents a specific component installed on a specific ship
 */
USTRUCT(BlueprintType)
struct FInstalledComponent
{
	GENERATED_BODY()

	/** Slot this component is installed in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	FName SlotId;

	/** Component definition reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	FName ComponentDefinitionId;

	/** Current condition (0-MaxCondition) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	int32 CurrentCondition = 100;

	/** Timestamp when this component was installed (Unix timestamp) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Component")
	int64 InstalledTime = 0;

	FInstalledComponent()
		: SlotId(NAME_None)
		, ComponentDefinitionId(NAME_None)
		, CurrentCondition(100)
		, InstalledTime(0)
	{}

	FInstalledComponent(FName InSlotId, FName InComponentId, int32 InCondition)
		: SlotId(InSlotId)
		, ComponentDefinitionId(InComponentId)
		, CurrentCondition(InCondition)
		, InstalledTime(0)
	{}
};

/**
 * Calculated ship statistics
 * Derived from frame + installed components
 */
USTRUCT(BlueprintType)
struct FShipCalculatedStats
{
	GENERATED_BODY()

	// ========== Mass & Capacity ==========

	/** Total mass (frame + components + cargo) */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats")
	int32 TotalMass = 0;

	/** Final cargo capacity */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats")
	int32 CargoCapacity = 0;

	/** Final fuel capacity */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats")
	int32 FuelCapacity = 0;

	/** Final crew capacity */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats")
	int32 CrewCapacity = 0;

	// ========== Power ==========

	/** Total power generation */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Power")
	int32 PowerGeneration = 0;

	/** Total power consumption */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Power")
	int32 PowerConsumption = 0;

	/** Available power (Generation - Consumption) */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Power")
	int32 AvailablePower = 0;

	/** Is power budget valid? (Generation >= Consumption) */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Power")
	bool bPowerBudgetValid = false;

	// ========== Performance ==========

	/** Speed modifier (from engines) */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Performance")
	float SpeedModifier = 0.0f;

	/** Acceleration (from engines) */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Performance")
	float Acceleration = 0.0f;

	// ========== Defense ==========

	/** Total shield capacity */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Defense")
	int32 ShieldCapacity = 0;

	/** Total shield recharge rate */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Defense")
	int32 ShieldRechargeRate = 0;

	// ========== Offense ==========

	/** Total weapon damage potential */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Offense")
	int32 WeaponDamage = 0;

	// ========== Utility ==========

	/** Sensor range */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Utility")
	int32 SensorRange = 0;

	/** Mining efficiency */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Utility")
	float MiningEfficiency = 0.0f;

	// ========== Validation ==========

	/** Does the ship have a functional engine? */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Validation")
	bool bHasEngine = false;

	/** Does the ship have a functional power plant? */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Validation")
	bool bHasPowerPlant = false;

	/** Is the ship operational? (has engine + power + valid power budget) */
	UPROPERTY(BlueprintReadOnly, Category = "Ship|Stats|Validation")
	bool bIsOperational = false;

	FShipCalculatedStats()
		: TotalMass(0)
		, CargoCapacity(0)
		, FuelCapacity(0)
		, CrewCapacity(0)
		, PowerGeneration(0)
		, PowerConsumption(0)
		, AvailablePower(0)
		, bPowerBudgetValid(false)
		, SpeedModifier(0.0f)
		, Acceleration(0.0f)
		, ShieldCapacity(0)
		, ShieldRechargeRate(0)
		, WeaponDamage(0)
		, SensorRange(0)
		, MiningEfficiency(0.0f)
		, bHasEngine(false)
		, bHasPowerPlant(false)
		, bIsOperational(false)
	{}
};
