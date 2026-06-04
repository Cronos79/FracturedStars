// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Universe/UniverseTypes.h"
#include "FogOfWarSubsystem.generated.h"

class UUniverseSubsystem;

/**
 * Fog of War Subsystem
 * 
 * SERVER AUTHORITY:
 * - Maintains per-player FPlayerFogOfWarState
 * - Updates visibility when player gains/loses observation capability
 * - Ages information over time
 * - Filters dynamic data before transmission to clients
 * 
 * CLIENT BEHAVIOR:
 * - Does NOT maintain fog-of-war state locally
 * - Receives only filtered queries/updates from server
 * - Displays information based on server-provided visibility
 * 
 * DESIGN PHILOSOPHY:
 * "The server always knows the real state. The player should never possess perfect information."
 * This subsystem enforces that contract.
 */
UCLASS()
class FRACTUREDSTARS_API UFogOfWarSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// SERVER: Player Registration & Management
	// ========================================================================

	/**
	 * Register a new player connection (SERVER ONLY)
	 * Creates fog-of-war state for this player
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Server")
	void RegisterPlayer(int32 PlayerId);

	/**
	 * Unregister a player connection (SERVER ONLY)
	 * Removes fog-of-war state for this player
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Server")
	void UnregisterPlayer(int32 PlayerId);

	// ========================================================================
	// SERVER: Visibility Updates
	// ========================================================================

	/**
	 * Update visibility when player focuses on a system (SERVER ONLY)
	 * Called by UniverseSubsystem when player enters/observes system
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Server")
	void OnPlayerFocusSystem(int32 PlayerId, FName SystemId);

	/**
	 * Update visibility when player unfocuses from a system (SERVER ONLY)
	 * Called by UniverseSubsystem when player leaves/stops observing system
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Server")
	void OnPlayerUnfocusSystem(int32 PlayerId, FName SystemId);

	/**
	 * Grant discovery of a system (moves Hidden -> Known) (SERVER ONLY)
	 * Called when player discovers system via nav data, rumors, or exploration
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Server")
	void DiscoverSystem(int32 PlayerId, FName SystemId);

	/**
	 * Complete survey of a system (moves Known -> Surveyed) (SERVER ONLY)
	 * Called when player completes detailed scan or visit
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Server")
	void CompleteSurvey(int32 PlayerId, FName SystemId);

	/**
	 * Add observation capability (crew/ship/station) (SERVER ONLY)
	 * Moves system to ActiveObservation and enables live updates
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Server")
	void AddObservationSource(int32 PlayerId, FName SystemId);

	/**
	 * Remove observation capability (crew/ship/station) (SERVER ONLY)
	 * If no other sources remain, downgrades from ActiveObservation
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Server")
	void RemoveObservationSource(int32 PlayerId, FName SystemId);

	// ========================================================================
	// SERVER: Information Aging & Updates
	// ========================================================================

	/**
	 * Update known state for a system (SERVER ONLY)
	 * Called when player receives intelligence/news/market update
	 * Ages existing information
	 */
	void UpdateKnownState(int32 PlayerId, FName SystemId, const FSystemKnownState& NewState);

	/**
	 * Tick all players' fog-of-war state (SERVER ONLY)
	 * Ages information, downgrades visibility when observation sources expire
	 * Called periodically by timer
	 */
	void TickFogOfWar(float DeltaTime);

	// ========================================================================
	// QUERIES: Visibility & Known State (SERVER filters, CLIENT receives)
	// ========================================================================

	/**
	 * Get player's visibility level for a system
	 * SERVER: Returns actual visibility from fog-of-war state
	 * CLIENT: Should not call directly; use filtered query results
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Query")
	ESystemVisibility GetSystemVisibility(int32 PlayerId, FName SystemId) const;

	/**
	 * Check if player has active observation in a system
	 * SERVER: Returns true if player has crew/ships/stations or active focus
	 * CLIENT: Should not call directly; use filtered query results
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Query")
	bool HasActiveObservation(int32 PlayerId, FName SystemId) const;

	/**
	 * Get player's known state for a system (SERVER ONLY)
	 * Returns snapshot of what player knows (prices, population, etc.)
	 * CLIENT: Never receives this directly; gets filtered results instead
	 */
	bool GetKnownState(int32 PlayerId, FName SystemId, FSystemKnownState& OutState) const;

	/**
	 * Get all systems visible to player at a given level or higher
	 * SERVER: Filters system list by visibility
	 * CLIENT: Should not call directly; receives filtered results via RPC
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Query")
	TArray<FName> GetSystemsWithVisibility(int32 PlayerId, ESystemVisibility MinLevel) const;

	// ========================================================================
	// UTILITY
	// ========================================================================

	/**
	 * Calculate information age in game time
	 * Returns how old the known state is compared to current time
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Utility")
	float GetInformationAge(const FUniverseTime& LastUpdate, const FUniverseTime& CurrentTime) const;

	// ========================================================================
	// VISIBILITY-FILTERED ECONOMY QUERIES
	// ========================================================================
	// These methods integrate fog-of-war with economy queries.
	// SERVER: Returns real data if active observation, known/stale data otherwise
	// CLIENT: Should receive results via RPC, never query directly

	/**
	 * Get market price for a good (visibility-filtered)
	 * Returns real price if ActiveObservation, last known price if Surveyed+, 0 if lower visibility
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Economy")
	float GetVisiblePrice(int32 PlayerId, FName SystemId, int32 LocationId, EGoodType GoodType) const;

	/**
	 * Check if player knows about a shortage (visibility-filtered)
	 * Returns current state if ActiveObservation, last known state if Surveyed+, false if lower visibility
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Economy")
	bool GetVisibleShortage(int32 PlayerId, FName SystemId, int32 LocationId, EGoodType GoodType) const;

	/**
	 * Get all market prices visible to player (visibility-filtered)
	 * Returns map of Good -> Price for all visible goods
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Economy")
	TMap<EGoodType, float> GetVisibleMarketPrices(int32 PlayerId, FName SystemId, int32 LocationId) const;

	// ========================================================================
	// FUTURE: Crew & Asset Visibility Extension
	// ========================================================================
	// These methods provide hooks for crew/ship/station visibility extension
	// Placeholder implementations - will be fully implemented when crew/ship systems exist

	/**
	 * Register a crew member stationed at a location (FUTURE)
	 * Extends visibility to that system even when player not focused
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Assets")
	void RegisterCrewMember(int32 PlayerId, FName SystemId, int32 CrewId);

	/**
	 * Unregister a crew member from a location (FUTURE)
	 * Removes their visibility contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Assets")
	void UnregisterCrewMember(int32 PlayerId, FName SystemId, int32 CrewId);

	/**
	 * Register a player-owned ship in a system (FUTURE)
	 * Extends visibility to that system
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Assets")
	void RegisterShip(int32 PlayerId, FName SystemId, int32 ShipId);

	/**
	 * Unregister a ship from a system (FUTURE)
	 * Removes its visibility contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Assets")
	void UnregisterShip(int32 PlayerId, FName SystemId, int32 ShipId);

	/**
	 * Register a player-owned station in a system (FUTURE)
	 * Extends visibility to that system permanently
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Assets")
	void RegisterStation(int32 PlayerId, FName SystemId, int32 StationId);

	/**
	 * Unregister a station from a system (FUTURE)
	 * Removes its visibility contribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Assets")
	void UnregisterStation(int32 PlayerId, FName SystemId, int32 StationId);

	// ========================================================================
	// DEBUG & TESTING
	// ========================================================================
	// These functions are for development/testing purposes
	// They help validate fog-of-war behavior in editor/PIE

	/**
	 * Print player's fog-of-war state to log (DEBUG)
	 * Shows all known systems and their visibility levels
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Debug")
	void DebugPrintPlayerFogState(int32 PlayerId) const;

	/**
	 * Force set visibility level for a system (DEBUG/TEST ONLY)
	 * Bypasses normal discovery/survey flow for testing
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Debug")
	void DebugSetVisibility(int32 PlayerId, FName SystemId, ESystemVisibility NewVisibility);

	/**
	 * Simulate information aging by backdating timestamps (DEBUG/TEST ONLY)
	 * Makes known state appear older for testing staleness
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Debug")
	void DebugSimulateInformationAge(int32 PlayerId, FName SystemId, float GameDaysAgo);

	/**
	 * Compare real vs known state for a system (DEBUG)
	 * Prints side-by-side comparison to log
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Debug")
	void DebugCompareRealVsKnown(int32 PlayerId, FName SystemId) const;

	/**
	 * Get count of systems at each visibility level (DEBUG)
	 * Returns string with counts: "Hidden: 450, Known: 30, Surveyed: 15, Active: 5"
	 */
	UFUNCTION(BlueprintCallable, Category = "Fog of War|Debug")
	FString DebugGetVisibilitySummary(int32 PlayerId) const;

