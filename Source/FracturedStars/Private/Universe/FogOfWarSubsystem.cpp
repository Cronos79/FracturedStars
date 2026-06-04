// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/FogOfWarSubsystem.h"
#include "Universe/UniverseSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UFogOfWarSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Note: UniverseSubsystem reference will be acquired lazily when first needed
	// This avoids dependency on initialization order

	UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Subsystem initialized (will determine authority when first accessed)"));
}

void UFogOfWarSubsystem::Deinitialize()
{
	// Clear timer
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(TickTimerHandle);
	}

	// Clear all fog-of-war state
	PlayerFogOfWarStates.Empty();

	Super::Deinitialize();
}

// ============================================================================
// SERVER: Player Registration & Management
// ============================================================================

void UFogOfWarSubsystem::RegisterPlayer(int32 PlayerId)
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar] RegisterPlayer called on CLIENT - ignoring"));
		return;
	}

	if (PlayerFogOfWarStates.Contains(PlayerId))
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar] Player %d already registered"), PlayerId);
		return;
	}

	FPlayerFogOfWarState NewState;
	NewState.PlayerId = PlayerId;
	PlayerFogOfWarStates.Add(PlayerId, NewState);

	UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Registered player %d"), PlayerId);
}

void UFogOfWarSubsystem::UnregisterPlayer(int32 PlayerId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar] UnregisterPlayer called on CLIENT - ignoring"));
		return;
	}

	if (PlayerFogOfWarStates.Remove(PlayerId) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Unregistered player %d"), PlayerId);
	}
}

// ============================================================================
// SERVER: Visibility Updates
// ============================================================================

void UFogOfWarSubsystem::OnPlayerFocusSystem(int32 PlayerId, FName SystemId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);
	if (!KnownState)
	{
		return;
	}

	// If system was Hidden, promote to Known
	if (KnownState->VisibilityLevel == ESystemVisibility::Hidden)
	{
		KnownState->VisibilityLevel = ESystemVisibility::Known;
		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d discovered system %s (Hidden -> Known)"), PlayerId, *SystemId.ToString());
	}

	// Add to active observation
	FPlayerFogOfWarState* PlayerState = GetOrCreatePlayerState(PlayerId);
	if (PlayerState)
	{
		PlayerState->ActiveObservationSystems.Add(SystemId);

		// Promote to ActiveObservation
		if (KnownState->VisibilityLevel != ESystemVisibility::ActiveObservation)
		{
			KnownState->VisibilityLevel = ESystemVisibility::ActiveObservation;
			UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d now actively observing system %s"), PlayerId, *SystemId.ToString());
		}

		// Take fresh snapshot of real state
		SnapshotRealState(PlayerId, SystemId);
	}
}

void UFogOfWarSubsystem::OnPlayerUnfocusSystem(int32 PlayerId, FName SystemId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	FPlayerFogOfWarState* PlayerState = GetOrCreatePlayerState(PlayerId);
	if (!PlayerState)
	{
		return;
	}

	// Remove from active observation
	PlayerState->ActiveObservationSystems.Remove(SystemId);

	// If no other observation sources (crew/ships/stations), downgrade visibility
	// For now, immediately downgrade since we haven't added crew/ship tracking yet
	DowngradeVisibility(PlayerId, SystemId);

	UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d unfocused system %s"), PlayerId, *SystemId.ToString());
}

void UFogOfWarSubsystem::DiscoverSystem(int32 PlayerId, FName SystemId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);
	if (!KnownState)
	{
		return;
	}

	// Only promote from Hidden to Known
	if (KnownState->VisibilityLevel == ESystemVisibility::Hidden)
	{
		KnownState->VisibilityLevel = ESystemVisibility::Known;
		KnownState->LastIntelligenceTime = UniverseSubsystem->GetGameTime();

		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d discovered system %s"), PlayerId, *SystemId.ToString());
	}
}

void UFogOfWarSubsystem::CompleteSurvey(int32 PlayerId, FName SystemId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);
	if (!KnownState)
	{
		return;
	}

	// Promote to Surveyed (or maintain higher level)
	if (KnownState->VisibilityLevel < ESystemVisibility::Surveyed)
	{
		KnownState->VisibilityLevel = ESystemVisibility::Surveyed;
	}

	KnownState->bSurveyCompleted = true;
	KnownState->LastObservationTime = UniverseSubsystem->GetGameTime();

	// Take snapshot of current state
	SnapshotRealState(PlayerId, SystemId);

	UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d completed survey of system %s"), PlayerId, *SystemId.ToString());
}

