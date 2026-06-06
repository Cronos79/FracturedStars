// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Universe/FactionSubsystem.h"
#include "Universe/UniverseSubsystem.h"
#include "Universe/EconomySubsystem.h"
#include "Universe/LogisticsSubsystem.h"

// ============================================================================
// SUBSYSTEM LIFECYCLE
// ============================================================================

void UFactionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("FactionSubsystem: Initialized (waiting for universe generation)"));
}

void UFactionSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FactionTickTimer);
	}

	Factions.Empty();
	bIsInitialized = false;

	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("FactionSubsystem: Deinitialized"));
}

void UFactionSubsystem::InitializeFactions(const FUniverseData& UniverseData)
{
	UE_LOG(LogTemp, Log, TEXT("FactionSubsystem: Initializing faction simulation..."));

	// Copy faction data from universe (local cache for frequent access)
	Factions = UniverseData.Factions;

	if (Factions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FactionSubsystem: No factions found in universe data!"));
		return;
	}

	bIsInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("FactionSubsystem: Loaded %d factions"), Factions.Num());

	// Set up periodic faction analysis tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FactionTickTimer,
			[this]()
			{
				// Sprint 7: Run faction analysis on timer
				TickFactions(FactionTickRate);
			},
			FactionTickRate,
			true // Loop
		);

		UE_LOG(LogTemp, Log, TEXT("FactionSubsystem: Tick timer started (%.1fs interval)"), FactionTickRate);
	}
}

void UFactionSubsystem::TickFactions(float DeltaTime)
{
	if (!bIsInitialized || Factions.Num() == 0)
		return;

	// Sprint 7: Run analysis for all factions
	// Consumer-only pattern: query existing systems, update faction awareness

	for (int32 FactionId = 0; FactionId < Factions.Num(); ++FactionId)
	{
		// Gather economic data (shortages & surpluses)
		GatherFactionShortages(FactionId);
		GatherFactionSurpluses(FactionId);

		// Calculate stress level
		CalculateFactionEconomicStress(FactionId);

		// Analyze dependencies on other factions
		AnalyzeFactionDependencies(FactionId);

		// Check ship production capability
		EvaluateShipProductionCapability(FactionId);

		// Trigger events for significant conditions (future hook integration)
		TriggerFactionEvents(FactionId);
	}

	UE_LOG(LogTemp, Verbose, TEXT("FactionSubsystem: Faction analysis tick completed for %d factions"), Factions.Num());
}

// ============================================================================
// FACTION DATA ACCESS
// ============================================================================

bool UFactionSubsystem::GetFactionById(int32 FactionId, FFactionData& OutFaction) const
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("FactionSubsystem: Not initialized"));
		return false;
	}

	if (FactionId < 0 || FactionId >= Factions.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("FactionSubsystem: Invalid faction ID %d"), FactionId);
		return false;
	}

	OutFaction = Factions[FactionId];
	return true;
}

int32 UFactionSubsystem::GetSystemControllingFaction(int32 SystemId) const
{
	if (!bIsInitialized)
		return -1;

	// Query universe subsystem for system data
	if (UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>())
	{
		FStarSystemData System;
		if (UniverseSys->GetSystemById(SystemId, System))
		{
			return System.ControllingFactionId;
		}
	}

	return -1;
}

// ============================================================================
// ECONOMIC AWARENESS (Sprint 7: Consumer of existing systems)
// ============================================================================

