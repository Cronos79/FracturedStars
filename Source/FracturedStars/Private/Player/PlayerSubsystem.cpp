// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Player/PlayerSubsystem.h"
#include "Universe/UniverseSubsystem.h"
#include "Universe/FogOfWarSubsystem.h"

void UPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize crew name pool
	InitializeCrewNamePool();

	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Initialized - ready for player/ship/crew management"));
}

void UPlayerSubsystem::Deinitialize()
{
	// Clear all player data
	Players.Empty();
	Ships.Empty();
	Crew.Empty();

	Super::Deinitialize();
}

// ============================================================================
// PLAYER MANAGEMENT
// ============================================================================

int32 UPlayerSubsystem::CreatePlayer(const FString& PlayerName, int32 StartingSystemId)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] CreatePlayer called on CLIENT - ignoring"));
		return -1;
	}

	// Auto-select starting system if not specified
	int32 SpawnSystemId = StartingSystemId;
	int32 SpawnLocationId = 0;

	if (SpawnSystemId == -1)
	{
		// Find first system with at least one location
		const FUniverseData& UniverseData = Universe->GetUniverseData();
		for (const FStarSystemData& System : UniverseData.Systems)
		{
			if (System.Locations.Num() > 0)
			{
				SpawnSystemId = System.SystemId;
				SpawnLocationId = 0; // First location
				break;
			}
		}

		if (SpawnSystemId == -1)
		{
			UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Failed to find valid starting system"));
			return -1;
		}
	}

	// Validate starting location exists
	if (!IsValidLocation(SpawnSystemId, SpawnLocationId))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Invalid starting location: System %d, Location %d"), 
			SpawnSystemId, SpawnLocationId);
		return -1;
	}

	// Create player profile
	FPlayerProfileData NewPlayer;
	NewPlayer.PlayerId = NextPlayerId++;
	NewPlayer.PlayerName = PlayerName;
	NewPlayer.Credits = 5000;
	NewPlayer.CurrentSystemId = SpawnSystemId;
	NewPlayer.CurrentLocationId = SpawnLocationId;
	NewPlayer.CurrentShipId = -1; // Not aboard a ship yet
	NewPlayer.Status = EPlayerStatus::Active;
	NewPlayer.CreatedTime = FDateTime::UtcNow().ToUnixTimestamp();
	NewPlayer.LastActiveTime = NewPlayer.CreatedTime;

	Players.Add(NewPlayer.PlayerId, NewPlayer);

	// Register player with fog-of-war
	UFogOfWarSubsystem* FogOfWar = EnsureFogOfWarSubsystem();
	if (FogOfWar)
	{
		FogOfWar->RegisterPlayer(NewPlayer.PlayerId);
		// Discover starting system
		FogOfWar->DiscoverSystem(NewPlayer.PlayerId, FName(*FString::FromInt(SpawnSystemId)));
		FogOfWar->CompleteSurvey(NewPlayer.PlayerId, FName(*FString::FromInt(SpawnSystemId)));
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Created player %d: '%s' at System %d, Location %d"), 
		NewPlayer.PlayerId, *PlayerName, SpawnSystemId, SpawnLocationId);

	return NewPlayer.PlayerId;
}

FPlayerProfileData UPlayerSubsystem::GetPlayerProfile(int32 PlayerId) const
{
	const FPlayerProfileData* Player = Players.Find(PlayerId);
	if (Player)
	{
		return *Player;
	}

	// Return invalid player
	FPlayerProfileData Invalid;
	Invalid.PlayerId = -1;
	return Invalid;
}

bool UPlayerSubsystem::PlayerExists(int32 PlayerId) const
{
	return Players.Contains(PlayerId);
}

TArray<int32> UPlayerSubsystem::GetAllPlayerIds() const
{
	TArray<int32> PlayerIds;
	Players.GetKeys(PlayerIds);
	return PlayerIds;
}

// ============================================================================
// SHIP MANAGEMENT
// ============================================================================