void UFogOfWarSubsystem::AddObservationSource(int32 PlayerId, FName SystemId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	FPlayerFogOfWarState* PlayerState = GetOrCreatePlayerState(PlayerId);
	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);

	if (!PlayerState || !KnownState)
	{
		return;
	}

	// Add to active observation
	PlayerState->ActiveObservationSystems.Add(SystemId);

	// Promote to ActiveObservation
	if (KnownState->VisibilityLevel != ESystemVisibility::ActiveObservation)
	{
		KnownState->VisibilityLevel = ESystemVisibility::ActiveObservation;
		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d added observation source in system %s (now active)"), PlayerId, *SystemId.ToString());
	}

	// Take fresh snapshot
	SnapshotRealState(PlayerId, SystemId);
}

void UFogOfWarSubsystem::RemoveObservationSource(int32 PlayerId, FName SystemId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	FPlayerFogOfWarState* PlayerState = GetOrCreatePlayerState(PlayerId);
	if (!PlayerState)
	{
		return;
	}

	// Remove from active observation
	PlayerState->ActiveObservationSystems.Remove(SystemId);

	// Downgrade if no observation sources remain
	DowngradeVisibility(PlayerId, SystemId);

	UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d removed observation source from system %s"), PlayerId, *SystemId.ToString());
}

// ============================================================================
// SERVER: Information Aging & Updates
// ============================================================================

void UFogOfWarSubsystem::UpdateKnownState(int32 PlayerId, FName SystemId, const FSystemKnownState& NewState)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);
	if (!KnownState)
	{
		return;
	}

	// Merge new information
	if (NewState.LastObservationTime > KnownState->LastObservationTime)
	{
		KnownState->LastObservationTime = NewState.LastObservationTime;
	}

	if (NewState.LastIntelligenceTime > KnownState->LastIntelligenceTime)
	{
		KnownState->LastIntelligenceTime = NewState.LastIntelligenceTime;
	}

	// Update known prices (newer information wins)
	for (const auto& PricePair : NewState.KnownPrices)
	{
		KnownState->KnownPrices.Add(PricePair.Key, PricePair.Value);
	}

	// Update known supply state
	for (const auto& SupplyPair : NewState.KnownSupplyState)
	{
		KnownState->KnownSupplyState.Add(SupplyPair.Key, SupplyPair.Value);
	}

	// Update population if newer
	if (NewState.LastKnownPopulation > 0)
	{
		KnownState->LastKnownPopulation = NewState.LastKnownPopulation;
	}

	UE_LOG(LogTemp, Verbose, TEXT("[FogOfWar] Updated known state for player %d, system %s"), PlayerId, *SystemId.ToString());
}

void UFogOfWarSubsystem::TickFogOfWar(float DeltaTime)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	// For now, this is a placeholder for future information-aging logic
	// Future: check observation times, downgrade stale data, expire old intel, etc.

	// Example future logic:
	// - If LastObservationTime is > 30 game-days old and no active observation, mark prices as "stale"
	// - If LastIntelligenceTime is > 90 game-days old, expire old intel reports
	// - etc.
}

// ============================================================================
// QUERIES: Visibility & Known State
// ============================================================================

ESystemVisibility UFogOfWarSubsystem::GetSystemVisibility(int32 PlayerId, FName SystemId) const
{
	const FPlayerFogOfWarState* PlayerState = GetPlayerState(PlayerId);
	if (!PlayerState)
	{
		return ESystemVisibility::Hidden;
	}

	const FSystemKnownState* KnownState = PlayerState->KnownSystems.Find(SystemId);
	if (!KnownState)
	{
		return ESystemVisibility::Hidden;
	}

	return KnownState->VisibilityLevel;
}

bool UFogOfWarSubsystem::HasActiveObservation(int32 PlayerId, FName SystemId) const
{
	const FPlayerFogOfWarState* PlayerState = GetPlayerState(PlayerId);
	if (!PlayerState)
	{
		return false;
	}

	return PlayerState->ActiveObservationSystems.Contains(SystemId);
}

bool UFogOfWarSubsystem::GetKnownState(int32 PlayerId, FName SystemId, FSystemKnownState& OutState) const
{
	const FPlayerFogOfWarState* PlayerState = GetPlayerState(PlayerId);
	if (!PlayerState)
	{
		return false;
	}

	const FSystemKnownState* KnownState = PlayerState->KnownSystems.Find(SystemId);
	if (!KnownState)
	{
		return false;
	}

	OutState = *KnownState;
	return true;
}

