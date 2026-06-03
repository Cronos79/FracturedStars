// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

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

	/**
	 * Print current game time info
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Debug")
	void PrintTimeInfo() const;

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

	// Player Presence Tracking (Phase 2: Multiplayer)
	// NOTE: For RTS/Stellaris-style gameplay, "entering/leaving" refers to camera focus/observation,
	// not physical player location. Player is a strategic controller viewing systems via UI.

	/**
	 * Register a player focusing on a system (camera/UI observation)
	 * Server-only: Wakes up system if first player, triggers catch-up and live simulation
	 * 
	 * RTS Model: This is called when player's camera/UI focuses on a system (galaxy map click,
	 * system detail panel open, etc). NOT when a player character physically enters.
	 * 
	 * @param Player - Player controller focusing on the system
	 * @param SystemId - System being observed
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
	void OnPlayerEnterSystem(APlayerController* Player, int32 SystemId);

	/**
	 * Register a player unfocusing from a system (camera/UI switches away)
	 * Server-only: Puts system to sleep if last player, stops live simulation
	 * 
	 * RTS Model: Called when player's camera/UI switches to a different system or closes
	 * the system view. System returns to background/catch-up mode if no other players watching.
	 * 
	 * @param Player - Player controller unfocusing from the system
	 * @param SystemId - System no longer being observed
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
	void OnPlayerLeaveSystem(APlayerController* Player, int32 SystemId);

	/**
	 * Check if any players are currently observing a system
	 * @param SystemId - System to check
	 * @return True if at least one player is watching/focusing on the system
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Multiplayer")
	bool HasPlayersInSystem(int32 SystemId) const;

	/**
	 * Get count of players observing a system
	 * @param SystemId - System to check
	 * @return Number of players currently watching the system
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Multiplayer")
	int32 GetPlayerCountInSystem(int32 SystemId) const;

	// Game Time Functions

	/**
	 * Get current game time and date
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Time")
	FUniverseTime GetGameTime() const;

	/**
	 * Get formatted date string (e.g., "July 8, 2094")
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Time")
	FString GetFormattedDate() const;

	/**
	 * Get formatted time string (e.g., "14:35:22")
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Time")
	FString GetFormattedTime() const;

	/**
	 * Get formatted date and time (e.g., "July 8, 2094 14:35:22")
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Time")
	FString GetFormattedDateTime() const;

	/**
	 * Get elapsed days since universe start
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Time")
	int32 GetElapsedDays() const;

	/**
	 * Set time scale multiplier (how fast time passes)
	 * Default: 24.0 (1 real second = 24 game seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Time")
	void SetTimeScale(float NewTimeScale);

	/**
	 * Get current time scale
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Time")
	float GetTimeScale() const;

	/**
	 * Enable/disable time advancement
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe|Time")
	void SetTimePaused(bool bPaused);

	/**
	 * Check if time is paused
	 */
	UFUNCTION(BlueprintPure, Category = "Universe|Time")
	bool IsTimePaused() const;

private:
	UPROPERTY()
	FUniverseData UniverseData;

	UPROPERTY()
	bool bIsGenerated = false;

	UPROPERTY()
	bool bTimePaused = false;

	// Player presence tracking: SystemId -> Array of players in that system
	// Server-only: Used to determine which systems need active simulation
	TMap<int32, TArray<APlayerController*>> PlayersInSystem;

	// Timer handle for time advancement
	FTimerHandle TimeAdvancementTimer;

	// Network authority helpers
	bool IsAuthority() const;
	bool IsClient() const;

	// BFS pathfinding helper
	TArray<int32> FindPathInternal(int32 StartId, int32 EndId) const;

	// Time advancement helpers
	void AdvanceTime(float DeltaSeconds);
	void UpdateGameTime(double DeltaGameSeconds);
	bool IsLeapYear(int32 Year) const;
	int32 GetDaysInMonth(int32 Month, int32 Year) const;
	FString GetMonthName(int32 Month) const;
};