void UFactionSubsystem::GatherFactionShortages(int32 FactionId)
{
	if (FactionId < 0 || FactionId >= Factions.Num())
		return;

	FFactionData& Faction = Factions[FactionId];

	// Query economy subsystem for shortages (CONSUMER PATTERN)
	UEconomySubsystem* EconomySys = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();

	if (!EconomySys || !UniverseSys || !UniverseSys->IsUniverseGenerated())
		return;

	const FUniverseData& Universe = UniverseSys->GetUniverseData();

	// Aggregate shortages by good type for this faction's territory
	TMap<EGoodType, float> GoodTypeSeverity;
	TMap<EGoodType, int32> GoodTypeCount;

	// Scan all good types for shortages in faction territory
	TArray<EGoodType> AllGoodTypes = {
		EGoodType::Food, EGoodType::Water, EGoodType::Medicine, EGoodType::Fuel,
		EGoodType::Ore, EGoodType::RefinedMetals, EGoodType::Machinery, EGoodType::Electronics,
		EGoodType::Weapons, EGoodType::ConsumerGoods, EGoodType::IndustrialParts,
		EGoodType::AdvancedComponents, EGoodType::ResearchMaterials, EGoodType::Contraband
	};

	for (EGoodType GoodType : AllGoodTypes)
	{
		// Query economy for shortages of this good type (consumer pattern)
		TArray<FShortageLocation> Shortages = EconomySys->FindShortageLocations(Universe, GoodType);

		// Filter for faction-controlled territory
		for (const FShortageLocation& Shortage : Shortages)
		{
			if (Shortage.SystemId >= 0 && Shortage.SystemId < Universe.Systems.Num())
			{
				const FStarSystemData& System = Universe.Systems[Shortage.SystemId];
				if (System.ControllingFactionId == FactionId)
				{
					// Accumulate severity for this good type
					float& TotalSeverity = GoodTypeSeverity.FindOrAdd(GoodType, 0.0f);
					TotalSeverity += Shortage.Severity;

					int32& Count = GoodTypeCount.FindOrAdd(GoodType, 0);
					Count++;
				}
			}
		}
	}

	// Update faction's strategic priorities
	Faction.StrategicPriorities.Empty();
	for (const auto& Pair : GoodTypeSeverity)
	{
		EGoodType GoodType = Pair.Key;
		float TotalSeverity = Pair.Value;
		int32 Count = GoodTypeCount[GoodType];

		// Calculate average severity and priority weight
		float AvgSeverity = TotalSeverity / FMath::Max(1, Count);

		// Priority = severity * count (more widespread shortages = higher priority)
		float Priority = AvgSeverity * FMath::Sqrt((float)Count);

		Faction.StrategicPriorities.Add(FStrategicPriority(GoodType, Priority, AvgSeverity));
	}

	// Sort priorities by priority weight (highest first)
	Faction.StrategicPriorities.Sort([](const FStrategicPriority& A, const FStrategicPriority& B)
	{
		return A.Priority > B.Priority;
	});
}

void UFactionSubsystem::GatherFactionSurpluses(int32 FactionId)
{
	if (FactionId < 0 || FactionId >= Factions.Num())
		return;

	FFactionData& Faction = Factions[FactionId];

	// Query universe for faction territory
	UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	if (!UniverseSys || !UniverseSys->IsUniverseGenerated())
		return;

	const FUniverseData& Universe = UniverseSys->GetUniverseData();

	// Clear current export tracking
	Faction.CurrentExports.Empty();

	// Scan all locations in faction-controlled systems
	for (int32 SystemId : Faction.ControlledSystemIds)
	{
		if (SystemId < 0 || SystemId >= Universe.Systems.Num())
			continue;

		const FStarSystemData& System = Universe.Systems[SystemId];

		for (const FLocationData& Location : System.Locations)
		{
			// Only count locations owned by this faction
			if (Location.OwningFactionId == FactionId)
			{
				// Check market for surplus goods
				for (const FMarketGoodEntry& Entry : Location.Market.Goods)
				{
					if (Entry.bIsSurplus && Entry.Stock > 0)
					{
						// Track total surplus production across faction
						int32& ExportVolume = Faction.CurrentExports.FindOrAdd(Entry.GoodType, 0);
						ExportVolume += Entry.Stock;
					}
				}
			}
		}
	}
}

void UFactionSubsystem::CalculateFactionEconomicStress(int32 FactionId)
{
	if (FactionId < 0 || FactionId >= Factions.Num())
		return;

	FFactionData& Faction = Factions[FactionId];

	// Economic stress = aggregate of shortage priorities across territory
	// Higher priorities (critical shortages) contribute more to stress

	float TotalStress = 0.0f;
	float MaxPossibleStress = 0.0f;

	for (const FStrategicPriority& Priority : Faction.StrategicPriorities)
	{
		// Essential goods (food, water, medicine, fuel) weighted higher
		float Weight = 1.0f;
		if (Priority.GoodType == EGoodType::Food ||
			Priority.GoodType == EGoodType::Water ||
			Priority.GoodType == EGoodType::Medicine ||
			Priority.GoodType == EGoodType::Fuel)
		{
			Weight = 2.0f; // Critical goods have double weight
		}

		TotalStress += Priority.Priority * Weight;
		MaxPossibleStress += 10.0f * Weight; // Assume max priority of 10 per good
	}

	// Normalize stress to 0.0-1.0 range
	if (MaxPossibleStress > 0.0f)
	{
		Faction.CurrentEconomicStress = FMath::Clamp(TotalStress / MaxPossibleStress, 0.0f, 1.0f);
	}
	else
	{
		// No shortages = no stress
		Faction.CurrentEconomicStress = 0.0f;
	}

	// Log significant stress changes
	if (Faction.CurrentEconomicStress > 0.6f)
	{
		UE_LOG(LogTemp, Warning, TEXT("FactionSubsystem: %s economic stress: %.2f [HIGH]"),
			*Faction.FactionName, Faction.CurrentEconomicStress);
	}
}