TArray<FName> UFogOfWarSubsystem::GetSystemsWithVisibility(int32 PlayerId, ESystemVisibility MinLevel) const
{
	TArray<FName> Result;

	const FPlayerFogOfWarState* PlayerState = GetPlayerState(PlayerId);
	if (!PlayerState)
	{
		return Result;
	}

	for (const auto& Pair : PlayerState->KnownSystems)
	{
		if (Pair.Value.VisibilityLevel >= MinLevel)
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

// ============================================================================
// UTILITY
// ============================================================================

float UFogOfWarSubsystem::GetInformationAge(const FUniverseTime& LastUpdate, const FUniverseTime& CurrentTime) const
{
	// Calculate difference in game-days
	int32 DaysDiff = (CurrentTime.Year - LastUpdate.Year) * 365 + (CurrentTime.Day - LastUpdate.Day);
	float HoursDiff = static_cast<float>(DaysDiff * 24) + (CurrentTime.Hour - LastUpdate.Hour);

	return HoursDiff / 24.0f;  // Return age in game-days
}

// ============================================================================
// PRIVATE HELPERS
// ============================================================================

FPlayerFogOfWarState* UFogOfWarSubsystem::GetOrCreatePlayerState(int32 PlayerId)
{
	if (!PlayerFogOfWarStates.Contains(PlayerId))
	{
		FPlayerFogOfWarState NewState;
		NewState.PlayerId = PlayerId;
		PlayerFogOfWarStates.Add(PlayerId, NewState);
	}

	return PlayerFogOfWarStates.Find(PlayerId);
}

const FPlayerFogOfWarState* UFogOfWarSubsystem::GetPlayerState(int32 PlayerId) const
{
	return PlayerFogOfWarStates.Find(PlayerId);
}

FSystemKnownState* UFogOfWarSubsystem::GetOrCreateKnownState(int32 PlayerId, FName SystemId)
{
	FPlayerFogOfWarState* PlayerState = GetOrCreatePlayerState(PlayerId);
	if (!PlayerState)
	{
		return nullptr;
	}

	if (!PlayerState->KnownSystems.Contains(SystemId))
	{
		FSystemKnownState NewKnown;
		NewKnown.VisibilityLevel = ESystemVisibility::Hidden;
		PlayerState->KnownSystems.Add(SystemId, NewKnown);
	}

	return PlayerState->KnownSystems.Find(SystemId);
}

void UFogOfWarSubsystem::DowngradeVisibility(int32 PlayerId, FName SystemId)
{
	FPlayerFogOfWarState* PlayerState = GetOrCreatePlayerState(PlayerId);
	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);

	if (!PlayerState || !KnownState)
	{
		return;
	}

	// If still in active observation set, don't downgrade
	if (PlayerState->ActiveObservationSystems.Contains(SystemId))
	{
		return;
	}

	// Downgrade from ActiveObservation to Surveyed (or Known if not surveyed)
	if (KnownState->VisibilityLevel == ESystemVisibility::ActiveObservation)
	{
		if (KnownState->bSurveyCompleted)
		{
			KnownState->VisibilityLevel = ESystemVisibility::Surveyed;
			UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d: system %s downgraded to Surveyed (no active observation)"), PlayerId, *SystemId.ToString());
		}
		else
		{
			KnownState->VisibilityLevel = ESystemVisibility::Known;
			UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d: system %s downgraded to Known (no active observation)"), PlayerId, *SystemId.ToString());
		}
	}
}

void UFogOfWarSubsystem::SnapshotRealState(int32 PlayerId, FName SystemId)
{
	if (!UniverseSubsystem)
	{
		return;
	}

	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);
	if (!KnownState)
	{
		return;
	}

	// Update observation time
	KnownState->LastObservationTime = UniverseSubsystem->GetGameTime();

	// Get the actual system data
	const FUniverseData& UniverseData = UniverseSubsystem->GetUniverseData();

	// Find system in universe data
	int32 SystemIdInt = FCString::Atoi(*SystemId.ToString());
	const FStarSystemData* SystemData = UniverseData.Systems.FindByPredicate(
		[SystemIdInt](const FStarSystemData& System) { return System.SystemId == SystemIdInt; }
	);

	if (!SystemData)
	{
		return;
	}

	// Snapshot faction control
	KnownState->LastKnownFaction = FName(*FString::FromInt(SystemData->ControllingFactionId));

	// Snapshot market data for all locations in system
	KnownState->KnownPrices.Empty();
	KnownState->KnownSupplyState.Empty();
	KnownState->LastKnownPopulation = 0;

	for (const FLocationData& Location : SystemData->Locations)
	{
		// Accumulate population
		KnownState->LastKnownPopulation += Location.Population;

		// Snapshot market prices
		for (const FMarketGoodEntry& Good : Location.Market.Goods)
		{
			// Store price (use location-average if multiple locations trade the same good)
			if (!KnownState->KnownPrices.Contains(Good.GoodType))
			{
				KnownState->KnownPrices.Add(Good.GoodType, Good.CurrentPrice);
			}
			else
			{
				// Average prices across locations
				float ExistingPrice = KnownState->KnownPrices[Good.GoodType];
				KnownState->KnownPrices[Good.GoodType] = (ExistingPrice + Good.CurrentPrice) / 2.0f;
			}

			// Store supply state (shortage/surplus)
			float SupplyRatio = 0.0f;
			if (Good.TargetStock > 0)
			{
				SupplyRatio = (static_cast<float>(Good.Stock) / Good.TargetStock) - 1.0f;
				// Negative = shortage, Positive = surplus, Zero = balanced
			}

			if (Good.bIsShortage)
			{
				SupplyRatio = FMath::Min(SupplyRatio, -0.5f);  // Mark as significant shortage
			}
			else if (Good.bIsSurplus)
			{
				SupplyRatio = FMath::Max(SupplyRatio, 0.5f);   // Mark as significant surplus
			}

			KnownState->KnownSupplyState.Add(Good.GoodType, SupplyRatio);
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("[FogOfWar] Snapshotted real state for player %d, system %s - %d goods, pop %d"), 
		PlayerId, *SystemId.ToString(), KnownState->KnownPrices.Num(), KnownState->LastKnownPopulation);
}

// ============================================================================
// VISIBILITY-FILTERED ECONOMY QUERIES
// ============================================================================

float UFogOfWarSubsystem::GetVisiblePrice(int32 PlayerId, FName SystemId, int32 LocationId, EGoodType GoodType) const
{
	if (!UniverseSubsystem)
	{
		return 0.0f;
	}

	// Check visibility level
	ESystemVisibility Visibility = GetSystemVisibility(PlayerId, SystemId);

	// If ActiveObservation, return real-time price
	if (Visibility == ESystemVisibility::ActiveObservation)
	{
		int32 SystemIdInt = FCString::Atoi(*SystemId.ToString());
		return UniverseSubsystem->GetGoodPrice(SystemIdInt, LocationId, GoodType);
	}

	// If Surveyed or Known, return last known price (may be stale)
	if (Visibility == ESystemVisibility::Surveyed || Visibility == ESystemVisibility::Known)
	{
		FSystemKnownState OutState;
		if (GetKnownState(PlayerId, SystemId, OutState))
		{
			const float* Price = OutState.KnownPrices.Find(GoodType);
			return Price ? *Price : 0.0f;
		}
	}

	// Hidden - no information
	return 0.0f;
}

bool UFogOfWarSubsystem::GetVisibleShortage(int32 PlayerId, FName SystemId, int32 LocationId, EGoodType GoodType) const
{
	if (!UniverseSubsystem)
	{
		return false;
	}

	// Check visibility level
	ESystemVisibility Visibility = GetSystemVisibility(PlayerId, SystemId);

	// If ActiveObservation, return real-time shortage state
	if (Visibility == ESystemVisibility::ActiveObservation)
	{
		int32 SystemIdInt = FCString::Atoi(*SystemId.ToString());
		return UniverseSubsystem->HasShortage(SystemIdInt, LocationId, GoodType);
	}

	// If Surveyed, return last known shortage state (may be stale)
	if (Visibility == ESystemVisibility::Surveyed)
	{
		FSystemKnownState OutState;
		if (GetKnownState(PlayerId, SystemId, OutState))
		{
			const float* SupplyRatio = OutState.KnownSupplyState.Find(GoodType);
			if (SupplyRatio)
			{
				return *SupplyRatio < -0.3f;  // Shortage threshold
			}
		}
	}

	// Known or Hidden - no detailed shortage information
	return false;
}

TMap<EGoodType, float> UFogOfWarSubsystem::GetVisibleMarketPrices(int32 PlayerId, FName SystemId, int32 LocationId) const
{
	TMap<EGoodType, float> Result;

	if (!UniverseSubsystem)
	{
		return Result;
	}

	// Check visibility level
	ESystemVisibility Visibility = GetSystemVisibility(PlayerId, SystemId);

	// If ActiveObservation, return real-time prices
	if (Visibility == ESystemVisibility::ActiveObservation)
	{
		int32 SystemIdInt = FCString::Atoi(*SystemId.ToString());
		FMarketState Market = UniverseSubsystem->GetMarketState(SystemIdInt, LocationId);

		for (const FMarketGoodEntry& Good : Market.Goods)
		{
			Result.Add(Good.GoodType, Good.CurrentPrice);
		}

		return Result;
	}

	// If Surveyed or Known, return last known prices (may be stale)
	if (Visibility == ESystemVisibility::Surveyed || Visibility == ESystemVisibility::Known)
	{
		FSystemKnownState OutState;
		if (GetKnownState(PlayerId, SystemId, OutState))
		{
			Result = OutState.KnownPrices;
		}
	}

	// Hidden - no information
	return Result;
}

// ============================================================================
// FUTURE: Crew & Asset Visibility Extension
// ============================================================================

void UFogOfWarSubsystem::RegisterCrewMember(int32 PlayerId, FName SystemId, int32 CrewId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	// Get or create crew sources map for this player
	TMap<FName, TSet<int32>>& PlayerCrewMap = PlayerCrewSources.FindOrAdd(PlayerId);
	TSet<int32>& CrewSet = PlayerCrewMap.FindOrAdd(SystemId);

	if (!CrewSet.Contains(CrewId))
	{
		CrewSet.Add(CrewId);

		// Add observation source for this system
		AddObservationSource(PlayerId, SystemId);

		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d: crew %d registered in system %s (extended visibility)"), 
			PlayerId, CrewId, *SystemId.ToString());
	}
}