int32 UPlayerSubsystem::CreateStarterShip(int32 PlayerId, int32 SystemId, int32 LocationId)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] CreateStarterShip called on CLIENT - ignoring"));
		return -1;
	}

	// Validate player exists
	FPlayerProfileData* Player = Players.Find(PlayerId);
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot create ship - player %d not found"), PlayerId);
		return -1;
	}

	// Validate location
	if (!IsValidLocation(SystemId, LocationId))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot create ship - invalid location: System %d, Location %d"), 
			SystemId, LocationId);
		return -1;
	}

	// Create ship
	FShipData NewShip;
	NewShip.ShipId = NextShipId++;
	NewShip.ShipName = TEXT("Starter Ship"); // Generic name for Sprint 4
	NewShip.OwnerPlayerId = PlayerId;
	NewShip.CurrentSystemId = SystemId;
	NewShip.CurrentLocationId = LocationId;
	NewShip.Status = EShipStatus::Docked;
	NewShip.CargoCapacity = 100;
	NewShip.CurrentCargoUsed = 0;
	NewShip.FuelCapacity = 100;
	NewShip.CurrentFuel = 100; // Start with full fuel
	NewShip.CrewCapacity = 3;

	Ships.Add(NewShip.ShipId, NewShip);

	// Add to player's owned ships
	Player->OwnedShipIds.Add(NewShip.ShipId);

	// If player not aboard a ship yet, board this one
	if (Player->CurrentShipId == -1)
	{
		Player->CurrentShipId = NewShip.ShipId;
		Player->CurrentSystemId = SystemId;
		Player->CurrentLocationId = LocationId;
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Created starter ship %d for player %d at System %d, Location %d"), 
		NewShip.ShipId, PlayerId, SystemId, LocationId);

	return NewShip.ShipId;
}

FShipData UPlayerSubsystem::GetShip(int32 ShipId) const
{
	const FShipData* Ship = Ships.Find(ShipId);
	if (Ship)
	{
		return *Ship;
	}

	// Return invalid ship
	FShipData Invalid;
	Invalid.ShipId = -1;
	return Invalid;
}

TArray<FShipData> UPlayerSubsystem::GetPlayerShips(int32 PlayerId) const
{
	TArray<FShipData> PlayerShips;

	const FPlayerProfileData* Player = Players.Find(PlayerId);
	if (!Player)
	{
		return PlayerShips;
	}

	for (int32 ShipId : Player->OwnedShipIds)
	{
		const FShipData* Ship = Ships.Find(ShipId);
		if (Ship)
		{
			PlayerShips.Add(*Ship);
		}
	}

	return PlayerShips;
}

bool UPlayerSubsystem::PlayerOwnsShip(int32 PlayerId, int32 ShipId) const
{
	const FShipData* Ship = Ships.Find(ShipId);
	return Ship && Ship->OwnerPlayerId == PlayerId;
}

// ============================================================================
// CREW MANAGEMENT
// ============================================================================

int32 UPlayerSubsystem::CreateCrewMember(int32 PlayerId, int32 ShipId)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] CreateCrewMember called on CLIENT - ignoring"));
		return -1;
	}

	// Validate player exists
	FPlayerProfileData* Player = Players.Find(PlayerId);
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot create crew - player %d not found"), PlayerId);
		return -1;
	}

	// If assigning to ship, validate ship exists and is owned by player
	if (ShipId != -1)
	{
		const FShipData* Ship = Ships.Find(ShipId);
		if (!Ship || Ship->OwnerPlayerId != PlayerId)
		{
			UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot create crew - ship %d invalid or not owned by player %d"), 
				ShipId, PlayerId);
			return -1;
		}
	}

	// Create crew member
	FCrewMemberData NewCrew;
	NewCrew.CrewId = NextCrewId++;
	NewCrew.Name = GenerateRandomCrewName();
	NewCrew.OwnerPlayerId = PlayerId;
	NewCrew.AssignedShipId = ShipId;
	NewCrew.Status = (ShipId == -1) ? ECrewStatus::Available : ECrewStatus::Assigned;

	// Set location based on assignment
	if (ShipId != -1)
	{
		const FShipData* Ship = Ships.Find(ShipId);
		NewCrew.CurrentSystemId = Ship->CurrentSystemId;
		NewCrew.CurrentLocationId = Ship->CurrentLocationId;

		// Add to ship's crew list
		FShipData* MutableShip = Ships.Find(ShipId);
		MutableShip->AssignedCrewIds.Add(NewCrew.CrewId);
	}
	else
	{
		// Unassigned crew at player's current location
		NewCrew.CurrentSystemId = Player->CurrentSystemId;
		NewCrew.CurrentLocationId = Player->CurrentLocationId;
	}

	Crew.Add(NewCrew.CrewId, NewCrew);

	// Add to player's crew list
	Player->CrewIds.Add(NewCrew.CrewId);

	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Created crew %d: '%s' for player %d (assigned to ship %d)"), 
		NewCrew.CrewId, *NewCrew.Name, PlayerId, ShipId);

	return NewCrew.CrewId;
}