// ============================================================================
// DEPENDENCY TRACKING (Sprint 7)
// ============================================================================

void UFactionSubsystem::AnalyzeFactionDependencies(int32 FactionId)
{
	if (FactionId < 0 || FactionId >= Factions.Num())
		return;

	FFactionData& Faction = Factions[FactionId];

	// Query universe for all systems
	UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	if (!UniverseSys || !UniverseSys->IsUniverseGenerated())
		return;

	const FUniverseData& Universe = UniverseSys->GetUniverseData();

	// Clear existing dependencies
	Faction.CriticalDependencies.Empty();

	// For each good type in faction's strategic priorities (shortages)
	for (const FStrategicPriority& Priority : Faction.StrategicPriorities)
	{
		EGoodType NeededGood = Priority.GoodType;

		// Track which factions produce this good (have surpluses)
		TMap<int32, int32> ProducerFactionVolumes;

		// Scan all other factions for surplus production
		for (int32 OtherFactionId = 0; OtherFactionId < Factions.Num(); ++OtherFactionId)
		{
			if (OtherFactionId == FactionId)
				continue; // Skip self

			const FFactionData& OtherFaction = Factions[OtherFactionId];

			// Check if other faction has surplus of this good
			const int32* SurplusVolume = OtherFaction.CurrentExports.Find(NeededGood);
			if (SurplusVolume && *SurplusVolume > 0)
			{
				ProducerFactionVolumes.Add(OtherFactionId, *SurplusVolume);
			}
		}

		// If this shortage depends on external production, record critical dependency
		if (ProducerFactionVolumes.Num() > 0)
		{
			// Build array of producing faction IDs for this good
			TArray<int32> ProducerFactions;
			for (const auto& Pair : ProducerFactionVolumes)
			{
				ProducerFactions.Add(Pair.Key);
			}

			// Create dependency entry
			FFactionDependencyGoods Dependency;
			Dependency.Goods.Add(NeededGood);

			// Store in CriticalDependencies map
			// Key = primary producing faction (highest volume)
			int32 PrimaryProducer = -1;
			int32 MaxVolume = 0;
			for (const auto& Pair : ProducerFactionVolumes)
			{
				if (Pair.Value > MaxVolume)
				{
					MaxVolume = Pair.Value;
					PrimaryProducer = Pair.Key;
				}
			}

			if (PrimaryProducer >= 0)
			{
				// Add or update dependency on primary producer
				FFactionDependencyGoods& Entry = Faction.CriticalDependencies.FindOrAdd(PrimaryProducer);
				Entry.Goods.AddUnique(NeededGood);
			}
		}
	}

	// Log critical dependencies for debugging
	if (Faction.CriticalDependencies.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("FactionSubsystem: %s has %d critical dependencies"),
			*Faction.FactionName, Faction.CriticalDependencies.Num());

		for (const auto& Pair : Faction.CriticalDependencies)
		{
			int32 ProducerFactionId = Pair.Key;
			const FFactionDependencyGoods& DepGoods = Pair.Value;

			if (ProducerFactionId >= 0 && ProducerFactionId < Factions.Num())
			{
				const FFactionData& ProducerFaction = Factions[ProducerFactionId];
				UE_LOG(LogTemp, Log, TEXT("  -> Depends on %s for %d goods"),
					*ProducerFaction.FactionName, DepGoods.Goods.Num());
			}
		}
	}
}

// ============================================================================
// SHIP PRODUCTION AWARENESS (Sprint 7)
// ============================================================================