void UFogOfWarSubsystem::UnregisterCrewMember(int32 PlayerId, FName SystemId, int32 CrewId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	TMap<FName, TSet<int32>>* PlayerCrewMap = PlayerCrewSources.Find(PlayerId);
	if (!PlayerCrewMap)
	{
		return;
	}

	TSet<int32>* CrewSet = PlayerCrewMap->Find(SystemId);
	if (!CrewSet)
	{
		return;
	}

	if (CrewSet->Remove(CrewId) > 0)
	{
		// Check if any observation sources remain
		const TMap<FName, TSet<int32>>* ShipMap = PlayerShipSources.Find(PlayerId);
		const TMap<FName, TSet<int32>>* StationMap = PlayerStationSources.Find(PlayerId);

		bool bHasOtherSources = 
			(CrewSet->Num() > 0) ||
			(ShipMap && ShipMap->Contains(SystemId) && (*ShipMap)[SystemId].Num() > 0) ||
			(StationMap && StationMap->Contains(SystemId) && (*StationMap)[SystemId].Num() > 0);

		if (!bHasOtherSources)
		{
			// No more observation sources - remove observation capability
			RemoveObservationSource(PlayerId, SystemId);
		}

		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d: crew %d unregistered from system %s"), 
			PlayerId, CrewId, *SystemId.ToString());
	}
}

