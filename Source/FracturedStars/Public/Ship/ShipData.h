// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Universe/UniverseTypes.h"
#include "Ship/ShipTypes.h"
#include "ShipData.generated.h"

/**
 * Ship Operational Status
 * Independent of player/NPC/crew ownership
 */
UENUM(BlueprintType)
enum class EShipStatus : uint8
{
	Docked       UMETA(DisplayName = "Docked"),         // Ship is docked at a location
	InTransit    UMETA(DisplayName = "In Transit"),    // Ship is traveling between systems (future)
	Active       UMETA(DisplayName = "Active"),        // Ship is operational
	Disabled     UMETA(DisplayName = "Disabled"),      // Ship is damaged/disabled (future)
	Destroyed    UMETA(DisplayName = "Destroyed")      // Ship is destroyed (future)
};

/**
 * Persistent ship data
 * Represents a physical ship in the universe
 * 
 * DESIGN PHILOSOPHY:
 * - Ships are independent of owner type (player, NPC, crew)
 * - NPCs load Ship + NPC modules (not Player)
 * - Players load Ship + Player modules (not NPC)
 * - Crew loads Ship + Crew modules
 * - Ship = Frame + Installed Components (Sprint 5.5)
 */
USTRUCT(BlueprintType)
struct FRACTUREDSTARS_API FShipData
{
	GENERATED_BODY()

	/** Unique ship identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	int32 ShipId = -1;

	/** Ship display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	FString ShipName;

	/** Owner ID (player, NPC, faction, etc. - interpretation depends on OwnerType) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Ownership")
	int32 OwnerId = -1;

	/** Owner type tag (e.g., "Player", "NPC", "Faction", "Pirate") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Ownership")
	FName OwnerType;

	/** Current system the ship is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Location")
	int32 CurrentSystemId = -1;

	/** Current location within system (station/planet/space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Location")
	int32 CurrentLocationId = -1;

	/** Ship operational status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	EShipStatus Status = EShipStatus::Docked;

	// ========================================================================
	// SPRINT 5.5: Component-Based Ship Architecture
	// ========================================================================

	/** Ship frame definition ID (determines slot layout and base stats) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame")
	FName FrameId;

	/** Hull condition (0-100%, affects structural integrity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Frame")
	float HullCondition = 100.0f;

	/** Installed components (engines, power plants, shields, weapons, utility) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Components")
	TArray<FInstalledComponent> InstalledComponents;

	// ========================================================================
	// CALCULATED STATS (derived from frame + components)
	// These are recalculated when components change
	// ========================================================================

	/** Maximum cargo capacity (tons or units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats")
	int32 CargoCapacity = 100;

	/** Current cargo weight/volume used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Cargo")
	int32 CurrentCargoUsed = 0;

	/** Maximum fuel capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats")
	int32 FuelCapacity = 100;

	/** Current fuel remaining */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Fuel")
	int32 CurrentFuel = 100;

	/** Maximum crew capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats")
	int32 CrewCapacity = 3;

	/** List of crew IDs currently assigned to this ship */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Crew")
	TArray<int32> AssignedCrewIds;

	/** Total power generation (from power plants) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats|Power")
	int32 PowerGeneration = 0;

	/** Total power consumption (from all systems) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats|Power")
	int32 PowerConsumption = 0;

	/** Speed modifier (from engines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats|Performance")
	float SpeedModifier = 1.0f;

	/** Shield capacity (from shield generators) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats|Defense")
	int32 ShieldCapacity = 0;

	/** Current shield strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats|Defense")
	int32 CurrentShields = 0;

	/** Total weapon damage potential */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats|Offense")
	int32 WeaponDamage = 0;

	/** Mining efficiency (0.0-1.0 multiplier from mining lasers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats|Utility")
	float MiningEfficiency = 0.0f;

	/** Sensor range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats|Utility")
	int32 SensorRange = 0;

	/** Is the ship operational? (has required components + valid power) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Stats")
	bool bIsOperational = false;

	// ========================================================================
	// CARGO INVENTORY
	// ========================================================================

	/** Cargo goods stored on ship (good type -> quantity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Cargo")
	TMap<EGoodType, int32> CargoInventory;

	// Future expansion placeholders:
	// - Combat history
	// - Jump drive charge state
	// - Damage map (which components are damaged)
	// - Reputation/fame for this specific ship
};