FCrewMemberData UPlayerSubsystem::GetCrewMember(int32 CrewId) const
{
	const FCrewMemberData* CrewMember = Crew.Find(CrewId);
	if (CrewMember)
	{
		return *CrewMember;
	}

	// Return invalid crew
	FCrewMemberData Invalid;
	Invalid.CrewId = -1;
	return Invalid;
}

TArray<FCrewMemberData> UPlayerSubsystem::GetPlayerCrew(int32 PlayerId) const
{
	TArray<FCrewMemberData> PlayerCrew;

	const FPlayerProfileData* Player = Players.Find(PlayerId);
	if (!Player)
	{
		return PlayerCrew;
	}

	for (int32 CrewId : Player->CrewIds)
	{
		const FCrewMemberData* CrewMember = Crew.Find(CrewId);
		if (CrewMember)
		{
			PlayerCrew.Add(*CrewMember);
		}
	}

	return PlayerCrew;
}

bool UPlayerSubsystem::AssignCrewToShip(int32 CrewId, int32 ShipId)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] AssignCrewToShip called on CLIENT - ignoring"));
		return false;
	}

	// Get crew member
	FCrewMemberData* CrewMember = Crew.Find(CrewId);
	if (!CrewMember)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot assign crew - crew %d not found"), CrewId);
		return false;
	}

	// Remove from old ship if assigned
	if (CrewMember->AssignedShipId != -1)
	{
		FShipData* OldShip = Ships.Find(CrewMember->AssignedShipId);
		if (OldShip)
		{
			OldShip->AssignedCrewIds.Remove(CrewId);
		}
	}

	// Assign to new ship (or unassign if ShipId == -1)
	if (ShipId == -1)
	{
		// Unassign
		CrewMember->AssignedShipId = -1;
		CrewMember->Status = ECrewStatus::Available;
		UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Crew %d unassigned from ship"), CrewId);
	}
	else
	{
		// Validate ship exists and is owned by same player
		FShipData* Ship = Ships.Find(ShipId);
		if (!Ship || Ship->OwnerPlayerId != CrewMember->OwnerPlayerId)
		{
			UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot assign crew %d - ship %d invalid or different owner"), 
				CrewId, ShipId);
			return false;
		}

		// Assign
		CrewMember->AssignedShipId = ShipId;
		CrewMember->Status = ECrewStatus::Assigned;
		CrewMember->CurrentSystemId = Ship->CurrentSystemId;
		CrewMember->CurrentLocationId = Ship->CurrentLocationId;

		Ship->AssignedCrewIds.Add(CrewId);

		UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Crew %d assigned to ship %d"), CrewId, ShipId);
	}

	return true;
}

// ============================================================================
// MOVEMENT & LOCATION
// ============================================================================

bool UPlayerSubsystem::MoveShipToConnectedSystem(int32 ShipId, int32 TargetSystemId)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] MoveShipToConnectedSystem called on CLIENT - ignoring"));
		return false;
	}

	// Get ship
	FShipData* Ship = Ships.Find(ShipId);
	if (!Ship)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot move ship - ship %d not found"), ShipId);
		return false;
	}

	// Validate systems are connected
	if (!AreSystemsConnected(Ship->CurrentSystemId, TargetSystemId))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot move ship %d - systems %d and %d are not connected"), 
			ShipId, Ship->CurrentSystemId, TargetSystemId);
		return false;
	}

	// Validate target system exists
	if (!IsValidLocation(TargetSystemId, 0))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot move ship %d - target system %d does not exist"), 
			ShipId, TargetSystemId);
		return false;
	}

	int32 OldSystemId = Ship->CurrentSystemId;

	// Update ship location
	Ship->CurrentSystemId = TargetSystemId;
	Ship->CurrentLocationId = 0; // Arrive at first location in system
	Ship->Status = EShipStatus::Active;

	// Log fuel consumption (but don't enforce empty = stuck in Sprint 4)
	int32 FuelCost = 10; // Placeholder cost
	Ship->CurrentFuel = FMath::Max(0, Ship->CurrentFuel - FuelCost);
	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Ship %d consumed %d fuel (remaining: %d)"), 
		ShipId, FuelCost, Ship->CurrentFuel);

	// Update player location if aboard this ship
	for (auto& PlayerPair : Players)
	{
		FPlayerProfileData& Player = PlayerPair.Value;
		if (Player.CurrentShipId == ShipId)
		{
			Player.CurrentSystemId = TargetSystemId;
			Player.CurrentLocationId = 0;

			// Update fog-of-war
			UpdateFogOfWarFromMovement(Player.PlayerId, OldSystemId, TargetSystemId);
		}
	}

	// Update crew location if assigned to this ship
	for (int32 CrewId : Ship->AssignedCrewIds)
	{
		FCrewMemberData* CrewMember = Crew.Find(CrewId);
		if (CrewMember)
		{
			CrewMember->CurrentSystemId = TargetSystemId;
			CrewMember->CurrentLocationId = 0;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Ship %d moved from system %d to system %d"), 
		ShipId, OldSystemId, TargetSystemId);

	return true;
}