void UFogOfWarSubsystem::RegisterShip(int32 PlayerId, FName SystemId, int32 ShipId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	TMap<FName, TSet<int32>>& PlayerShipMap = PlayerShipSources.FindOrAdd(PlayerId);
	TSet<int32>& ShipSet = PlayerShipMap.FindOrAdd(SystemId);

	if (!ShipSet.Contains(ShipId))
	{
		ShipSet.Add(ShipId);
		AddObservationSource(PlayerId, SystemId);

		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d: ship %d registered in system %s (extended visibility)"), 
			PlayerId, ShipId, *SystemId.ToString());
	}
}

void UFogOfWarSubsystem::UnregisterShip(int32 PlayerId, FName SystemId, int32 ShipId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	TMap<FName, TSet<int32>>* PlayerShipMap = PlayerShipSources.Find(PlayerId);
	if (!PlayerShipMap)
	{
		return;
	}

	TSet<int32>* ShipSet = PlayerShipMap->Find(SystemId);
	if (!ShipSet)
	{
		return;
	}

	if (ShipSet->Remove(ShipId) > 0)
	{
		// Check if any observation sources remain
		const TMap<FName, TSet<int32>>* CrewMap = PlayerCrewSources.Find(PlayerId);
		const TMap<FName, TSet<int32>>* StationMap = PlayerStationSources.Find(PlayerId);

		bool bHasOtherSources = 
			(ShipSet->Num() > 0) ||
			(CrewMap && CrewMap->Contains(SystemId) && (*CrewMap)[SystemId].Num() > 0) ||
			(StationMap && StationMap->Contains(SystemId) && (*StationMap)[SystemId].Num() > 0);

		if (!bHasOtherSources)
		{
			RemoveObservationSource(PlayerId, SystemId);
		}

		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d: ship %d unregistered from system %s"), 
			PlayerId, ShipId, *SystemId.ToString());
	}
}