private:
	// SERVER ONLY: Map of PlayerId -> FogOfWar state
	// Client never has this data
	UPROPERTY()
	TMap<int32, FPlayerFogOfWarState> PlayerFogOfWarStates;

	// Reference to universe subsystem for time queries and system lookups
	UPROPERTY()
	TObjectPtr<UUniverseSubsystem> UniverseSubsystem;

	// Timer handle for periodic fog-of-war tick
	FTimerHandle TickTimerHandle;

	// Tick interval (seconds of real time, not game time)
	float TickInterval = 1.0f;

	// FUTURE: Track crew/ship/station visibility sources per player
	// Maps PlayerId -> (SystemId -> Set of Asset IDs)
	// Not reflected (internal tracking only, not replicated)
	TMap<int32, TMap<FName, TSet<int32>>> PlayerCrewSources;
	TMap<int32, TMap<FName, TSet<int32>>> PlayerShipSources;
	TMap<int32, TMap<FName, TSet<int32>>> PlayerStationSources;

	// Helper: Get or create fog-of-war state for player
	FPlayerFogOfWarState* GetOrCreatePlayerState(int32 PlayerId);

	// Helper: Get read-only fog-of-war state for player
	const FPlayerFogOfWarState* GetPlayerState(int32 PlayerId) const;

	// Helper: Get or create known state for system
	FSystemKnownState* GetOrCreateKnownState(int32 PlayerId, FName SystemId);

	// Helper: Downgrade visibility when observation ends
	void DowngradeVisibility(int32 PlayerId, FName SystemId);

	// Helper: Take snapshot of current real state for known state
	void SnapshotRealState(int32 PlayerId, FName SystemId);

	// Helper: Ensure UniverseSubsystem reference is valid (lazy initialization)
	UUniverseSubsystem* EnsureUniverseSubsystem() const;
};
