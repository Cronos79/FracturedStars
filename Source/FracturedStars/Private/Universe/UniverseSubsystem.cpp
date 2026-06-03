// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Universe/UniverseSubsystem.h"
#include "Universe/UniverseGenerator.h"
#include "Universe/EconomySubsystem.h"
#include "Containers/Queue.h"

// Network authority helpers
bool UUniverseSubsystem::IsAuthority() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return true; // Standalone/editor - act as authority
	}

	ENetMode NetMode = World->GetNetMode();
	return NetMode == NM_DedicatedServer || NetMode == NM_ListenServer || NetMode == NM_Standalone;
}

bool UUniverseSubsystem::IsClient() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return World->GetNetMode() == NM_Client;
}

void UUniverseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Only start time advancement on server/standalone
	// Clients will receive time updates via replication (future)
	if (IsAuthority())
	{
		// Set up time advancement timer (10 times per second for smooth time progression)
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(TimeAdvancementTimer, [this]()
			{
				if (bIsGenerated && !bTimePaused)
				{
					// Get real delta time (0.1 seconds)
					float DeltaTime = 0.1f;

					// Apply time scale to get game time delta
					double DeltaGameSeconds = DeltaTime * UniverseData.Config.TimeScale;

					// Update the calendar
					UpdateGameTime(DeltaGameSeconds);
				}
			}, 0.1f, true);
		}

		UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Initialized (Server/Standalone)"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Initialized (Client - read-only mode)"));
	}
}

void UUniverseSubsystem::Deinitialize()
{
	// Clear timer (only needed on authority, but safe to call on clients)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimeAdvancementTimer);
	}

	Super::Deinitialize();

	if (IsAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Deinitialized (Server/Standalone)"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Deinitialized (Client)"));
	}
}