void UFogOfWarSubsystem::RegisterStation(int32 PlayerId, FName SystemId, int32 StationId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	TMap<FName, TSet<int32>>& PlayerStationMap = PlayerStationSources.FindOrAdd(PlayerId);
	TSet<int32>& StationSet = PlayerStationMap.FindOrAdd(SystemId);

	if (!StationSet.Contains(StationId))
	{
		StationSet.Add(StationId);
		AddObservationSource(PlayerId, SystemId);

		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d: station %d registered in system %s (permanent visibility)"), 
			PlayerId, StationId, *SystemId.ToString());
	}
}

void UFogOfWarSubsystem::UnregisterStation(int32 PlayerId, FName SystemId, int32 StationId)
{
	if (!UniverseSubsystem || !UniverseSubsystem->IsAuthority())
	{
		return;  // SERVER ONLY
	}

	TMap<FName, TSet<int32>>* PlayerStationMap = PlayerStationSources.Find(PlayerId);
	if (!PlayerStationMap)
	{
		return;
	}

	TSet<int32>* StationSet = PlayerStationMap->Find(SystemId);
	if (!StationSet)
	{
		return;
	}

	if (StationSet->Remove(StationId) > 0)
	{
		// Check if any observation sources remain
		const TMap<FName, TSet<int32>>* CrewMap = PlayerCrewSources.Find(PlayerId);
		const TMap<FName, TSet<int32>>* ShipMap = PlayerShipSources.Find(PlayerId);

		bool bHasOtherSources = 
			(StationSet->Num() > 0) ||
			(CrewMap && CrewMap->Contains(SystemId) && (*CrewMap)[SystemId].Num() > 0) ||
			(ShipMap && ShipMap->Contains(SystemId) && (*ShipMap)[SystemId].Num() > 0);

		if (!bHasOtherSources)
		{
			RemoveObservationSource(PlayerId, SystemId);
		}

		UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Player %d: station %d unregistered from system %s"), 
			PlayerId, StationId, *SystemId.ToString());
	}
}

// ============================================================================
// DEBUG & TESTING
// ============================================================================

void UFogOfWarSubsystem::DebugPrintPlayerFogState(int32 PlayerId) const
{
	const FPlayerFogOfWarState* PlayerState = GetPlayerState(PlayerId);
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] Player %d has no fog-of-war state"), PlayerId);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========== FOG OF WAR STATE: Player %d =========="), PlayerId);
	UE_LOG(LogTemp, Log, TEXT("Total Known Systems: %d"), PlayerState->KnownSystems.Num());
	UE_LOG(LogTemp, Log, TEXT("Active Observation Systems: %d"), PlayerState->ActiveObservationSystems.Num());

	// Count systems by visibility level
	int32 HiddenCount = 0;
	int32 KnownCount = 0;
	int32 SurveyedCount = 0;
	int32 ActiveCount = 0;

	for (const auto& Pair : PlayerState->KnownSystems)
	{
		switch (Pair.Value.VisibilityLevel)
		{
			case ESystemVisibility::Hidden: HiddenCount++; break;
			case ESystemVisibility::Known: KnownCount++; break;
			case ESystemVisibility::Surveyed: SurveyedCount++; break;
			case ESystemVisibility::ActiveObservation: ActiveCount++; break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("  Hidden: %d, Known: %d, Surveyed: %d, Active: %d"), 
		HiddenCount, KnownCount, SurveyedCount, ActiveCount);

	// Print first 10 known systems as examples
	UE_LOG(LogTemp, Log, TEXT("Known Systems (first 10):"));
	int32 Count = 0;
	for (const auto& Pair : PlayerState->KnownSystems)
	{
		if (Count++ >= 10) break;

		FString VisibilityStr;
		switch (Pair.Value.VisibilityLevel)
		{
			case ESystemVisibility::Hidden: VisibilityStr = TEXT("Hidden"); break;
			case ESystemVisibility::Known: VisibilityStr = TEXT("Known"); break;
			case ESystemVisibility::Surveyed: VisibilityStr = TEXT("Surveyed"); break;
			case ESystemVisibility::ActiveObservation: VisibilityStr = TEXT("Active"); break;
		}

		UE_LOG(LogTemp, Log, TEXT("  %s: %s (Pop: %d, Prices: %d)"), 
			*Pair.Key.ToString(), *VisibilityStr, 
			Pair.Value.LastKnownPopulation, Pair.Value.KnownPrices.Num());
	}

	UE_LOG(LogTemp, Log, TEXT("===================================================="));
}

