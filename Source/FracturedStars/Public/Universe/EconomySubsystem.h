// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UniverseTypes.h"
#include "EconomyTypes.h"
#include "EconomySubsystem.generated.h"

/**
 * Economy Subsystem
 * Manages economy simulation with two-tier update model:
 * - Active System: Real-time ticks (5-20s intervals)
 * - Background Systems: Time-compressed catch-up simulation
 * 
 * Sprint 3: Performance requirement - only player's current system simulated in real-time
 */
UCLASS()
class FRACTUREDSTARS_API UEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Initialize economy for all locations in the universe
	 * Called after universe generation to set up markets
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void InitializeEconomy(FUniverseData& UniverseData);

	/**
	 * Set active system for real-time economy simulation
	 * All other systems will use time-compressed catch-up when accessed
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void SetActiveSystem(FUniverseData& UniverseData, int32 SystemId);

	/**
	 * Tick active system economy (called by timer)
	 * Simulates production, consumption, and price changes
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void TickActiveSystemEconomy(FUniverseData& UniverseData, float DeltaTime);

	/**
	 * Catch up background system to current game time
	 * Time-compressed simulation for systems not visited recently
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void CatchUpSystemEconomy(FUniverseData& UniverseData, int32 SystemId);

	/**
	 * Query market state at a location
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	const FMarketState& GetMarketState(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId) const;

	/**
	 * Get current price for a good at a location
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	float GetGoodPrice(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId, EGoodType GoodType) const;

	/**
	 * Check if a location has a shortage of a good
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool HasShortage(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId, EGoodType GoodType) const;

	/**
	 * Find locations with shortages of a specific good (for trade route planning)
	 * Returns globally unique location references with SystemId + LocationId
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	TArray<FShortageLocation> FindShortageLocations(const FUniverseData& UniverseData, EGoodType GoodType) const;

	/**
	 * Print economy statistics for debugging
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Debug")
	void PrintEconomyStats(const FUniverseData& UniverseData) const;

protected:
	/**
	 * Initialize market for a single location
	 * Sets up goods based on production/consumption profiles
	 */
	void InitializeLocationMarket(FLocationData& Location, FRandomStream& RNG);

	/**
	 * Create good definition catalog (static data)
	 */
	TMap<EGoodType, FGoodDefinition> CreateGoodsCatalog() const;

	/**
	 * Simulate one economy cycle for a location
	 * Updates stock, prices, and shortage/surplus flags
	 */
	void SimulateLocationEconomy(int32 SystemId, FLocationData& Location, float DeltaHours, const TMap<EGoodType, FGoodDefinition>& GoodsCatalog);

	/**
	 * Update market prices based on supply/demand
	 */
	void UpdateMarketPrices(FMarketState& Market, const TMap<EGoodType, FGoodDefinition>& GoodsCatalog);

	/**
	 * Detect shortages and surpluses
	 */
	void UpdateShortagesAndSurpluses(FMarketState& Market);

	// Sprint 5: Production & Consumption Methods

	/**
	 * Process production at a location using recipes from economic profile
	 */
	void TickProduction(int32 SystemId, FLocationData& Location, float DeltaHours);

	/**
	 * Process consumption at a location using consumption profile
	 */
	void TickConsumption(int32 SystemId, FLocationData& Location, float DeltaHours);

	/**
	 * Update production efficiency based on input availability
	 */
	void UpdateProductionEfficiency(int32 SystemId, FLocationData& Location);

	/**
	 * Calculate economic stress from critical good shortages
	 */
	float CalculateEconomicStress(int32 SystemId, const FLocationData& Location);

	// Sprint 5: Inventory Helper Methods

	/**
	 * Add goods to location inventory (uses UniverseSubsystem persistence)
	 */
	void AddToInventory(int32 SystemId, int32 LocationId, EGoodType GoodType, int32 Quantity);

	/**
	 * Remove goods from location inventory (uses UniverseSubsystem persistence)
	 * Returns actual amount removed
	 */
	int32 RemoveFromInventory(int32 SystemId, int32 LocationId, EGoodType GoodType, int32 Quantity);

	/**
	 * Get current stock of a good at a location
	 */
	int32 GetStock(int32 SystemId, int32 LocationId, EGoodType GoodType);

	/**
	 * Ensure a market entry exists for a good (creates with defaults if missing)
	 */
	void EnsureMarketEntry(int32 SystemId, int32 LocationId, EGoodType GoodType);

private:
	// Timer handle for active system ticks
	FTimerHandle ActiveSystemTickTimer;

	// Goods catalog (static definitions)
	TMap<EGoodType, FGoodDefinition> GoodsCatalog;
};
