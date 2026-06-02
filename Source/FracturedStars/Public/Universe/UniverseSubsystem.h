// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Universe/UniverseTypes.h"
#include "UniverseSubsystem.generated.h"

/**
 * Universe Subsystem
 * Manages the persistent universe state
 * Server-authoritative, persists across level transitions
 */
UCLASS()
class FRACTUREDSTARS_API UUniverseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Generate a new universe
	 * @param Config - Generation parameters
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe")
	bool GenerateUniverse(const FUniverseConfig& Config);

	/**
	 * Check if universe has been generated
	 */
	UFUNCTION(BlueprintPure, Category = "Universe")
	bool IsUniverseGenerated() const { return bIsGenerated; }

	/**
	 * Get universe data (read-only)
	 */
	UFUNCTION(BlueprintPure, Category = "Universe")
	const FUniverseData& GetUniverseData() const { return UniverseData; }

	/**
	 * Get system by ID
	 */
	UFUNCTION(BlueprintPure, Category = "Universe")
	bool GetSystemById(int32 SystemId, FStarSystemData& OutSystem) const;

	/**
	 * Get all systems in a region type
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe")
	TArray<FStarSystemData> GetSystemsByRegion(ERegionType RegionType) const;

	/**
	 * Find path between two systems
	 * @return Array of system IDs forming the path (empty if no path)
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe")
	TArray<int32> FindPath(int32 StartSystemId, int32 EndSystemId) const;

	/**
	 * Get jump distance between systems
	 * @return Number of jumps, or -1 if unreachable
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe")
	int32 GetJumpDistance(int32 SystemA, int32 SystemB) const;

	/**
	 * Print universe statistics to log
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Debug")
	void PrintUniverseInfo() const;

	/**
	 * Print specific system info
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Debug")
	void PrintSystemInfo(int32 SystemId) const;

	// Sprint 2: Content query functions

	/**
	 * Get celestial bodies in a system
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Content")
	TArray<FCelestialBodyData> GetCelestialBodiesInSystem(int32 SystemId) const;

	/**
	 * Get locations in a system
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Content")
	TArray<FLocationData> GetLocationsInSystem(int32 SystemId) const;

	/**
	 * Get all locations owned by a faction
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Content")
	TArray<FLocationData> GetLocationsByOwner(int32 FactionId) const;

	/**
	 * Print detailed system content (celestial bodies, locations, population)
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Debug")
	void PrintSystemContent(int32 SystemId) const;

	// Sprint 3: Economy integration functions

	/**
	 * Initialize economy for all locations
	 * Call after universe generation
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Economy")
	void InitializeEconomy();

	/**
	 * Set which system is actively simulated in real-time
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Economy")
	void SetActiveEconomySystem(int32 SystemId);

	/**
	 * Get current active system ID
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Economy")
	int32 GetActiveEconomySystemId() const;

	/**
	 * Get current game time (seconds since universe creation)
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Economy")
	double GetCurrentGameTime() const;

	/**
	 * Get market state at a location
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Economy")
	FMarketState GetMarketState(int32 SystemId, int32 LocationId) const;

	/**
	 * Get price of a good at a location
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Economy")
	float GetGoodPrice(int32 SystemId, int32 LocationId, EGoodType GoodType) const;

	/**
	 * Check if location has shortage of a good
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Economy")
	bool HasShortage(int32 SystemId, int32 LocationId, EGoodType GoodType) const;

	/**
	 * Find all locations with shortages of a specific good
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Economy")
	TArray<int32> FindShortageLocations(EGoodType GoodType) const;

	/**
	 * Print economy statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Debug")
	void PrintEconomyStats() const;

private:
	UPROPERTY()
	FUniverseData UniverseData;

	UPROPERTY()
	bool bIsGenerated = false;

	// BFS pathfinding helper
	TArray<int32> FindPathInternal(int32 StartId, int32 EndId) const;
};