bool UPlayerSubsystem::BoardShip(int32 PlayerId, int32 ShipId)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] BoardShip called on CLIENT - ignoring"));
		return false;
	}

	// Get player and ship
	FPlayerProfileData* Player = Players.Find(PlayerId);
	const FShipData* Ship = Ships.Find(ShipId);

	if (!Player || !Ship)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot board - player %d or ship %d not found"), 
			PlayerId, ShipId);
		return false;
	}

	// Validate ownership
	if (Ship->OwnerPlayerId != PlayerId)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot board - player %d does not own ship %d"), 
			PlayerId, ShipId);
		return false;
	}

	// Validate same location
	if (Player->CurrentSystemId != Ship->CurrentSystemId || 
		Player->CurrentLocationId != Ship->CurrentLocationId)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot board - player and ship not at same location"));
		return false;
	}

	// Board ship
	Player->CurrentShipId = ShipId;

	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Player %d boarded ship %d"), PlayerId, ShipId);

	return true;
}

bool UPlayerSubsystem::DisembarkShip(int32 PlayerId)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] DisembarkShip called on CLIENT - ignoring"));
		return false;
	}

	// Get player
	FPlayerProfileData* Player = Players.Find(PlayerId);
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Cannot disembark - player %d not found"), PlayerId);
		return false;
	}

	if (Player->CurrentShipId == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] Player %d not aboard a ship"), PlayerId);
		return false;
	}

	int32 OldShipId = Player->CurrentShipId;
	Player->CurrentShipId = -1;

	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Player %d disembarked from ship %d"), PlayerId, OldShipId);

	return true;
}

// ============================================================================
// DEBUG & TESTING
// ============================================================================

void UPlayerSubsystem::PrintPlayerInfo(int32 PlayerId) const
{
	const FPlayerProfileData* Player = Players.Find(PlayerId);
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem DEBUG] Player %d not found"), PlayerId);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========== PLAYER INFO: %d =========="), PlayerId);
	UE_LOG(LogTemp, Log, TEXT("Name: %s"), *Player->PlayerName);
	UE_LOG(LogTemp, Log, TEXT("Credits: %d"), Player->Credits);
	UE_LOG(LogTemp, Log, TEXT("Location: System %d, Location %d"), Player->CurrentSystemId, Player->CurrentLocationId);
	UE_LOG(LogTemp, Log, TEXT("Current Ship: %d"), Player->CurrentShipId);
	UE_LOG(LogTemp, Log, TEXT("Owned Ships: %d"), Player->OwnedShipIds.Num());
	UE_LOG(LogTemp, Log, TEXT("Crew Count: %d"), Player->CrewIds.Num());
	UE_LOG(LogTemp, Log, TEXT("Status: %s"), 
		Player->Status == EPlayerStatus::Active ? TEXT("Active") :
		Player->Status == EPlayerStatus::Idle ? TEXT("Idle") : TEXT("Offline"));
	UE_LOG(LogTemp, Log, TEXT("===================================="));
}