void UFogOfWarSubsystem::DebugSetVisibility(int32 PlayerId, FName SystemId, ESystemVisibility NewVisibility)
{
	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);
	if (!KnownState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] Failed to get/create known state for player %d, system %s"), 
			PlayerId, *SystemId.ToString());
		return;
	}

	ESystemVisibility OldVisibility = KnownState->VisibilityLevel;
	KnownState->VisibilityLevel = NewVisibility;

	// If setting to ActiveObservation, take a fresh snapshot
	if (NewVisibility == ESystemVisibility::ActiveObservation && UniverseSubsystem)
	{
		SnapshotRealState(PlayerId, SystemId);
	}

	// If setting to Surveyed, mark survey complete
	if (NewVisibility >= ESystemVisibility::Surveyed)
	{
		KnownState->bSurveyCompleted = true;
	}

	UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] Player %d: System %s visibility changed %d -> %d"), 
		PlayerId, *SystemId.ToString(), (int32)OldVisibility, (int32)NewVisibility);
}

void UFogOfWarSubsystem::DebugSimulateInformationAge(int32 PlayerId, FName SystemId, float GameDaysAgo)
{
	FSystemKnownState* KnownState = GetOrCreateKnownState(PlayerId, SystemId);
	if (!KnownState || !UniverseSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] Failed to age information for player %d, system %s"), 
			PlayerId, *SystemId.ToString());
		return;
	}

	// Get current time and subtract the specified days
	FUniverseTime CurrentTime = UniverseSubsystem->GetGameTime();
	double SecondsToSubtract = GameDaysAgo * 24.0 * 3600.0; // Days to seconds

	// Create aged timestamp
	FUniverseTime AgedTime = CurrentTime;
	AgedTime.TotalElapsedSeconds -= SecondsToSubtract;

	// Recalculate date components (simplified - just for testing)
	int32 DaysToSubtract = static_cast<int32>(GameDaysAgo);
	AgedTime.Day -= DaysToSubtract;
	while (AgedTime.Day < 1)
	{
		AgedTime.Month--;
		if (AgedTime.Month < 1)
		{
			AgedTime.Month = 12;
			AgedTime.Year--;
		}
		AgedTime.Day += 30; // Simplified month length
	}

	KnownState->LastObservationTime = AgedTime;
	KnownState->LastIntelligenceTime = AgedTime;

	UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] Player %d: System %s information aged by %.1f days"), 
		PlayerId, *SystemId.ToString(), GameDaysAgo);
}