void UFactionSubsystem::EvaluateShipProductionCapability(int32 FactionId)
{
	if (FactionId < 0 || FactionId >= Factions.Num())
		return;

	FFactionData& Faction = Factions[FactionId];

	// Query universe for faction territory
	UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	if (!UniverseSys || !UniverseSys->IsUniverseGenerated())
		return;

	const FUniverseData& Universe = UniverseSys->GetUniverseData();

	// CONSUMER PATTERN: Query existing markets for ship component availability
	// Does NOT create parallel ship economy - reads from existing market data

	// Track availability of key ship component categories
	bool bHasFrames = false;
	bool bHasEngines = false;
	bool bHasPowerPlants = false;
	bool bHasShields = false;
	bool bHasWeapons = false;
	bool bHasUtility = false;

	// Scan faction-controlled shipyards and trade hubs for ship components
	for (int32 SystemId : Faction.ControlledSystemIds)
	{
		if (SystemId < 0 || SystemId >= Universe.Systems.Num())
			continue;

		const FStarSystemData& System = Universe.Systems[SystemId];

		for (const FLocationData& Location : System.Locations)
		{
			// Only check faction-owned production/trade locations
			if (Location.OwningFactionId == FactionId &&
				(Location.LocationType == ELocationType::Shipyard || Location.LocationType == ELocationType::TradeHub))
			{
				// Check market inventory for ship components (consumer query)
				for (const FMarketGoodEntry& Entry : Location.Market.Goods)
				{
					if (Entry.Stock > 0)
					{
						// Check component category
						if (Entry.GoodType >= EGoodType::ShipFrame_StarterMining && Entry.GoodType <= EGoodType::ShipFrame_Battleship)
							bHasFrames = true;
						else if (Entry.GoodType >= EGoodType::Engine_TitanI_Small && Entry.GoodType <= EGoodType::Engine_TitanII_Large)
							bHasEngines = true;
						else if (Entry.GoodType >= EGoodType::PowerPlant_NovaI_Small && Entry.GoodType <= EGoodType::PowerPlant_NovaI_Large)
							bHasPowerPlants = true;
						else if (Entry.GoodType >= EGoodType::Shield_AtlasI_Small && Entry.GoodType <= EGoodType::Shield_AtlasII_Medium)
							bHasShields = true;
						else if (Entry.GoodType >= EGoodType::Weapon_Laser_Small && Entry.GoodType <= EGoodType::Weapon_MissileLauncher_Small)
							bHasWeapons = true;
						else if (Entry.GoodType >= EGoodType::Utility_MiningLaser_Small && Entry.GoodType <= EGoodType::Utility_SensorArray_Medium)
							bHasUtility = true;
					}
				}
			}
		}
	}

	// Log ship production capability (for future ship-building missions/events)
	bool bCanProduceShips = bHasFrames && bHasEngines && bHasPowerPlants;
	bool bCanProduceCombatShips = bCanProduceShips && bHasWeapons && bHasShields;

	if (bCanProduceShips)
	{
		UE_LOG(LogTemp, Log, TEXT("FactionSubsystem: %s has ship production capability (Frames:%d Engines:%d Power:%d Shields:%d Weapons:%d)"),
			*Faction.FactionName, bHasFrames, bHasEngines, bHasPowerPlants, bHasShields, bHasWeapons);
	}
	else
	{
		// Log what's missing
		TArray<FString> MissingComponents;
		if (!bHasFrames) MissingComponents.Add(TEXT("Frames"));
		if (!bHasEngines) MissingComponents.Add(TEXT("Engines"));
		if (!bHasPowerPlants) MissingComponents.Add(TEXT("PowerPlants"));

		if (MissingComponents.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("FactionSubsystem: %s CANNOT produce ships - missing: %s"),
				*Faction.FactionName, *FString::Join(MissingComponents, TEXT(", ")));
		}
	}

	// Note: This is awareness only - actual ship production will be handled by logistics/economy systems
	// Future sprints can use this data to trigger ship-building missions or economic events
}

// ============================================================================
// FUTURE HOOKS (Sprint 7: data-only)
// ============================================================================