void UPlayerSubsystem::PrintShipInfo(int32 ShipId) const
{
	const FShipData* Ship = Ships.Find(ShipId);
	if (!Ship)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem DEBUG] Ship %d not found"), ShipId);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========== SHIP INFO: %d =========="), ShipId);
	UE_LOG(LogTemp, Log, TEXT("Name: %s"), *Ship->ShipName);
	UE_LOG(LogTemp, Log, TEXT("Owner: Player %d"), Ship->OwnerPlayerId);
	UE_LOG(LogTemp, Log, TEXT("Location: System %d, Location %d"), Ship->CurrentSystemId, Ship->CurrentLocationId);
	UE_LOG(LogTemp, Log, TEXT("Status: %s"), 
		Ship->Status == EShipStatus::Docked ? TEXT("Docked") :
		Ship->Status == EShipStatus::Active ? TEXT("Active") :
		Ship->Status == EShipStatus::InTransit ? TEXT("In Transit") : TEXT("Disabled"));
	UE_LOG(LogTemp, Log, TEXT("Cargo: %d / %d"), Ship->CurrentCargoUsed, Ship->CargoCapacity);
	UE_LOG(LogTemp, Log, TEXT("Fuel: %d / %d"), Ship->CurrentFuel, Ship->FuelCapacity);
	UE_LOG(LogTemp, Log, TEXT("Crew: %d / %d"), Ship->AssignedCrewIds.Num(), Ship->CrewCapacity);
	UE_LOG(LogTemp, Log, TEXT("===================================="));
}

void UPlayerSubsystem::PrintCrewInfo(int32 CrewId) const
{
	const FCrewMemberData* CrewMember = Crew.Find(CrewId);
	if (!CrewMember)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem DEBUG] Crew %d not found"), CrewId);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========== CREW INFO: %d =========="), CrewId);
	UE_LOG(LogTemp, Log, TEXT("Name: %s"), *CrewMember->Name);
	UE_LOG(LogTemp, Log, TEXT("Owner: Player %d"), CrewMember->OwnerPlayerId);
	UE_LOG(LogTemp, Log, TEXT("Assigned Ship: %d"), CrewMember->AssignedShipId);
	UE_LOG(LogTemp, Log, TEXT("Location: System %d, Location %d"), CrewMember->CurrentSystemId, CrewMember->CurrentLocationId);
	UE_LOG(LogTemp, Log, TEXT("Status: %s"), 
		CrewMember->Status == ECrewStatus::Available ? TEXT("Available") :
		CrewMember->Status == ECrewStatus::Assigned ? TEXT("Assigned") :
		CrewMember->Status == ECrewStatus::Injured ? TEXT("Injured") : TEXT("Dead"));
	UE_LOG(LogTemp, Log, TEXT("===================================="));
}

void UPlayerSubsystem::PrintAllPlayerState(int32 PlayerId) const
{
	PrintPlayerInfo(PlayerId);

	const FPlayerProfileData* Player = Players.Find(PlayerId);
	if (!Player)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Player %d Ships:"), PlayerId);
	for (int32 ShipId : Player->OwnedShipIds)
	{
		PrintShipInfo(ShipId);
	}

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Player %d Crew:"), PlayerId);
	for (int32 CrewId : Player->CrewIds)
	{
		PrintCrewInfo(CrewId);
	}
}