void UFogOfWarSubsystem::DebugCompareRealVsKnown(int32 PlayerId, FName SystemId) const
{
	UUniverseSubsystem* Universe = EnsureUniverseSubsystem();
	if (!Universe)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] UniverseSubsystem not available"));
		return;
	}

	const FPlayerFogOfWarState* PlayerState = GetPlayerState(PlayerId);
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] Player %d has no fog-of-war state"), PlayerId);
		return;
	}

	const FSystemKnownState* KnownState = PlayerState->KnownSystems.Find(SystemId);
	if (!KnownState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] Player %d has no knowledge of system %s"), 
			PlayerId, *SystemId.ToString());
		return;
	}

	// Get real system data
	const FUniverseData& UniverseData = Universe->GetUniverseData();
	int32 SystemIdInt = FCString::Atoi(*SystemId.ToString());
	const FStarSystemData* SystemData = UniverseData.Systems.FindByPredicate(
		[SystemIdInt](const FStarSystemData& System) { return System.SystemId == SystemIdInt; }
	);

	if (!SystemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FogOfWar DEBUG] System %s not found in universe data"), *SystemId.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========== REAL vs KNOWN: System %s (Player %d) =========="), 
		*SystemId.ToString(), PlayerId);

	// Calculate real population
	int32 RealPopulation = 0;
	for (const FLocationData& Location : SystemData->Locations)
	{
		RealPopulation += Location.Population;
	}

	UE_LOG(LogTemp, Log, TEXT("Population - Real: %d, Known: %d, Diff: %d"), 
		RealPopulation, KnownState->LastKnownPopulation, RealPopulation - KnownState->LastKnownPopulation);

	UE_LOG(LogTemp, Log, TEXT("Faction - Real: %d, Known: %s"), 
		SystemData->ControllingFactionId, *KnownState->LastKnownFaction.ToString());

	// Compare prices for first location
	if (SystemData->Locations.Num() > 0 && KnownState->KnownPrices.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Price Comparison (first location):"));
		const FLocationData& FirstLocation = SystemData->Locations[0];

		for (const FMarketGoodEntry& Good : FirstLocation.Market.Goods)
		{
			const float* KnownPrice = KnownState->KnownPrices.Find(Good.GoodType);
			if (KnownPrice)
			{
				float PriceDiff = Good.CurrentPrice - *KnownPrice;
				float PriceDiffPercent = (*KnownPrice > 0) ? (PriceDiff / *KnownPrice * 100.0f) : 0.0f;

				UE_LOG(LogTemp, Log, TEXT("  %s - Real: %.2f, Known: %.2f, Diff: %.2f (%.1f%%)"), 
					*UEnum::GetValueAsString(Good.GoodType), 
					Good.CurrentPrice, *KnownPrice, PriceDiff, PriceDiffPercent);
			}
		}
	}

	// Information age
	float Age = GetInformationAge(KnownState->LastObservationTime, Universe->GetGameTime());
	UE_LOG(LogTemp, Log, TEXT("Information Age: %.2f days"), Age);

	UE_LOG(LogTemp, Log, TEXT("=========================================================="));
}

FString UFogOfWarSubsystem::DebugGetVisibilitySummary(int32 PlayerId) const
{
	const FPlayerFogOfWarState* PlayerState = GetPlayerState(PlayerId);
	if (!PlayerState)
	{
		return FString::Printf(TEXT("Player %d: No fog-of-war state"), PlayerId);
	}

	int32 HiddenCount = 0;
	int32 KnownCount = 0;
	int32 SurveyedCount = 0;
	int32 ActiveCount = 0;

	for (const auto& Pair : PlayerState->KnownSystems)
	{
		switch (Pair.Value.VisibilityLevel)
		{
			case ESystemVisibility::Hidden: HiddenCount++; break;
			case ESystemVisibility::Known: KnownCount++; break;
			case ESystemVisibility::Surveyed: SurveyedCount++; break;
			case ESystemVisibility::ActiveObservation: ActiveCount++; break;
		}
	}

	return FString::Printf(TEXT("Hidden: %d, Known: %d, Surveyed: %d, Active: %d"), 
		HiddenCount, KnownCount, SurveyedCount, ActiveCount);
}

// ============================================================================
// HELPER: Ensure UniverseSubsystem Reference
// ============================================================================

UUniverseSubsystem* UFogOfWarSubsystem::EnsureUniverseSubsystem() const
{
	// Lazy initialization - acquire reference on first use
	if (!UniverseSubsystem)
	{
		// Cast away const for lazy initialization (common pattern for cached subsystem refs)
		UFogOfWarSubsystem* MutableThis = const_cast<UFogOfWarSubsystem*>(this);
		MutableThis->UniverseSubsystem = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();

		if (!MutableThis->UniverseSubsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("[FogOfWar] Failed to acquire UniverseSubsystem reference"));
			return nullptr;
		}

		// SERVER ONLY: Start periodic tick for information aging if we're on server
		if (MutableThis->UniverseSubsystem->IsAuthority())
		{
			UWorld* World = GetWorld();
			if (World && !MutableThis->TickTimerHandle.IsValid())
			{
				World->GetTimerManager().SetTimer(
					MutableThis->TickTimerHandle,
					FTimerDelegate::CreateUObject(MutableThis, &UFogOfWarSubsystem::TickFogOfWar, MutableThis->TickInterval),
					MutableThis->TickInterval,
					true  // Loop
				);

				UE_LOG(LogTemp, Log, TEXT("[FogOfWar] Started SERVER tick timer (authority confirmed)"));
			}
		}
	}

	return UniverseSubsystem;
}