void UFactionSubsystem::TriggerFactionEvents(int32 FactionId)
{
	if (FactionId < 0 || FactionId >= Factions.Num())
		return;

	const FFactionData& Faction = Factions[FactionId];

	// Generate events based on analysis data (future hooks for news/missions)

	// High economic stress event
	if (Faction.CurrentEconomicStress > 0.7f)
	{
		FString Event = FString::Printf(TEXT("Economic Crisis: %s experiencing severe shortages (stress: %.2f)"),
			*Faction.FactionName, Faction.CurrentEconomicStress);
		RecordEconomicEvent(FactionId, Event);

		// Mission opportunity: deliver critical goods
		if (Faction.StrategicPriorities.Num() > 0)
		{
			const FStrategicPriority& TopPriority = Faction.StrategicPriorities[0];
			FString GoodName = UEnum::GetDisplayValueAsText(TopPriority.GoodType).ToString();
			FString Mission = FString::Printf(TEXT("URGENT: %s needs %s delivery to home system (ID: %d)"),
				*Faction.FactionName, *GoodName, Faction.HomeSystemId);
			RecordMissionOpportunity(FactionId, Mission);
		}
	}

	// Critical dependency event
	if (Faction.CriticalDependencies.Num() > 0)
	{
		for (const auto& Pair : Faction.CriticalDependencies)
		{
			int32 SupplierFactionId = Pair.Key;
			const FFactionDependencyGoods& DepGoods = Pair.Value;

			if (DepGoods.Goods.Num() >= 3) // Depends on supplier for 3+ goods
			{
				FFactionData SupplierFaction;
				if (GetFactionById(SupplierFactionId, SupplierFaction))
				{
					FString Event = FString::Printf(TEXT("Trade Dependency: %s heavily dependent on %s for %d critical goods"),
						*Faction.FactionName, *SupplierFaction.FactionName, DepGoods.Goods.Num());
					RecordEconomicEvent(FactionId, Event);

					// Mission opportunity: establish direct trade route
					FString Mission = FString::Printf(TEXT("Trade Route: Establish reliable supply line between %s and %s"),
						*Faction.FactionName, *SupplierFaction.FactionName);
					RecordMissionOpportunity(FactionId, Mission);
				}
			}
		}
	}

	// Large surplus event (export opportunity)
	if (Faction.CurrentExports.Num() > 0)
	{
		for (const auto& Pair : Faction.CurrentExports)
		{
			if (Pair.Value > 1000) // Large surplus
			{
				FString GoodName = UEnum::GetDisplayValueAsText(Pair.Key).ToString();
				FString Event = FString::Printf(TEXT("Export Boom: %s has major surplus of %s (%d units)"),
					*Faction.FactionName, *GoodName, Pair.Value);
				RecordEconomicEvent(FactionId, Event);
			}
		}
	}
}

void UFactionSubsystem::RecordEconomicEvent(int32 FactionId, const FString& EventDescription)
{
	if (FactionId >= 0 && FactionId < Factions.Num())
	{
		Factions[FactionId].NewsEvents.Add(EventDescription);
		UE_LOG(LogTemp, Verbose, TEXT("FactionSubsystem: [Faction %d] Event: %s"), FactionId, *EventDescription);
	}
}

void UFactionSubsystem::RecordMissionOpportunity(int32 FactionId, const FString& OpportunityDescription)
{
	if (FactionId >= 0 && FactionId < Factions.Num())
	{
		Factions[FactionId].MissionOpportunities.Add(OpportunityDescription);
		UE_LOG(LogTemp, Verbose, TEXT("FactionSubsystem: [Faction %d] Mission: %s"), FactionId, *OpportunityDescription);
	}
}

// ============================================================================
// DEBUG & DIAGNOSTICS
// ============================================================================