int32 UPlayerSubsystem::CreateTestPlayer(const FString& PlayerName)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] CreateTestPlayer called on CLIENT - ignoring"));
		return -1;
	}

	// Create player at auto-selected starting location
	int32 PlayerId = CreatePlayer(PlayerName, -1);
	if (PlayerId == -1)
	{
		return -1;
	}

	const FPlayerProfileData* Player = Players.Find(PlayerId);
	if (!Player)
	{
		return -1;
	}

	// Create starter ship at player's location
	int32 ShipId = CreateStarterShip(PlayerId, Player->CurrentSystemId, Player->CurrentLocationId);
	if (ShipId == -1)
	{
		return -1;
	}

	// Create 3 crew members assigned to ship
	for (int32 i = 0; i < 3; i++)
	{
		CreateCrewMember(PlayerId, ShipId);
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Created test player %d with ship and crew"), PlayerId);

	return PlayerId;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void UPlayerSubsystem::InitializeCrewNamePool()
{
	// First names (mix of various origins)
	CrewFirstNames = {
		TEXT("Alex"), TEXT("Jordan"), TEXT("Morgan"), TEXT("Casey"), TEXT("Taylor"),
		TEXT("Riley"), TEXT("Avery"), TEXT("Quinn"), TEXT("Skyler"), TEXT("Cameron"),
		TEXT("Aria"), TEXT("Zara"), TEXT("Kai"), TEXT("Nova"), TEXT("Phoenix"),
		TEXT("River"), TEXT("Sage"), TEXT("Rowan"), TEXT("Blake"), TEXT("Drew"),
		TEXT("Finn"), TEXT("Reese"), TEXT("Charlie"), TEXT("Sam"), TEXT("Jamie"),
		TEXT("Ash"), TEXT("Parker"), TEXT("Harper"), TEXT("Emery"), TEXT("Dakota")
	};

	// Last names (various sci-fi/space-themed)
	CrewLastNames = {
		TEXT("Vega"), TEXT("Orion"), TEXT("Nova"), TEXT("Stellar"), TEXT("Cosmos"),
		TEXT("Nebula"), TEXT("Quantum"), TEXT("Horizon"), TEXT("Eclipse"), TEXT("Zenith"),
		TEXT("Atlas"), TEXT("Titan"), TEXT("Comet"), TEXT("Pulsar"), TEXT("Void"),
		TEXT("Starling"), TEXT("Stratos"), TEXT("Celestia"), TEXT("Aether"), TEXT("Nexus"),
		TEXT("Drift"), TEXT("Cipher"), TEXT("Vector"), TEXT("Helix"), TEXT("Talon"),
		TEXT("Raven"), TEXT("Drake"), TEXT("Hawk"), TEXT("Falcon"), TEXT("Phoenix")
	};
}

FString UPlayerSubsystem::GenerateRandomCrewName() const
{
	if (CrewFirstNames.Num() == 0 || CrewLastNames.Num() == 0)
	{
		return TEXT("Unknown Crew");
	}

	int32 FirstIndex = FMath::RandRange(0, CrewFirstNames.Num() - 1);
	int32 LastIndex = FMath::RandRange(0, CrewLastNames.Num() - 1);

	return FString::Printf(TEXT("%s %s"), *CrewFirstNames[FirstIndex], *CrewLastNames[LastIndex]);
}

bool UPlayerSubsystem::IsValidLocation(int32 SystemId, int32 LocationId) const
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe)
	{
		return false;
	}

	const FUniverseData& UniverseData = Universe->GetUniverseData();
	const FStarSystemData* System = UniverseData.Systems.FindByPredicate(
		[SystemId](const FStarSystemData& S) { return S.SystemId == SystemId; }
	);

	if (!System)
	{
		return false;
	}

	return LocationId >= 0 && LocationId < System->Locations.Num();
}

bool UPlayerSubsystem::AreSystemsConnected(int32 SystemA, int32 SystemB) const
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe)
	{
		return false;
	}

	// Use UniverseSubsystem's path-finding to check connectivity
	TArray<int32> Path = Universe->FindPath(SystemA, SystemB);
	return Path.Num() > 0;
}

void UPlayerSubsystem::UpdateFogOfWarFromMovement(int32 PlayerId, int32 OldSystemId, int32 NewSystemId)
{
	UFogOfWarSubsystem* FogOfWar = EnsureFogOfWarSubsystem();
	if (!FogOfWar)
	{
		return;
	}

	// Unfocus old system
	if (OldSystemId != -1)
	{
		FogOfWar->OnPlayerUnfocusSystem(PlayerId, FName(*FString::FromInt(OldSystemId)));
	}

	// Discover and focus new system
	FogOfWar->DiscoverSystem(PlayerId, FName(*FString::FromInt(NewSystemId)));
	FogOfWar->CompleteSurvey(PlayerId, FName(*FString::FromInt(NewSystemId)));
	FogOfWar->OnPlayerFocusSystem(PlayerId, FName(*FString::FromInt(NewSystemId)));
}

UUniverseSubsystem* UPlayerSubsystem::EnsureUniverseSubsystem() const
{
	if (!UniverseSubsystem)
	{
		// Cast away const for lazy initialization (common pattern for cached subsystem refs)
		UPlayerSubsystem* MutableThis = const_cast<UPlayerSubsystem*>(this);
		MutableThis->UniverseSubsystem = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();

		if (!MutableThis->UniverseSubsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Failed to acquire UniverseSubsystem reference"));
		}
	}

	return UniverseSubsystem;
}

UFogOfWarSubsystem* UPlayerSubsystem::EnsureFogOfWarSubsystem() const
{
	if (!FogOfWarSubsystem)
	{
		// Cast away const for lazy initialization (common pattern for cached subsystem refs)
		UPlayerSubsystem* MutableThis = const_cast<UPlayerSubsystem*>(this);
		MutableThis->FogOfWarSubsystem = GetGameInstance()->GetSubsystem<UFogOfWarSubsystem>();

		if (!MutableThis->FogOfWarSubsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Failed to acquire FogOfWarSubsystem reference"));
		}
	}

	return FogOfWarSubsystem;
}