bool UUniverseSubsystem::GenerateUniverse(const FUniverseConfig& Config)
{
	// Only server/standalone can generate universe
	// Clients will generate locally from seed (future networking sprint)
	if (IsClient())
	{
		UE_LOG(LogTemp, Error, TEXT("UniverseSubsystem: Clients cannot generate universe - server authority required!"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Generating universe (Server/Standalone)..."));

	// Generate universe
	UniverseData = UUniverseGenerator::GenerateUniverse(Config);
	bIsGenerated = true;

	// Initialize game time from config
	UniverseData.CurrentTime = FUniverseTime(Config.StartYear, Config.StartMonth, Config.StartDay);

	UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Universe ready! (%d systems)"), UniverseData.Systems.Num());
	UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Game time initialized to %s"), *GetFormattedDate());

	return true;
}

bool UUniverseSubsystem::GetSystemById(int32 SystemId, FStarSystemData& OutSystem) const
{
	if (!bIsGenerated)
	{
		UE_LOG(LogTemp, Warning, TEXT("UniverseSubsystem: Universe not generated"));
		return false;
	}

	if (SystemId < 0 || SystemId >= UniverseData.Systems.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UniverseSubsystem: Invalid system ID %d"), SystemId);
		return false;
	}

	OutSystem = UniverseData.Systems[SystemId];
	return true;
}

TArray<FStarSystemData> UUniverseSubsystem::GetSystemsByRegion(ERegionType RegionType) const
{
	TArray<FStarSystemData> Result;

	if (!bIsGenerated)
		return Result;

	for (const FStarSystemData& System : UniverseData.Systems)
	{
		if (System.RegionType == RegionType)
		{
			Result.Add(System);
		}
	}

	return Result;
}

TArray<int32> UUniverseSubsystem::FindPath(int32 StartSystemId, int32 EndSystemId) const
{
	if (!bIsGenerated)
		return TArray<int32>();

	return FindPathInternal(StartSystemId, EndSystemId);
}

int32 UUniverseSubsystem::GetJumpDistance(int32 SystemA, int32 SystemB) const
{
	TArray<int32> Path = FindPath(SystemA, SystemB);
	return Path.Num() > 0 ? Path.Num() - 1 : -1;
}

void UUniverseSubsystem::PrintUniverseInfo() const
{
	if (!bIsGenerated)
	{
		UE_LOG(LogTemp, Warning, TEXT("Universe not generated yet"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("UNIVERSE INFORMATION"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Seed: %d"), UniverseData.Config.Seed);
	UE_LOG(LogTemp, Log, TEXT("Total Systems: %d"), UniverseData.Systems.Num());
	UE_LOG(LogTemp, Log, TEXT("Factions: %d"), UniverseData.Config.FactionCount);
	UE_LOG(LogTemp, Log, TEXT("Generated: %s"), *UniverseData.GenerationTime.ToString());

	// Count regions
	TMap<ERegionType, int32> RegionCounts;
	for (const FStarSystemData& System : UniverseData.Systems)
	{
		int32& Count = RegionCounts.FindOrAdd(System.RegionType, 0);
		Count++;
	}

	UE_LOG(LogTemp, Log, TEXT("\nRegion Distribution:"));
	for (const auto& Pair : RegionCounts)
	{
		FString RegionName;
		switch (Pair.Key)
		{
		case ERegionType::FactionCore: RegionName = TEXT("Faction Core"); break;
		case ERegionType::FactionFrontier: RegionName = TEXT("Frontier"); break;
		case ERegionType::Neutral: RegionName = TEXT("Neutral"); break;
		case ERegionType::Lawless: RegionName = TEXT("Lawless"); break;
		case ERegionType::Disputed: RegionName = TEXT("Disputed"); break;
		default: RegionName = TEXT("Unknown"); break;
		}

		float Percentage = 100.0f * Pair.Value / UniverseData.Systems.Num();
		UE_LOG(LogTemp, Log, TEXT("  %s: %d (%.1f%%)"), *RegionName, Pair.Value, Percentage);
	}

	// Faction home systems
	UE_LOG(LogTemp, Log, TEXT("\nFaction Home Systems:"));
	for (int32 i = 0; i < UniverseData.FactionHomeSystems.Num(); ++i)
	{
		int32 SystemId = UniverseData.FactionHomeSystems[i];
		const FStarSystemData& System = UniverseData.Systems[SystemId];
		UE_LOG(LogTemp, Log, TEXT("  Faction %d: System %d (%s) at %s"),
			i, SystemId, *System.SystemName, *System.Coordinates.ToString());
	}

	// Sample systems
	UE_LOG(LogTemp, Log, TEXT("\nSample Systems:"));
	for (int32 i = 0; i < FMath::Min(5, UniverseData.Systems.Num()); ++i)
	{
		const FStarSystemData& System = UniverseData.Systems[i];
		UE_LOG(LogTemp, Log, TEXT("  [%d] %s - Lawfulness: %.2f - Connections: %d"),
			System.SystemId, *System.SystemName, System.Lawfulness, System.ConnectedSystemIds.Num());
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void UUniverseSubsystem::PrintSystemInfo(int32 SystemId) const
{
	FStarSystemData System;
	if (!GetSystemById(SystemId, System))
		return;

	FString RegionName;
	switch (System.RegionType)
	{
	case ERegionType::FactionCore: RegionName = TEXT("Faction Core"); break;
	case ERegionType::FactionFrontier: RegionName = TEXT("Frontier"); break;
	case ERegionType::Neutral: RegionName = TEXT("Neutral"); break;
	case ERegionType::Lawless: RegionName = TEXT("Lawless"); break;
	case ERegionType::Disputed: RegionName = TEXT("Disputed"); break;
	default: RegionName = TEXT("Unknown"); break;
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("SYSTEM: %s (ID: %d)"), *System.SystemName, System.SystemId);
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Region: %s"), *RegionName);
	UE_LOG(LogTemp, Log, TEXT("Lawfulness: %.2f"), System.Lawfulness);
	UE_LOG(LogTemp, Log, TEXT("Controlling Faction: %d"), System.ControllingFactionId);
	UE_LOG(LogTemp, Log, TEXT("Distance to Faction Core: %d jumps"), System.DistanceToFactionCore);
	UE_LOG(LogTemp, Log, TEXT("Coordinates: %s"), *System.Coordinates.ToString());
	UE_LOG(LogTemp, Log, TEXT("Connections: %d"), System.ConnectedSystemIds.Num());

	if (System.ConnectedSystemIds.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Connected Systems:"));
		for (int32 ConnectedId : System.ConnectedSystemIds)
		{
			const FStarSystemData& Connected = UniverseData.Systems[ConnectedId];
			UE_LOG(LogTemp, Log, TEXT("  -> %s (ID: %d)"), *Connected.SystemName, ConnectedId);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void UUniverseSubsystem::PrintTimeInfo() const
{
	if (!bIsGenerated)
	{
		UE_LOG(LogTemp, Warning, TEXT("Universe not generated yet"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========== Game Time Info =========="));
	UE_LOG(LogTemp, Log, TEXT("Current Date: %s"), *GetFormattedDate());
	UE_LOG(LogTemp, Log, TEXT("Current Time: %s"), *GetFormattedTime());
	UE_LOG(LogTemp, Log, TEXT("Full DateTime: %s"), *GetFormattedDateTime());
	UE_LOG(LogTemp, Log, TEXT("Elapsed Days: %.2f"), (float)GetElapsedDays());
	UE_LOG(LogTemp, Log, TEXT("Total Elapsed: %.1f hours (%.1f seconds)"), 
		(float)(UniverseData.CurrentTime.TotalElapsedSeconds / 3600.0), 
		(float)UniverseData.CurrentTime.TotalElapsedSeconds);
	UE_LOG(LogTemp, Log, TEXT("Time Scale: %.1fx (1 real hour = %.1f game hours)"),
		UniverseData.Config.TimeScale,
		UniverseData.Config.TimeScale);
	UE_LOG(LogTemp, Log, TEXT("Time Paused: %s"), bTimePaused ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Log, TEXT("===================================="));
}

TArray<int32> UUniverseSubsystem::FindPathInternal(int32 StartId, int32 EndId) const
{
	if (StartId < 0 || StartId >= UniverseData.Systems.Num() ||
		EndId < 0 || EndId >= UniverseData.Systems.Num())
	{
		return TArray<int32>();
	}

	// BFS
	TMap<int32, int32> CameFrom;
	TQueue<int32> Queue;
	TSet<int32> Visited;

	Queue.Enqueue(StartId);
	Visited.Add(StartId);
	CameFrom.Add(StartId, -1);

	while (!Queue.IsEmpty())
	{
		int32 CurrentId;
		Queue.Dequeue(CurrentId);

		if (CurrentId == EndId)
		{
			// Reconstruct path
			TArray<int32> Path;
			int32 Step = EndId;
			while (Step != -1)
			{
				Path.Insert(Step, 0);
				Step = CameFrom[Step];
			}
			return Path;
		}

		const FStarSystemData& CurrentSystem = UniverseData.Systems[CurrentId];
		for (int32 ConnectedId : CurrentSystem.ConnectedSystemIds)
		{
			if (!Visited.Contains(ConnectedId))
			{
				Queue.Enqueue(ConnectedId);
				Visited.Add(ConnectedId);
				CameFrom.Add(ConnectedId, CurrentId);
			}
		}
	}

	return TArray<int32>(); // No path found
}

// Sprint 2: Content query implementations

TArray<FCelestialBodyData> UUniverseSubsystem::GetCelestialBodiesInSystem(int32 SystemId) const
{
if (!bIsGenerated)
{
UE_LOG(LogTemp, Warning, TEXT("Universe not generated yet"));
return TArray<FCelestialBodyData>();
}

if (SystemId < 0 || SystemId >= UniverseData.Systems.Num())
{
UE_LOG(LogTemp, Warning, TEXT("Invalid system ID: %d"), SystemId);
return TArray<FCelestialBodyData>();
}

return UniverseData.Systems[SystemId].CelestialBodies;
}

TArray<FLocationData> UUniverseSubsystem::GetLocationsInSystem(int32 SystemId) const
{
if (!bIsGenerated)
{
UE_LOG(LogTemp, Warning, TEXT("Universe not generated yet"));
return TArray<FLocationData>();
}

if (SystemId < 0 || SystemId >= UniverseData.Systems.Num())
{
UE_LOG(LogTemp, Warning, TEXT("Invalid system ID: %d"), SystemId);
return TArray<FLocationData>();
}

return UniverseData.Systems[SystemId].Locations;
}

TArray<FLocationData> UUniverseSubsystem::GetLocationsByOwner(int32 FactionId) const
{
TArray<FLocationData> OwnedLocations;

if (!bIsGenerated)
{
UE_LOG(LogTemp, Warning, TEXT("Universe not generated yet"));
return OwnedLocations;
}

for (const FStarSystemData& System : UniverseData.Systems)
{
for (const FLocationData& Location : System.Locations)
{
if (Location.OwningFactionId == FactionId)
{
OwnedLocations.Add(Location);
}
}
}

return OwnedLocations;
}

void UUniverseSubsystem::PrintSystemContent(int32 SystemId) const
{
if (!bIsGenerated)
{
UE_LOG(LogTemp, Warning, TEXT("Universe not generated yet"));
return;
}

if (SystemId < 0 || SystemId >= UniverseData.Systems.Num())
{
UE_LOG(LogTemp, Warning, TEXT("Invalid system ID: %d"), SystemId);
return;
}

const FStarSystemData& System = UniverseData.Systems[SystemId];

UE_LOG(LogTemp, Log, TEXT("========================================"));
UE_LOG(LogTemp, Log, TEXT("SYSTEM CONTENT: %s (ID: %d)"), *System.SystemName, System.SystemId);
UE_LOG(LogTemp, Log, TEXT("========================================"));

UE_LOG(LogTemp, Log, TEXT("Celestial Bodies (%d):"), System.CelestialBodies.Num());
for (const FCelestialBodyData& Body : System.CelestialBodies)
{
FString BodyTypeStr;
switch (Body.BodyType)
{
case ECelestialBodyType::Star: BodyTypeStr = TEXT("Star"); break;
case ECelestialBodyType::Planet: BodyTypeStr = TEXT("Planet"); break;
case ECelestialBodyType::Moon: BodyTypeStr = TEXT("Moon"); break;
case ECelestialBodyType::AsteroidField: BodyTypeStr = TEXT("Asteroid Field"); break;
case ECelestialBodyType::GasGiant: BodyTypeStr = TEXT("Gas Giant"); break;
case ECelestialBodyType::IceGiant: BodyTypeStr = TEXT("Ice Giant"); break;
case ECelestialBodyType::DwarfPlanet: BodyTypeStr = TEXT("Dwarf Planet"); break;
case ECelestialBodyType::Anomaly: BodyTypeStr = TEXT("Anomaly"); break;
default: BodyTypeStr = TEXT("Unknown"); break;
}
UE_LOG(LogTemp, Log, TEXT("  - %s [%s] (Resource: %.2f)"), *Body.BodyName, *BodyTypeStr, Body.ResourceRichness);
}

UE_LOG(LogTemp, Log, TEXT("Locations (%d):"), System.Locations.Num());
int64 TotalPopulation = 0;
for (const FLocationData& Location : System.Locations)
{
TotalPopulation += Location.Population;
FString LocationTypeStr;
switch (Location.LocationType)
{
case ELocationType::Station: LocationTypeStr = TEXT("Station"); break;
case ELocationType::TradeHub: LocationTypeStr = TEXT("Trade Hub"); break;
case ELocationType::MiningColony: LocationTypeStr = TEXT("Mining Colony"); break;
case ELocationType::ResearchFacility: LocationTypeStr = TEXT("Research Facility"); break;
case ELocationType::MilitaryBase: LocationTypeStr = TEXT("Military Base"); break;
case ELocationType::PirateOutpost: LocationTypeStr = TEXT("Pirate Outpost"); break;
case ELocationType::AbandonedFacility: LocationTypeStr = TEXT("Abandoned Facility"); break;
case ELocationType::Shipyard: LocationTypeStr = TEXT("Shipyard"); break;
case ELocationType::RefuelingDepot: LocationTypeStr = TEXT("Refueling Depot"); break;
case ELocationType::Colony: LocationTypeStr = TEXT("Colony"); break;
default: LocationTypeStr = TEXT("Unknown"); break;
}
FString OwnerStr = (Location.OwningFactionId == -1) ? TEXT("Independent") : FString::Printf(TEXT("Faction %d"), Location.OwningFactionId);
UE_LOG(LogTemp, Log, TEXT("  - %s [%s] | Owner: %s | Pop: %d | Security: %.2f"), *Location.LocationName, *LocationTypeStr, *OwnerStr, Location.Population, Location.SecurityRating);
}

	UE_LOG(LogTemp, Log, TEXT("Total System Population: %lld"), TotalPopulation);
	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

// Sprint 3: Economy integration functions

void UUniverseSubsystem::InitializeEconomy()
{
	if (!bIsGenerated)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Cannot initialize economy: universe not generated"));
		return;
	}

	// Only server/standalone can initialize economy
	// Clients will query market data on-demand (future networking sprint)
	if (IsClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Clients cannot initialize economy - server authority required!"));
		return;
	}

	// Get economy subsystem
	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[UniverseSubsystem] EconomySubsystem not found!"));
		return;
	}

	// Initialize economy with our universe data
	EconomySubsystem->InitializeEconomy(UniverseData);

	UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] Economy initialized for universe (Server/Standalone)"));
}

void UUniverseSubsystem::SetActiveEconomySystem(int32 SystemId)
{
	if (!bIsGenerated)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Cannot set active system: universe not generated"));
		return;
	}

	// Only server/standalone can set active system
	// This will be the hook for "player enters system = wake up live sim"
	if (IsClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Clients cannot set active economy system - server authority required!"));
		return;
	}

	// Phase 2: Warn if trying to activate a system with no players
	// (Allowed for manual testing, but in production should only be player-driven)
	if (!HasPlayersInSystem(SystemId))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Setting active system %d with no players present (manual override)"), SystemId);
	}

	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[UniverseSubsystem] EconomySubsystem not found!"));
		return;
	}

	EconomySubsystem->SetActiveSystem(UniverseData, SystemId);

	UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] Active economy system set to %d (Server/Standalone)"), SystemId);
}

int32 UUniverseSubsystem::GetActiveEconomySystemId() const
{
	return UniverseData.ActiveSystemId;
}

FMarketState UUniverseSubsystem::GetMarketState(int32 SystemId, int32 LocationId) const
{
	if (!bIsGenerated)
	{
		return FMarketState();
	}

	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		return FMarketState();
	}

	return EconomySubsystem->GetMarketState(UniverseData, SystemId, LocationId);
}

float UUniverseSubsystem::GetGoodPrice(int32 SystemId, int32 LocationId, EGoodType GoodType) const
{
	if (!bIsGenerated)
	{
		return 0.0f;
	}

	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		return 0.0f;
	}

	return EconomySubsystem->GetGoodPrice(UniverseData, SystemId, LocationId, GoodType);
}

bool UUniverseSubsystem::HasShortage(int32 SystemId, int32 LocationId, EGoodType GoodType) const
{
	if (!bIsGenerated)
	{
		return false;
	}

	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		return false;
	}

	return EconomySubsystem->HasShortage(UniverseData, SystemId, LocationId, GoodType);
}

TArray<int32> UUniverseSubsystem::FindShortageLocations(EGoodType GoodType) const
{
	if (!bIsGenerated)
	{
		return TArray<int32>();
	}

	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		return TArray<int32>();
	}

	return EconomySubsystem->FindShortageLocations(UniverseData, GoodType);
}

void UUniverseSubsystem::PrintEconomyStats() const
{
	if (!bIsGenerated)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Cannot print economy stats: universe not generated"));
		return;
	}

	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[UniverseSubsystem] EconomySubsystem not found!"));
		return;
	}

	EconomySubsystem->PrintEconomyStats(UniverseData);
}

// Game Time Functions

FUniverseTime UUniverseSubsystem::GetGameTime() const
{
	return UniverseData.CurrentTime;
}

FString UUniverseSubsystem::GetFormattedDate() const
{
	const FUniverseTime& Time = UniverseData.CurrentTime;
	FString MonthName = GetMonthName(Time.Month);
	return FString::Printf(TEXT("%s %d, %d"), *MonthName, Time.Day, Time.Year);
}

FString UUniverseSubsystem::GetFormattedTime() const
{
	const FUniverseTime& Time = UniverseData.CurrentTime;
	return FString::Printf(TEXT("%02d:%02d:%02d"), Time.Hour, Time.Minute, Time.Second);
}

FString UUniverseSubsystem::GetFormattedDateTime() const
{
	return FString::Printf(TEXT("%s %s"), *GetFormattedDate(), *GetFormattedTime());
}

int32 UUniverseSubsystem::GetElapsedDays() const
{
	// Convert total elapsed seconds to days
	return FMath::FloorToInt(UniverseData.CurrentTime.TotalElapsedSeconds / 86400.0);
}

void UUniverseSubsystem::SetTimeScale(float NewTimeScale)
{
	UniverseData.Config.TimeScale = FMath::Max(0.0f, NewTimeScale);
	UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] Time scale set to %.2f"), UniverseData.Config.TimeScale);
}

float UUniverseSubsystem::GetTimeScale() const
{
	return UniverseData.Config.TimeScale;
}

void UUniverseSubsystem::SetTimePaused(bool bPaused)
{
	bTimePaused = bPaused;
	UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] Time %s"), bPaused ? TEXT("paused") : TEXT("resumed"));
}

bool UUniverseSubsystem::IsTimePaused() const
{
	return bTimePaused;
}

// Private helper functions

bool UUniverseSubsystem::IsLeapYear(int32 Year) const
{
	// Leap year if divisible by 4, except century years must be divisible by 400
	if (Year % 400 == 0) return true;
	if (Year % 100 == 0) return false;
	if (Year % 4 == 0) return true;
	return false;
}

int32 UUniverseSubsystem::GetDaysInMonth(int32 Month, int32 Year) const
{
	static const int32 DaysPerMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if (Month < 1 || Month > 12)
		return 30; // Default fallback

	int32 Days = DaysPerMonth[Month - 1];

	// Add leap day for February
	if (Month == 2 && IsLeapYear(Year))
		Days++;

	return Days;
}

FString UUniverseSubsystem::GetMonthName(int32 Month) const
{
	static const TArray<FString> MonthNames = {
		TEXT("January"), TEXT("February"), TEXT("March"), TEXT("April"),
		TEXT("May"), TEXT("June"), TEXT("July"), TEXT("August"),
		TEXT("September"), TEXT("October"), TEXT("November"), TEXT("December")
	};

	if (Month < 1 || Month > 12)
		return TEXT("Unknown");

	return MonthNames[Month - 1];
}

void UUniverseSubsystem::UpdateGameTime(double DeltaGameSeconds)
{
	FUniverseTime& Time = UniverseData.CurrentTime;

	// Add to total elapsed time
	Time.TotalElapsedSeconds += DeltaGameSeconds;

	// Add seconds
	Time.Second += FMath::FloorToInt(DeltaGameSeconds);
	DeltaGameSeconds -= FMath::FloorToInt(DeltaGameSeconds);

	// Handle second overflow
	if (Time.Second >= 60)
	{
		int32 Minutes = Time.Second / 60;
		Time.Second = Time.Second % 60;
		Time.Minute += Minutes;
	}

	// Handle minute overflow
	if (Time.Minute >= 60)
	{
		int32 Hours = Time.Minute / 60;
		Time.Minute = Time.Minute % 60;
		Time.Hour += Hours;
	}

	// Handle hour overflow
	if (Time.Hour >= 24)
	{
		int32 Days = Time.Hour / 24;
		Time.Hour = Time.Hour % 24;
		Time.Day += Days;
	}

	// Handle day/month/year overflow
	while (Time.Day > GetDaysInMonth(Time.Month, Time.Year))
	{
		Time.Day -= GetDaysInMonth(Time.Month, Time.Year);
		Time.Month++;

		if (Time.Month > 12)
		{
			Time.Month = 1;
			Time.Year++;
		}
	}
}

// Player Presence Tracking (Phase 2: Multiplayer)
// RTS/Stellaris Model: Player is a strategic camera controller, not a physical character
// "Enter/Leave" refers to UI focus/observation, not physical location in the game world

bool UUniverseSubsystem::HasPlayersInSystem(int32 SystemId) const
{
	const TArray<APlayerController*>* Players = PlayersInSystem.Find(SystemId);
	return Players && Players->Num() > 0;
}

int32 UUniverseSubsystem::GetPlayerCountInSystem(int32 SystemId) const
{
	const TArray<APlayerController*>* Players = PlayersInSystem.Find(SystemId);
	return Players ? Players->Num() : 0;
}

void UUniverseSubsystem::OnPlayerEnterSystem(APlayerController* Player, int32 SystemId)
{
	// Server-only operation
	if (IsClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Clients cannot register player presence - server authority required!"));
		return;
	}

	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] OnPlayerEnterSystem called with null player!"));
		return;
	}

	if (!bIsGenerated)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Cannot enter system: universe not generated"));
		return;
	}

	if (SystemId < 0 || SystemId >= UniverseData.Systems.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Invalid system ID: %d"), SystemId);
		return;
	}

	// Check if this is the first player in the system
	bool bWasEmpty = !HasPlayersInSystem(SystemId);

	// Add player to tracking
	TArray<APlayerController*>& Players = PlayersInSystem.FindOrAdd(SystemId);

	// Prevent duplicate entries
	if (!Players.Contains(Player))
	{
		Players.Add(Player);

		const FStarSystemData& System = UniverseData.Systems[SystemId];
		UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] Player entered %s (ID: %d). Players in system: %d"), 
			*System.SystemName, SystemId, Players.Num());

		// If this is the first player, wake up the system (catch-up + live simulation)
		if (bWasEmpty)
		{
			UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] First player in system - waking up live simulation"));
			SetActiveEconomySystem(SystemId);
		}
	}
}