void UFactionSubsystem::PrintFactionSummary(int32 FactionId) const
{
	FFactionData Faction;
	if (!GetFactionById(FactionId, Faction))
		return;

	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("FACTION: %s (ID: %d)"), *Faction.FactionName, Faction.FactionId);
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Type: %s"), 
		Faction.FactionType == EFactionType::Human ? TEXT("Human") : TEXT("Alien"));
	UE_LOG(LogTemp, Log, TEXT("Home System: %d"), Faction.HomeSystemId);
	UE_LOG(LogTemp, Log, TEXT("Core Systems: %d"), Faction.CoreSystemIds.Num());
	UE_LOG(LogTemp, Log, TEXT("Controlled Systems: %d"), Faction.ControlledSystemIds.Num());
	UE_LOG(LogTemp, Log, TEXT("Total Population: %lld"), Faction.TotalPopulation);
	UE_LOG(LogTemp, Log, TEXT("Economic Strength: %.1f"), Faction.EconomicStrength);
	UE_LOG(LogTemp, Log, TEXT("Industrial Strength: %.1f"), Faction.IndustrialStrength);
	UE_LOG(LogTemp, Log, TEXT("Credits: %.0f"), Faction.Credits);
	UE_LOG(LogTemp, Log, TEXT("Economic Stress: %.2f"), Faction.CurrentEconomicStress);
	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void UFactionSubsystem::PrintFactionEconomy(int32 FactionId) const
{
	FFactionData Faction;
	if (!GetFactionById(FactionId, Faction))
		return;

	UE_LOG(LogTemp, Log, TEXT("========== %s - ECONOMY =========="), *Faction.FactionName);
	UE_LOG(LogTemp, Log, TEXT("Economic Stress: %.2f"), Faction.CurrentEconomicStress);
	UE_LOG(LogTemp, Log, TEXT("Strategic Priorities (Shortages): %d"), Faction.StrategicPriorities.Num());

	// Show top shortage priorities
	if (Faction.StrategicPriorities.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Top Shortage Priorities:"));
		for (int32 i = 0; i < FMath::Min(5, Faction.StrategicPriorities.Num()); ++i)
		{
			const FStrategicPriority& Priority = Faction.StrategicPriorities[i];
			FString GoodName = UEnum::GetDisplayValueAsText(Priority.GoodType).ToString();
			UE_LOG(LogTemp, Log, TEXT("  %d. %s - Priority: %.1f, Severity: %.2f"),
				i + 1, *GoodName, Priority.Priority, Priority.ShortageSeverity);
		}
	}

	// Show current exports (surpluses)
	UE_LOG(LogTemp, Log, TEXT("Current Exports (Surpluses): %d goods"), Faction.CurrentExports.Num());
	if (Faction.CurrentExports.Num() > 0)
	{
		TArray<EGoodType> ExportGoods;
		Faction.CurrentExports.GetKeys(ExportGoods);

		for (int32 i = 0; i < FMath::Min(5, ExportGoods.Num()); ++i)
		{
			EGoodType GoodType = ExportGoods[i];
			int32 Volume = Faction.CurrentExports[GoodType];
			FString GoodName = UEnum::GetDisplayValueAsText(GoodType).ToString();
			UE_LOG(LogTemp, Log, TEXT("  - %s: %d units"), *GoodName, Volume);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("===================================="));
}

void UFactionSubsystem::PrintFactionDependencies(int32 FactionId) const
{
	FFactionData Faction;
	if (!GetFactionById(FactionId, Faction))
		return;

	UE_LOG(LogTemp, Log, TEXT("========== %s - DEPENDENCIES =========="), *Faction.FactionName);
	UE_LOG(LogTemp, Log, TEXT("Critical Dependencies: %d supplier factions"), Faction.CriticalDependencies.Num());

	// Show detailed dependencies on other factions
	if (Faction.CriticalDependencies.Num() > 0)
	{
		for (const auto& Pair : Faction.CriticalDependencies)
		{
			int32 SupplierFactionId = Pair.Key;
			const FFactionDependencyGoods& DependencyGoods = Pair.Value;

			// Get supplier faction name
			FFactionData SupplierFaction;
			FString SupplierName = FString::Printf(TEXT("Faction %d"), SupplierFactionId);
			if (GetFactionById(SupplierFactionId, SupplierFaction))
			{
				SupplierName = SupplierFaction.FactionName;
			}

			UE_LOG(LogTemp, Log, TEXT("  Depends on %s for %d goods:"),
				*SupplierName, DependencyGoods.Goods.Num());

			// List the specific goods
			for (EGoodType GoodType : DependencyGoods.Goods)
			{
				FString GoodName = UEnum::GetDisplayValueAsText(GoodType).ToString();
				UE_LOG(LogTemp, Log, TEXT("    - %s"), *GoodName);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("  No critical dependencies (self-sufficient or no shortages)"));
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void UFactionSubsystem::PrintUniverseFactionReport() const
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("FactionSubsystem: Not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("UNIVERSE FACTION REPORT"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Total Factions: %d"), Factions.Num());
	UE_LOG(LogTemp, Log, TEXT(""));

	for (const FFactionData& Faction : Factions)
	{
		UE_LOG(LogTemp, Log, TEXT("[%d] %s (%s)"), 
			Faction.FactionId,
			*Faction.FactionName,
			Faction.FactionType == EFactionType::Human ? TEXT("Human") : TEXT("Alien"));
		UE_LOG(LogTemp, Log, TEXT("    Territory: %d systems, Pop: %lld, Stress: %.2f"),
			Faction.ControlledSystemIds.Num(),
			Faction.TotalPopulation,
			Faction.CurrentEconomicStress);
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}
