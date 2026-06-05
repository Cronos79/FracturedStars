// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Ship/ShipData.h"
#include "PlayerTypes.generated.h"

/**
 * Player, Crew Data Structures
 * 
 * DESIGN PHILOSOPHY:
 * - Unreal PlayerController/Pawn are separate from in-universe player data
 * - Player is persistent simulation data, not a possessed character
 * - Ships are independent (see Ship/ShipData.h)
 * - Crew are independent (can be assigned to ships/locations)
 * - Data structures support future save/load
 * 
 * Sprint 4 Foundation - provides minimal playable identity layer
 * Sprint 5.5 - Ships moved to Ship/ShipData.h for module independence
 */

// ============================================================================
// ENUMS
// ============================================================================

/**
 * Crew member status
 */
UENUM(BlueprintType)
enum class ECrewStatus : uint8
{
	Available    UMETA(DisplayName = "Available"),     // Crew ready for assignment
	Assigned     UMETA(DisplayName = "Assigned"),      // Crew assigned to ship/location
	Injured      UMETA(DisplayName = "Injured"),       // Crew is injured (future)
	Dead         UMETA(DisplayName = "Dead")           // Crew is dead (future)
};

/**
 * Player connection/activity status
 */
UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
	Active       UMETA(DisplayName = "Active"),        // Player is online and playing
	Idle         UMETA(DisplayName = "Idle"),          // Player is online but inactive
	Offline      UMETA(DisplayName = "Offline")        // Player is logged off
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Persistent player profile
 * Represents the in-universe player character (not Unreal PlayerController)
 */
USTRUCT(BlueprintType)
struct FPlayerProfileData
{
	GENERATED_BODY()

	/** Unique player identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 PlayerId = -1;

	/** Player display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FString PlayerName;

	/** Current credits (money) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Economy")
	int32 Credits = 5000;

	/** Current system the player is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Location")
	int32 CurrentSystemId = -1;

	/** Current location within system (station/planet/etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Location")
	int32 CurrentLocationId = -1;

	/** Ship the player is currently aboard (-1 if not aboard a ship) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Ships")
	int32 CurrentShipId = -1;

	/** List of all ships owned by this player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Ships")
	TArray<int32> OwnedShipIds;

	/** List of all crew employed by this player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Crew")
	TArray<int32> CrewIds;

	/** Player connection status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	EPlayerStatus Status = EPlayerStatus::Active;

	/** Timestamp of player creation (Unix timestamp) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int64 CreatedTime = 0;

	/** Timestamp of last activity (Unix timestamp) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int64 LastActiveTime = 0;

	// Future expansion placeholders (Sprint 4 does not implement these)
	// Skills, traits, reputation, backstory, faction affinity, etc.
};

/**
 * Persistent crew member data
 * Represents a person employed by the player
 */
USTRUCT(BlueprintType)
struct FCrewMemberData
{
	GENERATED_BODY()

	/** Unique crew identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crew")
	int32 CrewId = -1;

	/** Crew member name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crew")
	FString Name;

	/** Player who employs this crew member */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crew|Ownership")
	int32 OwnerPlayerId = -1;

	/** Ship this crew is assigned to (-1 if unassigned) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crew|Assignment")
	int32 AssignedShipId = -1;

	/** Current system the crew is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crew|Location")
	int32 CurrentSystemId = -1;

	/** Current location within system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crew|Location")
	int32 CurrentLocationId = -1;

	/** Crew operational status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crew")
	ECrewStatus Status = ECrewStatus::Available;

	// Future expansion placeholders (Sprint 4 does not implement these)
	// Skills (piloting, engineering, trade, combat, etc.)
	// Traits (hardworking, lazy, friendly, loner, etc.)
	// Morale, loyalty, experience, salary, backstory, etc.
};