void UUniverseSubsystem::OnPlayerLeaveSystem(APlayerController* Player, int32 SystemId)
{
	// Server-only operation
	if (IsClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Clients cannot unregister player presence - server authority required!"));
		return;
	}

	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] OnPlayerLeaveSystem called with null player!"));
		return;
	}

	if (!bIsGenerated)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Cannot leave system: universe not generated"));
		return;
	}

	// Find the players array for this system
	TArray<APlayerController*>* Players = PlayersInSystem.Find(SystemId);
	if (!Players)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] No players tracked in system %d"), SystemId);
		return;
	}

	// Remove the player
	int32 RemovedCount = Players->Remove(Player);
	if (RemovedCount > 0)
	{
		const FStarSystemData& System = UniverseData.Systems[SystemId];
		UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] Player left %s (ID: %d). Players remaining: %d"), 
			*System.SystemName, SystemId, Players->Num());

		// If this was the last player, put system to sleep (stop live simulation)
		if (Players->Num() == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] Last player left system - putting to sleep mode"));

			// Get economy subsystem to stop active simulation
			UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
			if (EconomySubsystem)
			{
				// Clear active system if this was the active one
				if (UniverseData.ActiveSystemId == SystemId)
				{
					// Mark locations as inactive
					FStarSystemData& ActiveSystem = UniverseData.Systems[SystemId];
					for (FLocationData& Location : ActiveSystem.Locations)
					{
						Location.Market.bIsActiveSimulation = false;
					}

					// Clear active system ID (sleep mode)
					UniverseData.ActiveSystemId = -1;

					// Clear the economy tick timer (via GetWorld)
					if (UWorld* World = GetWorld())
					{
						// Note: Timer is owned by EconomySubsystem, so we can't directly clear it here
						// The EconomySubsystem will detect ActiveSystemId == -1 and stop ticking
						UE_LOG(LogTemp, Log, TEXT("[UniverseSubsystem] System put to sleep - ActiveSystemId cleared"));
					}
				}
			}

			// Clean up empty array from map
			PlayersInSystem.Remove(SystemId);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UniverseSubsystem] Player was not found in system %d tracking"), SystemId);
	}
}

