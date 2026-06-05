// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UniverseTypes.h"
#include "EconomyTypes.generated.h"

/**
 * Sprint 5: Economic Simulation Types
 * 
 * Extends the universe economy with:
 * - Production profiles and recipes
 * - Consumption requirements
 * - Economic stress calculation
 * - Two-tier simulation (active/inactive)
 */

/**
 * Good categories for Sprint 5 organization
 * Maps to the spec's 7 categories
 */
UENUM(BlueprintType)
enum class EGoodCategoryV2 : uint8
{
	Population		UMETA(DisplayName = "Population"),		// Food, Water, Medicine, Consumer Goods, Clothing
	Industrial		UMETA(DisplayName = "Industrial"),		// Ore, Refined Metals, Machinery, Electronics, Parts
	Energy			UMETA(DisplayName = "Energy"),			// Fuel, Reactor Fuel, Energy Cells
	Construction	UMETA(DisplayName = "Construction"),	// Building Materials, Structural Components, Habitat Components
	ShipSupport		UMETA(DisplayName = "Ship Support"),	// Ship Parts, Advanced Components, Repair Materials
	Advanced		UMETA(DisplayName = "Advanced"),		// Research Materials, Medical Supplies, High Tech
	Illegal			UMETA(DisplayName = "Illegal")			// Contraband, Illegal Weapons, Restricted Tech
};

/**
 * Production recipe input requirement
 * Defines what a location needs to produce a good
 */
USTRUCT(BlueprintType)
struct FProductionInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	EGoodType GoodType = EGoodType::Ore;

	// Units required per production cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 UnitsRequired = 10;

	FProductionInput()
		: GoodType(EGoodType::Ore)
		, UnitsRequired(10)
	{}

	FProductionInput(EGoodType InType, int32 InUnits)
		: GoodType(InType)
		, UnitsRequired(InUnits)
	{}
};

/**
 * Production recipe definition
 * Defines how a location produces a good
 */
USTRUCT(BlueprintType)
struct FProductionRecipe
{
	GENERATED_BODY()

	// Good produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	EGoodType OutputGood = EGoodType::Machinery;

	// Units produced per cycle (when inputs are available)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 OutputQuantity = 50;

	// Required inputs (empty = no inputs required, e.g., raw extraction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	TArray<FProductionInput> Inputs;

	// Production efficiency multiplier (0.0 - 1.0)
	// 0.0 = stopped, 1.0 = full production
	// Calculated based on input availability
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float CurrentEfficiency = 1.0f;

	FProductionRecipe()
		: OutputGood(EGoodType::Machinery)
		, OutputQuantity(50)
		, CurrentEfficiency(1.0f)
	{}
};

/**
 * Consumption profile entry
 * Defines what a location consumes per cycle
 */
USTRUCT(BlueprintType)
struct FConsumptionEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	EGoodType GoodType = EGoodType::Food;

	// Base consumption per cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 BaseConsumption = 10;

	// Population scaling factor (consumption = base + (population * scale))
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float PopulationScale = 0.001f;

	// Is this consumption critical? (causes stress if unavailable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	bool bIsCritical = false;

	FConsumptionEntry()
		: GoodType(EGoodType::Food)
		, BaseConsumption(10)
		, PopulationScale(0.001f)
		, bIsCritical(false)
	{}

	FConsumptionEntry(EGoodType InType, int32 InBase, float InScale, bool bCritical)
		: GoodType(InType)
		, BaseConsumption(InBase)
		, PopulationScale(InScale)
		, bIsCritical(bCritical)
	{}
};

/**
 * Location economic profile
 * Defines production and consumption behavior for a location
 */
USTRUCT(BlueprintType)
struct FLocationEconomicProfile
{
	GENERATED_BODY()

	// Production recipes for this location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	TArray<FProductionRecipe> ProductionRecipes;

	// Consumption requirements
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	TArray<FConsumptionEntry> ConsumptionProfile;

	// Current economic stress level (0.0 = healthy, 1.0 = critical)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float EconomicStress = 0.0f;

	FLocationEconomicProfile()
		: EconomicStress(0.0f)
	{}
};

/**
 * Economic shortage record
 * Tracks critical shortages for reporting
 */
USTRUCT(BlueprintType)
struct FEconomicShortage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 SystemId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 LocationId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	EGoodType GoodType = EGoodType::Food;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 ShortageAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float StressContribution = 0.0f;

	FEconomicShortage()
		: SystemId(-1)
		, LocationId(-1)
		, GoodType(EGoodType::Food)
		, ShortageAmount(0)
		, StressContribution(0.0f)
	{}
};

/**
 * Economic surplus record
 * Tracks surpluses for trade opportunities
 */
USTRUCT(BlueprintType)
struct FEconomicSurplus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 SystemId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 LocationId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	EGoodType GoodType = EGoodType::Food;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 SurplusAmount = 0;

	FEconomicSurplus()
		: SystemId(-1)
		, LocationId(-1)
		, GoodType(EGoodType::Food)
		, SurplusAmount(0)
	{}
};

/**
 * System economic state
 * Tracks last tick time and active status for two-tier simulation
 */
USTRUCT(BlueprintType)
struct FSystemEconomicState
{
	GENERATED_BODY()

	// Last time (TotalElapsedSeconds) this system's economy was ticked
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	double LastTickSeconds = 0.0;

	// Is this system actively simulated? (player present)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	bool bIsActive = false;

	// Accumulated tick debt (for catch-up calculations)
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 TicksOwed = 0;

	FSystemEconomicState()
		: LastTickSeconds(0.0)
		, bIsActive(false)
		, TicksOwed(0)
	{}
};

/**
 * Economic simulation report
 * Debug and validation data
 */
USTRUCT(BlueprintType)
struct FEconomicSimulationReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	double ReportTime = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 ActiveSystemCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 TotalShortages = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 TotalSurpluses = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TArray<FEconomicShortage> TopShortages;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TArray<FEconomicSurplus> TopSurpluses;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TMap<EGoodType, int32> TotalProduction;

	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TMap<EGoodType, int32> TotalConsumption;

	FEconomicSimulationReport()
		: ReportTime(0.0)
		, ActiveSystemCount(0)
		, TotalShortages(0)
		, TotalSurpluses(0)
	{}
};
