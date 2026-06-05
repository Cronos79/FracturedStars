// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Universe/EconomyTypes.h"
#include "EconomicSimulationSubsystem.generated.h"

class UUniverseSubsystem;

/**
 * Economic Simulation Subsystem
 * 
 * Sprint 5: Living Economy
 * 
 * PURPOSE:
 * - Simulate production and consumption across the universe
 * - Two-tier system: active (real-time) + inactive (catch-up batched)
 * - Calculate shortages, surpluses, and economic stress
 * - Adjust prices dynamically based on supply/demand
 * - Generate trade opportunities through interdependency
 * 
 * TWO-TIER ARCHITECTURE:
 * - ACTIVE SYSTEMS: Where players are located - tick every cycle (real-time)
 * - INACTIVE SYSTEMS: All other systems - batched catch-up updates
 * 
 * This prevents 1000+ markets from lagging the server while maintaining
 * universe-wide economic simulation.
 * 
 * DESIGN PHILOSOPHY:
 * "The economy drives everything."
 * Trade, missions, piracy, conflict emerge from economic pressure.
 * No location is self-sufficient. Interdependency is intentional.
 */
UCLASS()
class FRACTUREDSTARS_API UEconomicSimulationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// SERVER: Simulation Control
	// ========================================================================

	/**
	 * Start the economic simulation timer (SERVER ONLY)
	 * Called after universe generation and economy initialization
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Simulation")
	void StartEconomicSimulation();

	/**
	 * Stop the economic simulation timer
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Simulation")
	void StopEconomicSimulation();

	/**
	 * Check if simulation is running
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Simulation")
	bool IsSimulationRunning() const { return bIsSimulationRunning; }

	/**
	 * Set tick interval (seconds between ticks)
	 * Default: 1.0 second = 1 game hour
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Simulation")
	void SetTickInterval(float NewInterval);

	/**
	 * Get current tick interval
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Simulation")
	float GetTickInterval() const { return TickInterval; }

	// ========================================================================
	// SERVER: System Activity Management (Two-Tier)
	// ========================================================================

	/**
	 * Mark a system as active (player entered)
	 * Active systems tick in real-time
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Simulation")
	void ActivateSystem(int32 SystemId);

	/**
	 * Mark a system as inactive (no players present)
	 * Inactive systems use batched catch-up
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Simulation")
	void DeactivateSystem(int32 SystemId);

	/**
	 * Check if a system is actively simulated
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Query")
	bool IsSystemActive(int32 SystemId) const;

	/**
	 * Get count of active systems
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Query")
	int32 GetActiveSystemCount() const;

	// ========================================================================
	// SERVER: Production & Consumption
	// ========================================================================

	/**
	 * Tick production for a location
	 * Produces goods based on recipes and input availability
	 */
	void TickProduction(int32 SystemId, int32 LocationId);

	/**
	 * Tick consumption for a location
	 * Consumes goods and calculates shortages
	 */
	void TickConsumption(int32 SystemId, int32 LocationId);

	/**
	 * Update production efficiency based on input availability
	 */
	void UpdateProductionEfficiency(int32 SystemId, int32 LocationId);

	/**
	 * Calculate economic stress for a location
	 * Based on critical shortages
	 */
	float CalculateEconomicStress(int32 SystemId, int32 LocationId);

	/**
	 * Update market prices based on shortage/surplus
	 */
	void UpdateMarketPrices(int32 SystemId, int32 LocationId);

	// ========================================================================
	// DEBUG & REPORTS
	// ========================================================================

	/**
	 * Generate economic simulation report
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Debug")
	FEconomicSimulationReport GenerateReport();

	/**
	 * Print economic summary to log
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Debug")
	void PrintEconomicSummary();

	/**
	 * Print location economic state
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Debug")
	void PrintLocationEconomy(int32 SystemId, int32 LocationId);

	/**
	 * Get all shortages across the universe
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Query")
	TArray<FEconomicShortage> GetAllShortages();

	/**
	 * Get all surpluses across the universe
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Query")
	TArray<FEconomicSurplus> GetAllSurpluses();

private:
	// ========================================================================
	// INTERNAL: Simulation Loop
	// ========================================================================

	/** Main simulation tick callback */
	void OnEconomicTick();

	/** Tick all active systems (real-time) */
	void TickActiveSystems();

	/** Batch catch-up for inactive systems */
	void CatchUpInactiveSystems();

	/** Tick a single system's economy */
	void TickSystemEconomy(int32 SystemId, int32 TickCount = 1);

	/** Tick a single location's economy */
	void TickLocationEconomy(int32 SystemId, int32 LocationId, int32 TickCount = 1);

	// ========================================================================
	// INTERNAL: Helper Functions
	// ========================================================================

	/** Get universe subsystem reference (lazy initialization) */
	UUniverseSubsystem* GetUniverseSubsystem() const;

	/** Check if location has enough inputs for production */
	bool HasSufficientInputs(int32 SystemId, int32 LocationId, const FProductionRecipe& Recipe, int32 Multiplier = 1);

	/** Consume inputs for production */
	void ConsumeProductionInputs(int32 SystemId, int32 LocationId, const FProductionRecipe& Recipe, int32 Multiplier = 1);

	/** Add goods to location inventory */
	void AddToInventory(int32 SystemId, int32 LocationId, EGoodType GoodType, int32 Quantity);

	/** Remove goods from location inventory (returns actual amount removed) */
	int32 RemoveFromInventory(int32 SystemId, int32 LocationId, EGoodType GoodType, int32 Quantity);

	/** Get current stock of a good */
	int32 GetStock(int32 SystemId, int32 LocationId, EGoodType GoodType);

	/** Ensure market has entry for a good */
	void EnsureMarketEntry(int32 SystemId, int32 LocationId, EGoodType GoodType);

	// ========================================================================
	// DATA
	// ========================================================================

	/** Reference to universe subsystem */
	UPROPERTY()
	TObjectPtr<UUniverseSubsystem> UniverseSubsystem;

	/** System economic states (for two-tier tracking) */
	UPROPERTY()
	TMap<int32, FSystemEconomicState> SystemStates;

	/** Is simulation currently running? */
	UPROPERTY()
	bool bIsSimulationRunning = false;

	/** Tick interval in seconds (default 1.0 = 1 game hour) */
	UPROPERTY()
	float TickInterval = 1.0f;

	/** Timer handle for economic tick */
	FTimerHandle EconomicTickTimerHandle;

	/** Batch size for inactive system catch-up */
	UPROPERTY()
	int32 InactiveBatchSize = 50;

	/** Maximum ticks to catch up per frame (prevents hitches) */
	UPROPERTY()
	int32 MaxCatchUpTicks = 10;
};
