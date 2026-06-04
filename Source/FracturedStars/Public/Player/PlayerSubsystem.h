// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Player/PlayerTypes.h"
#include "PlayerSubsystem.generated.h"

class UUniverseSubsystem;
class UFogOfWarSubsystem;

/**
 * Player Subsystem
 * 
 * SERVER AUTHORITY:
 * - Maintains persistent player, ship, and crew data
 * - Validates ownership and commands
 * - Tracks location and movement
 * - Integrates with fog-of-war for visibility updates
 * 
 * CLIENT BEHAVIOR:
 * - Clients request actions via BlueprintCallable functions
 * - Server validates and executes
 * - Clients receive filtered data based on visibility
 * 
 * DESIGN PHILOSOPHY:
 * "The player is not the Unreal pawn. The player is persistent simulation data."
 * 
 * This subsystem manages the in-universe identity layer:
 * - Player profiles (credits, location, owned assets)
 * - Ships (location, cargo, fuel, crew assignments)
 * - Crew (skills, assignments, location)
 * 
 * Sprint 4 Foundation - minimal playable identity
 */
UCLASS()
class FRACTUREDSTARS_API UPlayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// SERVER: Player Management
	// ========================================================================

	/**
	 * Create a new player profile (SERVER ONLY)
	 * @param PlayerName Display name for the player
	 * @param StartingSystemId System to spawn player in (-1 = auto-select valid system)
	 * @return PlayerId of created player, or -1 on failure
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Management")
	int32 CreatePlayer(const FString& PlayerName, int32 StartingSystemId = -1);

	/**
	 * Get player profile data
	 * @param PlayerId Player to query
	 * @return Player profile data (PlayerId = -1 if not found)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Query")
	FPlayerProfileData GetPlayerProfile(int32 PlayerId) const;

	/**
	 * Check if player exists
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Query")
	bool PlayerExists(int32 PlayerId) const;

	/**
	 * Get all active player IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Query")
	TArray<int32> GetAllPlayerIds() const;

	// ========================================================================
	// SERVER: Ship Management
	// ========================================================================

	/**
	 * Create a starter ship for a player (SERVER ONLY)
	 * @param PlayerId Owner of the ship
	 * @param SystemId System to place ship in
	 * @param LocationId Location within system
	 * @return ShipId of created ship, or -1 on failure
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Ships")
	int32 CreateStarterShip(int32 PlayerId, int32 SystemId, int32 LocationId);

	/**
	 * Get ship data
	 * @param ShipId Ship to query
	 * @return Ship data (ShipId = -1 if not found)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Ships")
	FShipData GetShip(int32 ShipId) const;

	/**
	 * Get all ships owned by a player
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Ships")
	TArray<FShipData> GetPlayerShips(int32 PlayerId) const;

	/**
	 * Check if player owns ship
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Ships")
	bool PlayerOwnsShip(int32 PlayerId, int32 ShipId) const;

	// ========================================================================
	// SERVER: Crew Management
	// ========================================================================

	/**
	 * Create a crew member for a player (SERVER ONLY)
	 * @param PlayerId Owner of the crew
	 * @param ShipId Ship to assign crew to (-1 = unassigned)
	 * @return CrewId of created crew, or -1 on failure
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Crew")
	int32 CreateCrewMember(int32 PlayerId, int32 ShipId = -1);

	/**
	 * Get crew member data
	 * @param CrewId Crew to query
	 * @return Crew data (CrewId = -1 if not found)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Crew")
	FCrewMemberData GetCrewMember(int32 CrewId) const;

	/**
	 * Get all crew employed by a player
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Crew")
	TArray<FCrewMemberData> GetPlayerCrew(int32 PlayerId) const;

	/**
	 * Assign crew to a ship (SERVER ONLY)
	 * @param CrewId Crew to assign
	 * @param ShipId Ship to assign to (-1 = unassign)
	 * @return true if assignment succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Crew")
	bool AssignCrewToShip(int32 CrewId, int32 ShipId);

	// ========================================================================
	// SERVER: Movement & Location
	// ========================================================================

	/**
	 * Move ship to a connected system (SERVER ONLY)
	 * @param ShipId Ship to move
	 * @param TargetSystemId Destination system
	 * @return true if movement succeeded
	 * 
	 * Validates:
	 * - Ship exists
	 * - Target system is connected to current system
	 * - Updates player/crew location if aboard
	 * - Updates fog-of-war visibility
	 * - Logs fuel consumption (but does not enforce empty = stuck in Sprint 4)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	bool MoveShipToConnectedSystem(int32 ShipId, int32 TargetSystemId);

	/**
	 * Board a ship (player enters ship) (SERVER ONLY)
	 * @param PlayerId Player boarding
	 * @param ShipId Ship to board
	 * @return true if boarding succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	bool BoardShip(int32 PlayerId, int32 ShipId);

	/**
	 * Disembark from ship (player leaves ship) (SERVER ONLY)
	 * @param PlayerId Player disembarking
	 * @return true if disembarking succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	bool DisembarkShip(int32 PlayerId);

	// ========================================================================
	// DEBUG & TESTING
	// ========================================================================

	/**
	 * Print detailed player info to log (DEBUG)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Debug")
	void PrintPlayerInfo(int32 PlayerId) const;

	/**
	 * Print detailed ship info to log (DEBUG)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Debug")
	void PrintShipInfo(int32 ShipId) const;

	/**
	 * Print detailed crew info to log (DEBUG)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Debug")
	void PrintCrewInfo(int32 CrewId) const;

	/**
	 * Print all player state (player + ships + crew) (DEBUG)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Debug")
	void PrintAllPlayerState(int32 PlayerId) const;

	/**
	 * Create a fully equipped test player (DEBUG)
	 * Creates player + starter ship + crew at valid starting location
	 * @return PlayerId of created player
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Debug")
	int32 CreateTestPlayer(const FString& PlayerName = TEXT("Test Captain"));

private:
	// SERVER ONLY: Player data
	// Key: PlayerId
	UPROPERTY()
	TMap<int32, FPlayerProfileData> Players;

	// SERVER ONLY: Ship data
	// Key: ShipId
	UPROPERTY()
	TMap<int32, FShipData> Ships;

	// SERVER ONLY: Crew data
	// Key: CrewId
	UPROPERTY()
	TMap<int32, FCrewMemberData> Crew;

	// Reference to universe subsystem for location validation
	UPROPERTY()
	TObjectPtr<UUniverseSubsystem> UniverseSubsystem;

	// Reference to fog-of-war subsystem for visibility updates
	UPROPERTY()
	TObjectPtr<UFogOfWarSubsystem> FogOfWarSubsystem;

	// Next available IDs
	int32 NextPlayerId = 1;
	int32 NextShipId = 1001;
	int32 NextCrewId = 10001;

	// Crew name pool for random name generation
	TArray<FString> CrewFirstNames;
	TArray<FString> CrewLastNames;

	// Helper: Initialize crew name pool
	void InitializeCrewNamePool();

	// Helper: Generate random crew name
	FString GenerateRandomCrewName() const;

	// Helper: Validate system/location exists in universe
	bool IsValidLocation(int32 SystemId, int32 LocationId) const;

	// Helper: Check if two systems are connected
	bool AreSystemsConnected(int32 SystemA, int32 SystemB) const;

	// Helper: Update fog-of-war when player/ship moves
	void UpdateFogOfWarFromMovement(int32 PlayerId, int32 OldSystemId, int32 NewSystemId);

	// Helper: Ensure subsystem references are valid (lazy initialization)
	UUniverseSubsystem* EnsureUniverseSubsystem() const;
	UFogOfWarSubsystem* EnsureFogOfWarSubsystem() const;
};
