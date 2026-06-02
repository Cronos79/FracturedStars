// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/UniverseGenerator.h"
#include "Containers/Queue.h"

FUniverseData UUniverseGenerator::GenerateUniverse(const FUniverseConfig& Config)
{
	UE_LOG(LogTemp, Log, TEXT("=== Universe Generation Started ==="));
	UE_LOG(LogTemp, Log, TEXT("Seed: %d, Systems: %d, Factions: %d"), Config.Seed, Config.SystemCount, Config.FactionCount);

	FUniverseData Universe;
	Universe.Config = Config;

	// Initialize deterministic random stream
	FRandomStream RandStream(Config.Seed);

	// Generation pipeline - ORDER IS CRITICAL
	GenerateSystems(Universe, RandStream);
	GenerateJumpNetwork(Universe, RandStream);
	EnsureTraversability(Universe, RandStream); // MUST happen before faction placement!
	PlaceFactionCores(Universe, RandStream);    // Now pathfinding works correctly
	AssignRegions(Universe);
	CalculateLawfulness(Universe);
	GenerateSystemContent(Universe, RandStream); // Generate content after regions are assigned
	ValidateGeneration(Universe);

	UE_LOG(LogTemp, Log, TEXT("=== Universe Generation Complete ==="));
	return Universe;
}

void UUniverseGenerator::GenerateSystems(FUniverseData& Universe, FRandomStream& RandStream)
{
	UE_LOG(LogTemp, Log, TEXT("Generating %d star systems..."), Universe.Config.SystemCount);

	Universe.Systems.Reserve(Universe.Config.SystemCount);

	for (int32 i = 0; i < Universe.Config.SystemCount; ++i)
	{
		FStarSystemData System;
		System.SystemId = i;
		System.SystemName = GenerateSystemName(i, RandStream);
		System.Coordinates = GenerateCoordinates(i, Universe.Config.GalacticRadius, RandStream);

		Universe.Systems.Add(System);
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d systems"), Universe.Systems.Num());
}

void UUniverseGenerator::GenerateJumpNetwork(FUniverseData& Universe, FRandomStream& RandStream)
{
	UE_LOG(LogTemp, Log, TEXT("Generating jump network..."));

	const int32 AvgConnections = Universe.Config.AvgConnectionsPerSystem;
	int32 TotalConnections = 0;

	for (FStarSystemData& System : Universe.Systems)
	{
		// Target connections: AvgConnections ± 1
		int32 DesiredConnections = FMath::Clamp(
			AvgConnections + RandStream.RandRange(-1, 1),
			2, // Minimum 2 connections
			6  // Maximum 6 connections
		);

		// Current connections
		int32 CurrentConnections = System.ConnectedSystemIds.Num();

		// Add connections if needed
		while (CurrentConnections < DesiredConnections)
		{
			// Find closest unconnected system
			TSet<int32> ExcludeSet;
			ExcludeSet.Add(System.SystemId);
			for (int32 ConnectedId : System.ConnectedSystemIds)
			{
				ExcludeSet.Add(ConnectedId);
			}

			int32 ClosestId = FindClosestSystem(System.Coordinates, Universe.Systems, ExcludeSet);

			if (ClosestId == -1)
				break; // No more valid connections

			// Add BIDIRECTIONAL connection
			FStarSystemData& TargetSystem = Universe.Systems[ClosestId];
			System.ConnectedSystemIds.Add(ClosestId);
			TargetSystem.ConnectedSystemIds.Add(System.SystemId);

			TotalConnections++;
			CurrentConnections++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Created %d bidirectional connections"), TotalConnections);
}

void UUniverseGenerator::PlaceFactionCores(FUniverseData& Universe, FRandomStream& RandStream)
{
	UE_LOG(LogTemp, Log, TEXT("Placing %d faction core systems..."), Universe.Config.FactionCount);

	Universe.FactionHomeSystems.Empty();
	TArray<int32> SelectedCores;

	// Select first faction core randomly
	int32 FirstCore = RandStream.RandRange(0, Universe.Systems.Num() - 1);
	SelectedCores.Add(FirstCore);
	Universe.FactionHomeSystems.Add(FirstCore);
	UE_LOG(LogTemp, Log, TEXT("  Faction 0: System %d (%s) [Starting faction]"), 
		FirstCore, *Universe.Systems[FirstCore].SystemName);

	// Select remaining cores with ENFORCED minimum separation
	for (int32 FactionIdx = 1; FactionIdx < Universe.Config.FactionCount; ++FactionIdx)
	{
		int32 BestCandidate = -1;
		int32 BestMinDistance = 0;

		// Try many more candidates to ensure good separation
		for (int32 Attempt = 0; Attempt < 200; ++Attempt)
		{
			int32 Candidate = RandStream.RandRange(0, Universe.Systems.Num() - 1);

			// Skip if already selected
			if (SelectedCores.Contains(Candidate))
				continue;

			// Calculate minimum distance to existing cores
			int32 MinDistance = MAX_int32;
			for (int32 ExistingCore : SelectedCores)
			{
				int32 Distance = CalculateJumpDistance(Candidate, ExistingCore, Universe.Systems);
				MinDistance = FMath::Min(MinDistance, Distance);
			}

			// ENFORCE minimum separation requirement
			if (MinDistance < Universe.Config.MinFactionSeparation)
				continue; // Skip candidates that are too close

			// Keep candidate with best separation
			if (MinDistance > BestMinDistance)
			{
				BestMinDistance = MinDistance;
				BestCandidate = Candidate;
			}
		}

		// If we couldn't find a candidate meeting requirements, take the best we found
		if (BestCandidate == -1 && BestMinDistance > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("  Faction %d: Could not meet separation requirement of %d jumps (best: %d)"),
				FactionIdx, Universe.Config.MinFactionSeparation, BestMinDistance);
		}

		if (BestCandidate != -1)
		{
			SelectedCores.Add(BestCandidate);
			Universe.FactionHomeSystems.Add(BestCandidate);
			UE_LOG(LogTemp, Log, TEXT("  Faction %d: System %d (%s) - Min separation: %d jumps"),
				FactionIdx, BestCandidate, *Universe.Systems[BestCandidate].SystemName, BestMinDistance);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("  Faction %d: FAILED to place! Increase system count or reduce factions."), FactionIdx);
		}
	}
}

void UUniverseGenerator::AssignRegions(FUniverseData& Universe)
{
	UE_LOG(LogTemp, Log, TEXT("Assigning regions..."));

	// Calculate distance from each system to ALL faction cores
	for (FStarSystemData& System : Universe.Systems)
	{
		int32 MinDistance = MAX_int32;
		int32 SecondMinDistance = MAX_int32;
		int32 NearestFaction = -1;

		// Find nearest and second-nearest faction
		for (int32 FactionIdx = 0; FactionIdx < Universe.FactionHomeSystems.Num(); ++FactionIdx)
		{
			int32 CoreSystemId = Universe.FactionHomeSystems[FactionIdx];
			int32 Distance = CalculateJumpDistance(System.SystemId, CoreSystemId, Universe.Systems);

			if (Distance < MinDistance)
			{
				SecondMinDistance = MinDistance;
				MinDistance = Distance;
				NearestFaction = FactionIdx;
			}
			else if (Distance < SecondMinDistance)
			{
				SecondMinDistance = Distance;
			}
		}

		System.DistanceToFactionCore = MinDistance;

		// Calculate influence gap between nearest and second-nearest factions
		int32 InfluenceGap = SecondMinDistance - MinDistance;

		// ====== IMPROVED DISPUTED LOGIC ======
		// Disputed = Two factions have SIMILAR influence over the system
		// Example: System is 9 jumps from Faction A, 11 jumps from Faction B
		//          → gap of 2 jumps = contested border!

		bool bIsDisputed = false;
		if (MinDistance <= 12 &&           // Not too far from nearest faction
			SecondMinDistance <= 16 &&     // Second faction also has reach
			InfluenceGap <= 3)             // Similar influence (close competition)
		{
			bIsDisputed = true;
		}

		// Assign regions based on distance and influence
		if (bIsDisputed)
		{
			// Contested border zone - two factions competing for control
			System.RegionType = ERegionType::Disputed;
			System.ControllingFactionId = -1; // No clear controller
		}
		else if (MinDistance == 0)
		{
			// Home system
			System.RegionType = ERegionType::FactionCore;
			System.ControllingFactionId = NearestFaction;
		}
		else if (MinDistance <= 2)
		{
			// Core territory (2 jumps from home)
			System.RegionType = ERegionType::FactionCore;
			System.ControllingFactionId = NearestFaction;
		}
		else if (MinDistance <= 5)
		{
			// Frontier (3-5 jumps from home)
			System.RegionType = ERegionType::FactionFrontier;
			System.ControllingFactionId = NearestFaction;
		}
		else if (MinDistance <= 12)
		{
			// Outer influence sphere - neutral independent systems
			System.RegionType = ERegionType::Neutral;
			System.ControllingFactionId = -1;
		}
		else
		{
			// Far from all factions = lawless deep space
			System.RegionType = ERegionType::Lawless;
			System.ControllingFactionId = -1;
		}
	}

	// Second pass: Mark frontier systems as disputed if directly adjacent to rival faction territory
	for (FStarSystemData& System : Universe.Systems)
	{
		if (System.RegionType == ERegionType::FactionFrontier)
		{
			// Check if this frontier system touches a rival faction's core or frontier
			bool bTouchesRival = false;
			for (int32 ConnectedId : System.ConnectedSystemIds)
			{
				const FStarSystemData& ConnectedSystem = Universe.Systems[ConnectedId];

				// If connected to rival faction's core/frontier territory
				if ((ConnectedSystem.RegionType == ERegionType::FactionCore || 
					 ConnectedSystem.RegionType == ERegionType::FactionFrontier) &&
					ConnectedSystem.ControllingFactionId != -1 &&
					ConnectedSystem.ControllingFactionId != System.ControllingFactionId)
				{
					bTouchesRival = true;
					break;
				}
			}

			// Direct border contact = disputed
			if (bTouchesRival)
			{
				System.RegionType = ERegionType::Disputed;
				System.ControllingFactionId = -1; // No longer clearly controlled
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Regions assigned"));
}

void UUniverseGenerator::CalculateLawfulness(FUniverseData& Universe)
{
	UE_LOG(LogTemp, Log, TEXT("Calculating lawfulness values..."));

	for (FStarSystemData& System : Universe.Systems)
	{
		switch (System.RegionType)
		{
		case ERegionType::FactionCore:
			System.Lawfulness = FMath::FRandRange(0.9f, 1.0f);
			break;
		case ERegionType::FactionFrontier:
			System.Lawfulness = FMath::FRandRange(0.6f, 0.8f);
			break;
		case ERegionType::Neutral:
			System.Lawfulness = FMath::FRandRange(0.4f, 0.6f);
			break;
		case ERegionType::Disputed:
			System.Lawfulness = FMath::FRandRange(0.2f, 0.4f);
			break;
		case ERegionType::Lawless:
			System.Lawfulness = FMath::FRandRange(0.0f, 0.3f);
			break;
		default:
			System.Lawfulness = 0.5f;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Lawfulness calculated"));
}

void UUniverseGenerator::EnsureTraversability(FUniverseData& Universe, FRandomStream& RandStream)
{
	UE_LOG(LogTemp, Log, TEXT("Ensuring universe traversability..."));

	// Find all disconnected components
	TSet<int32> Visited;
	TArray<TArray<int32>> Components;

	for (const FStarSystemData& System : Universe.Systems)
	{
		if (!Visited.Contains(System.SystemId))
		{
			// BFS to find component
			TArray<int32> Component;
			TQueue<int32> Queue;
			Queue.Enqueue(System.SystemId);
			Visited.Add(System.SystemId);

			while (!Queue.IsEmpty())
			{
				int32 CurrentId;
				Queue.Dequeue(CurrentId);
				Component.Add(CurrentId);

				for (int32 ConnectedId : Universe.Systems[CurrentId].ConnectedSystemIds)
				{
					if (!Visited.Contains(ConnectedId))
					{
						Queue.Enqueue(ConnectedId);
						Visited.Add(ConnectedId);
					}
				}
			}

			Components.Add(Component);
		}
	}

	// Connect components if multiple exist
	if (Components.Num() > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found %d disconnected components! Connecting..."), Components.Num());

		for (int32 i = 1; i < Components.Num(); ++i)
		{
			// Find closest systems between component 0 and component i
			int32 BestA = -1;
			int32 BestB = -1;
			float BestDistance = MAX_flt;

			for (int32 IdA : Components[0])
			{
				for (int32 IdB : Components[i])
				{
					float Distance = GetDistance(
						Universe.Systems[IdA].Coordinates,
						Universe.Systems[IdB].Coordinates
					);

					if (Distance < BestDistance)
					{
						BestDistance = Distance;
						BestA = IdA;
						BestB = IdB;
					}
				}
			}

			// Connect them bidirectionally
			if (BestA != -1 && BestB != -1)
			{
				Universe.Systems[BestA].ConnectedSystemIds.Add(BestB);
				Universe.Systems[BestB].ConnectedSystemIds.Add(BestA);
				UE_LOG(LogTemp, Log, TEXT("  Connected component via systems %d <-> %d"), BestA, BestB);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Universe is fully connected"));
	}
}

void UUniverseGenerator::GenerateSystemContent(FUniverseData& Universe, FRandomStream& RandStream)
{
	UE_LOG(LogTemp, Log, TEXT("Generating system content..."));

	for (FStarSystemData& System : Universe.Systems)
	{
		// Create a system-specific random stream for determinism
		FRandomStream SystemRand(RandStream.GetCurrentSeed() + System.SystemId);

		// Generate celestial bodies first
		GenerateCelestialBodies(System, SystemRand);

		// Then generate locations based on the system's characteristics
		GenerateLocations(System, SystemRand);
	}

	UE_LOG(LogTemp, Log, TEXT("System content generation complete"));
}

void UUniverseGenerator::GenerateCelestialBodies(FStarSystemData& System, FRandomStream& RandStream)
{
	int32 NextBodyId = 0;

	// Generate 1-3 stars (most systems have 1, some binary/trinary)
	int32 StarCount = RandStream.RandRange(1, 100) > 85 ? (RandStream.RandRange(1, 100) > 70 ? 3 : 2) : 1;

	for (int32 i = 0; i < StarCount; ++i)
	{
		FCelestialBodyData Star;
		Star.BodyId = NextBodyId++;
		Star.BodyName = GenerateCelestialBodyName(System.SystemId, Star.BodyId, ECelestialBodyType::Star, RandStream);
		Star.BodyType = ECelestialBodyType::Star;
		Star.ParentBodyId = -1;
		Star.ResourceRichness = 0.0f; // Stars don't have mineable resources
		System.CelestialBodies.Add(Star);
	}

	// Generate planets (0-12 planets, influenced by system type)
	int32 PlanetCount = RandStream.RandRange(2, 12);

	// Lawless/frontier systems may have more asteroid fields
	bool bHasAsteroidBelt = RandStream.RandRange(1, 100) > 70;

	for (int32 i = 0; i < PlanetCount; ++i)
	{
		FCelestialBodyData Planet;
		Planet.BodyId = NextBodyId++;

		// Determine planet type
		int32 TypeRoll = RandStream.RandRange(1, 100);
		if (TypeRoll > 85)
		{
			Planet.BodyType = ECelestialBodyType::GasGiant;
		}
		else if (TypeRoll > 75)
		{
			Planet.BodyType = ECelestialBodyType::IceGiant;
		}
		else if (TypeRoll > 95)
		{
			Planet.BodyType = ECelestialBodyType::DwarfPlanet;
		}
		else
		{
			Planet.BodyType = ECelestialBodyType::Planet;
		}

		Planet.BodyName = GenerateCelestialBodyName(System.SystemId, Planet.BodyId, Planet.BodyType, RandStream);
		Planet.ParentBodyId = -1; // Planets orbit stars, not tracked explicitly
		Planet.ResourceRichness = RandStream.FRandRange(0.1f, 1.0f);

		System.CelestialBodies.Add(Planet);

		// Generate moons for this planet (0-3 moons, gas giants more likely to have moons)
		int32 MoonChance = (Planet.BodyType == ECelestialBodyType::GasGiant) ? 80 : 40;
		if (RandStream.RandRange(1, 100) < MoonChance)
		{
			int32 MoonCount = RandStream.RandRange(1, 3);
			for (int32 m = 0; m < MoonCount; ++m)
			{
				FCelestialBodyData Moon;
				Moon.BodyId = NextBodyId++;
				Moon.BodyName = GenerateCelestialBodyName(System.SystemId, Moon.BodyId, ECelestialBodyType::Moon, RandStream);
				Moon.BodyType = ECelestialBodyType::Moon;
				Moon.ParentBodyId = Planet.BodyId;
				Moon.ResourceRichness = RandStream.FRandRange(0.1f, 0.8f);
				System.CelestialBodies.Add(Moon);
			}
		}
	}

	// Add asteroid field if applicable
	if (bHasAsteroidBelt)
	{
		FCelestialBodyData Asteroids;
		Asteroids.BodyId = NextBodyId++;
		Asteroids.BodyName = GenerateCelestialBodyName(System.SystemId, Asteroids.BodyId, ECelestialBodyType::AsteroidField, RandStream);
		Asteroids.BodyType = ECelestialBodyType::AsteroidField;
		Asteroids.ParentBodyId = -1;
		Asteroids.ResourceRichness = RandStream.FRandRange(0.6f, 1.0f); // Asteroid fields often resource-rich
		System.CelestialBodies.Add(Asteroids);
	}

	// Rare anomalies in some systems
	if (RandStream.RandRange(1, 100) > 95)
	{
		FCelestialBodyData Anomaly;
		Anomaly.BodyId = NextBodyId++;
		Anomaly.BodyName = GenerateCelestialBodyName(System.SystemId, Anomaly.BodyId, ECelestialBodyType::Anomaly, RandStream);
		Anomaly.BodyType = ECelestialBodyType::Anomaly;
		Anomaly.ParentBodyId = -1;
		Anomaly.ResourceRichness = 0.0f; // Anomalies aren't mined
		System.CelestialBodies.Add(Anomaly);
	}
}

void UUniverseGenerator::GenerateLocations(FStarSystemData& System, FRandomStream& RandStream)
{
	// Location count and types depend heavily on region type
	int32 LocationCount = 0;

	switch (System.RegionType)
	{
	case ERegionType::FactionCore:
		// Core systems are densely developed
		LocationCount = RandStream.RandRange(3, 6);
		break;

	case ERegionType::FactionFrontier:
		// Frontier systems have moderate development
		LocationCount = RandStream.RandRange(2, 4);
		break;

	case ERegionType::Neutral:
		// Independent systems vary widely
		LocationCount = RandStream.RandRange(1, 3);
		break;

	case ERegionType::Lawless:
		// Lawless systems often have fewer legitimate locations
		LocationCount = RandStream.RandRange(1, 2);
		break;

	case ERegionType::Disputed:
		// Contested systems may have damaged or contested locations
		LocationCount = RandStream.RandRange(1, 3);
		break;

	default:
		LocationCount = 1; // Always at least one location
		break;
	}

	// Ensure every system has at least ONE location (validation requirement)
	LocationCount = FMath::Max(1, LocationCount);

	for (int32 i = 0; i < LocationCount; ++i)
	{
		FLocationData Location;
		Location.LocationId = i;

		// Determine location type based on region
		Location.LocationType = DetermineLocationType(System.RegionType, RandStream);
		Location.LocationName = GenerateLocationName(System.SystemId, Location.LocationId, Location.LocationType, RandStream);

		// Assign ownership based on region rules
		AssignLocationOwnership(Location, System.RegionType, System.ControllingFactionId, RandStream);

		// Generate population (varies by type and region)
		Location.Population = GeneratePopulation(Location.LocationType, System.RegionType, RandStream);

		// Security rating influenced by region lawfulness
		Location.SecurityRating = System.Lawfulness + RandStream.FRandRange(-0.2f, 0.2f);
		Location.SecurityRating = FMath::Clamp(Location.SecurityRating, 0.0f, 1.0f);

		// Generate economy hooks
		Location.Produces = CreateProductionProfile(Location.LocationType, System.RegionType, RandStream);
		Location.Consumes = CreateConsumptionProfile(Location.LocationType, Location.Population, RandStream);

		// Initialize empty inventory (for future economy system)
		Location.Inventory.Empty();

		System.Locations.Add(Location);
	}
}

ELocationType UUniverseGenerator::DetermineLocationType(ERegionType RegionType, FRandomStream& RandStream)
{
	int32 Roll = RandStream.RandRange(1, 100);

	switch (RegionType)
	{
	case ERegionType::FactionCore:
		// Core systems: major stations, trade hubs, military bases
		if (Roll > 80) return ELocationType::MilitaryBase;
		if (Roll > 60) return ELocationType::TradeHub;
		if (Roll > 40) return ELocationType::Shipyard;
		if (Roll > 20) return ELocationType::ResearchFacility;
		return ELocationType::Station;

	case ERegionType::FactionFrontier:
		// Frontier: mix of military and civilian
		if (Roll > 70) return ELocationType::MilitaryBase;
		if (Roll > 50) return ELocationType::RefuelingDepot;
		if (Roll > 30) return ELocationType::MiningColony;
		return ELocationType::Colony;

	case ERegionType::Neutral:
		// Neutral: civilian infrastructure
		if (Roll > 70) return ELocationType::TradeHub;
		if (Roll > 50) return ELocationType::MiningColony;
		if (Roll > 30) return ELocationType::Colony;
		return ELocationType::Station;

	case ERegionType::Lawless:
		// Lawless: pirate outposts, abandoned facilities
		if (Roll > 60) return ELocationType::PirateOutpost;
		if (Roll > 30) return ELocationType::AbandonedFacility;
		return ELocationType::RefuelingDepot;

	case ERegionType::Disputed:
		// Disputed: damaged or contested facilities
		if (Roll > 70) return ELocationType::MilitaryBase;
		if (Roll > 40) return ELocationType::AbandonedFacility;
		return ELocationType::Station;

	default:
		return ELocationType::Station;
	}
}

int32 UUniverseGenerator::GeneratePopulation(ELocationType LocationType, ERegionType RegionType, FRandomStream& RandStream)
{
	int32 BasePopulation = 0;

	switch (LocationType)
	{
	case ELocationType::TradeHub:
		BasePopulation = RandStream.RandRange(50000, 500000);
		break;
	case ELocationType::Colony:
		BasePopulation = RandStream.RandRange(10000, 100000);
		break;
	case ELocationType::Station:
		BasePopulation = RandStream.RandRange(5000, 50000);
		break;
	case ELocationType::MilitaryBase:
		BasePopulation = RandStream.RandRange(10000, 50000);
		break;
	case ELocationType::Shipyard:
		BasePopulation = RandStream.RandRange(20000, 100000);
		break;
	case ELocationType::MiningColony:
		BasePopulation = RandStream.RandRange(1000, 20000);
		break;
	case ELocationType::ResearchFacility:
		BasePopulation = RandStream.RandRange(500, 10000);
		break;
	case ELocationType::RefuelingDepot:
		BasePopulation = RandStream.RandRange(100, 5000);
		break;
	case ELocationType::PirateOutpost:
		BasePopulation = RandStream.RandRange(500, 5000);
		break;
	case ELocationType::AbandonedFacility:
		BasePopulation = 0; // Abandoned
		break;
	default:
		BasePopulation = RandStream.RandRange(1000, 10000);
		break;
	}

	// Region modifier (core systems more populous)
	float RegionMultiplier = 1.0f;
	switch (RegionType)
	{
	case ERegionType::FactionCore:
		RegionMultiplier = RandStream.FRandRange(1.2f, 1.5f);
		break;
	case ERegionType::Lawless:
		RegionMultiplier = RandStream.FRandRange(0.3f, 0.7f);
		break;
	case ERegionType::Disputed:
		RegionMultiplier = RandStream.FRandRange(0.4f, 0.8f); // War damage
		break;
	default:
		RegionMultiplier = 1.0f;
		break;
	}

	return FMath::RoundToInt(BasePopulation * RegionMultiplier);
}

void UUniverseGenerator::AssignLocationOwnership(FLocationData& Location, ERegionType RegionType, int32 ControllingFactionId, FRandomStream& RandStream)
{
	switch (RegionType)
	{
	case ERegionType::FactionCore:
		// Core systems: strong faction ownership
		Location.OwningFactionId = ControllingFactionId;
		break;

	case ERegionType::FactionFrontier:
		// Frontier: mostly faction-owned, some independent
		if (RandStream.RandRange(1, 100) > 20)
		{
			Location.OwningFactionId = ControllingFactionId;
		}
		else
		{
			Location.OwningFactionId = -1; // Independent
		}
		break;

	case ERegionType::Neutral:
		// Neutral: independent ownership
		Location.OwningFactionId = -1;
		break;

	case ERegionType::Lawless:
		// Lawless: unowned or pirate-controlled (represented as -1)
		Location.OwningFactionId = -1;
		break;

	case ERegionType::Disputed:
		// Disputed: contested, often no clear owner
		if (RandStream.RandRange(1, 100) > 70)
		{
			Location.OwningFactionId = ControllingFactionId; // Still controlled by one side
		}
		else
		{
			Location.OwningFactionId = -1; // No clear owner
		}
		break;

	default:
		Location.OwningFactionId = -1;
		break;
	}
}

TArray<FResourceEntry> UUniverseGenerator::CreateProductionProfile(ELocationType LocationType, ERegionType RegionType, FRandomStream& RandStream)
{
	TArray<FResourceEntry> Production;

	switch (LocationType)
	{
	case ELocationType::MiningColony:
		Production.Add(FResourceEntry(EResourceType::Ore, RandStream.RandRange(100, 500)));
		Production.Add(FResourceEntry(EResourceType::RefinedMetals, RandStream.RandRange(10, 100)));
		break;

	case ELocationType::Shipyard:
		Production.Add(FResourceEntry(EResourceType::IndustrialParts, RandStream.RandRange(50, 200)));
		break;

	case ELocationType::ResearchFacility:
		Production.Add(FResourceEntry(EResourceType::Electronics, RandStream.RandRange(20, 100)));
		Production.Add(FResourceEntry(EResourceType::ResearchMaterials, RandStream.RandRange(10, 50)));
		break;

	case ELocationType::MilitaryBase:
		Production.Add(FResourceEntry(EResourceType::Weapons, RandStream.RandRange(50, 200)));
		break;

	case ELocationType::Colony:
		Production.Add(FResourceEntry(EResourceType::Food, RandStream.RandRange(100, 500)));
		Production.Add(FResourceEntry(EResourceType::Water, RandStream.RandRange(100, 500)));
		break;

	case ELocationType::TradeHub:
		// Trade hubs don't produce, they distribute
		break;

	case ELocationType::RefuelingDepot:
		Production.Add(FResourceEntry(EResourceType::Fuel, RandStream.RandRange(200, 1000)));
		break;

	default:
		// Generic station produces a little of everything
		Production.Add(FResourceEntry(EResourceType::IndustrialParts, RandStream.RandRange(10, 50)));
		break;
	}

	return Production;
}

TArray<FResourceEntry> UUniverseGenerator::CreateConsumptionProfile(ELocationType LocationType, int32 Population, FRandomStream& RandStream)
{
	TArray<FResourceEntry> Consumption;

	// Base consumption scales with population
	int32 FoodConsumption = Population / 100;
	int32 WaterConsumption = Population / 100;
	int32 FuelConsumption = Population / 500;

	// All locations consume basics
	if (Population > 0)
	{
		Consumption.Add(FResourceEntry(EResourceType::Food, FoodConsumption));
		Consumption.Add(FResourceEntry(EResourceType::Water, WaterConsumption));
		Consumption.Add(FResourceEntry(EResourceType::Fuel, FuelConsumption));
	}

	// Specific location types have additional needs
	switch (LocationType)
	{
	case ELocationType::Shipyard:
		Consumption.Add(FResourceEntry(EResourceType::Ore, RandStream.RandRange(100, 500)));
		Consumption.Add(FResourceEntry(EResourceType::RefinedMetals, RandStream.RandRange(50, 200)));
		Consumption.Add(FResourceEntry(EResourceType::Electronics, RandStream.RandRange(50, 200)));
		break;

	case ELocationType::ResearchFacility:
		Consumption.Add(FResourceEntry(EResourceType::ResearchMaterials, RandStream.RandRange(20, 100)));
		Consumption.Add(FResourceEntry(EResourceType::Electronics, RandStream.RandRange(30, 150)));
		break;

	case ELocationType::MilitaryBase:
		Consumption.Add(FResourceEntry(EResourceType::Weapons, RandStream.RandRange(50, 200)));
		Consumption.Add(FResourceEntry(EResourceType::Machinery, RandStream.RandRange(30, 150)));
		break;

	case ELocationType::TradeHub:
		// Trade hubs need consumer goods
		Consumption.Add(FResourceEntry(EResourceType::ConsumerGoods, RandStream.RandRange(50, 300)));
		break;

	default:
		// Generic consumption
		Consumption.Add(FResourceEntry(EResourceType::Machinery, RandStream.RandRange(10, 50)));
		break;
	}

	return Consumption;
}

void UUniverseGenerator::ValidateGeneration(const FUniverseData& Universe)
{
	UE_LOG(LogTemp, Log, TEXT("Validating generation..."));

	// Count region types
	int32 CoreCount = 0, FrontierCount = 0, NeutralCount = 0, LawlessCount = 0, DisputedCount = 0;
	int32 DisputedByInfluence = 0;
	int32 DisputedByBorder = 0;

	for (const FStarSystemData& System : Universe.Systems)
	{
		switch (System.RegionType)
		{
		case ERegionType::FactionCore: CoreCount++; break;
		case ERegionType::FactionFrontier: FrontierCount++; break;
		case ERegionType::Neutral: NeutralCount++; break;
		case ERegionType::Lawless: LawlessCount++; break;
		case ERegionType::Disputed: 
			DisputedCount++;
			// Check if it's a border dispute (adjacent to rival)
			bool bIsBorderDispute = false;
			for (int32 ConnectedId : System.ConnectedSystemIds)
			{
				const FStarSystemData& Connected = Universe.Systems[ConnectedId];
				if ((Connected.RegionType == ERegionType::FactionCore || 
					 Connected.RegionType == ERegionType::FactionFrontier) &&
					Connected.ControllingFactionId != -1)
				{
					bIsBorderDispute = true;
					break;
				}
			}
			if (bIsBorderDispute)
				DisputedByBorder++;
			else
				DisputedByInfluence++;
			break;
		}

		// Validate bidirectional connections
		for (int32 ConnectedId : System.ConnectedSystemIds)
		{
			const FStarSystemData& ConnectedSystem = Universe.Systems[ConnectedId];
			if (!ConnectedSystem.ConnectedSystemIds.Contains(System.SystemId))
			{
				UE_LOG(LogTemp, Error, TEXT("Connection %d -> %d is NOT bidirectional!"), 
					System.SystemId, ConnectedId);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Region Distribution:"));
	UE_LOG(LogTemp, Log, TEXT("  Faction Core: %d (%.1f%%)"), CoreCount, 100.0f * CoreCount / Universe.Systems.Num());
	UE_LOG(LogTemp, Log, TEXT("  Frontier: %d (%.1f%%)"), FrontierCount, 100.0f * FrontierCount / Universe.Systems.Num());
	UE_LOG(LogTemp, Log, TEXT("  Neutral: %d (%.1f%%)"), NeutralCount, 100.0f * NeutralCount / Universe.Systems.Num());
	UE_LOG(LogTemp, Log, TEXT("  Lawless: %d (%.1f%%)"), LawlessCount, 100.0f * LawlessCount / Universe.Systems.Num());
	UE_LOG(LogTemp, Log, TEXT("  Disputed: %d (%.1f%%)"), DisputedCount, 100.0f * DisputedCount / Universe.Systems.Num());
	if (DisputedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("    - By Influence Gap: %d"), DisputedByInfluence);
		UE_LOG(LogTemp, Log, TEXT("    - By Direct Border: %d"), DisputedByBorder);
	}

	// Check faction separation
	for (int32 i = 0; i < Universe.FactionHomeSystems.Num(); ++i)
	{
		for (int32 j = i + 1; j < Universe.FactionHomeSystems.Num(); ++j)
		{
			int32 Distance = CalculateJumpDistance(
				Universe.FactionHomeSystems[i],
				Universe.FactionHomeSystems[j],
				Universe.Systems
			);
			UE_LOG(LogTemp, Log, TEXT("  Faction %d <-> %d: %d jumps"), i, j, Distance);
		}
	}

	// Validate content generation (Sprint 2)
	UE_LOG(LogTemp, Log, TEXT("Validating system content..."));

	int32 SystemsWithoutLocations = 0;
	int32 TotalCelestialBodies = 0;
	int32 TotalLocations = 0;
	int64 TotalPopulation = 0;
	int32 InvalidOwnershipCount = 0;

	for (const FStarSystemData& System : Universe.Systems)
	{
		// Verify every system has at least one location (CRITICAL)
		if (System.Locations.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("System %d (%s) has NO locations!"), 
				System.SystemId, *System.SystemName);
			SystemsWithoutLocations++;
		}

		// Count content
		TotalCelestialBodies += System.CelestialBodies.Num();
		TotalLocations += System.Locations.Num();

		// Validate ownership references
		for (const FLocationData& Location : System.Locations)
		{
			TotalPopulation += Location.Population;

			// Check if ownership references are valid
			if (Location.OwningFactionId >= Universe.Config.FactionCount)
			{
				UE_LOG(LogTemp, Error, TEXT("Location %s has invalid faction ID: %d"), 
					*Location.LocationName, Location.OwningFactionId);
				InvalidOwnershipCount++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Content Statistics:"));
	UE_LOG(LogTemp, Log, TEXT("  Total Celestial Bodies: %d (avg %.1f per system)"), 
		TotalCelestialBodies, (float)TotalCelestialBodies / Universe.Systems.Num());
	UE_LOG(LogTemp, Log, TEXT("  Total Locations: %d (avg %.1f per system)"), 
		TotalLocations, (float)TotalLocations / Universe.Systems.Num());
	UE_LOG(LogTemp, Log, TEXT("  Total Population: %lld"), TotalPopulation);
	UE_LOG(LogTemp, Log, TEXT("  Systems without locations: %d"), SystemsWithoutLocations);
	UE_LOG(LogTemp, Log, TEXT("  Invalid ownership references: %d"), InvalidOwnershipCount);

	if (SystemsWithoutLocations > 0)
	{
		UE_LOG(LogTemp, Error, TEXT("VALIDATION FAILED: %d systems have no locations!"), SystemsWithoutLocations);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Content validation passed"));
	}
}

// Helper function implementations

FString UUniverseGenerator::GenerateSystemName(int32 Index, FRandomStream& RandStream)
{
	static const TArray<FString> Prefixes = {
		TEXT("Alpha"), TEXT("Beta"), TEXT("Gamma"), TEXT("Delta"), TEXT("Epsilon"),
		TEXT("Zeta"), TEXT("Eta"), TEXT("Theta"), TEXT("Iota"), TEXT("Kappa"),
		TEXT("Lambda"), TEXT("Mu"), TEXT("Nu"), TEXT("Xi"), TEXT("Omicron"),
		TEXT("Pi"), TEXT("Rho"), TEXT("Sigma"), TEXT("Tau"), TEXT("Upsilon"),
		TEXT("Phi"), TEXT("Chi"), TEXT("Psi"), TEXT("Omega"), TEXT("Nova"),
		TEXT("Proxima"), TEXT("Kepler"), TEXT("Sirius"), TEXT("Vega"), TEXT("Rigel")
	};

	static const TArray<FString> Suffixes = {
		TEXT("Prime"), TEXT("Secundus"), TEXT("Tertius"), TEXT("Majoris"), TEXT("Minoris"),
		TEXT("Centauri"), TEXT("Station"), TEXT("Outpost"), TEXT("Haven"), TEXT("Junction")
	};

	int32 PrefixIdx = RandStream.RandRange(0, Prefixes.Num() - 1);
	int32 SuffixIdx = RandStream.RandRange(0, Suffixes.Num() - 1);

	return FString::Printf(TEXT("%s %s %d"), *Prefixes[PrefixIdx], *Suffixes[SuffixIdx], Index);
}

FVector UUniverseGenerator::GenerateCoordinates(int32 Index, float GalacticRadius, FRandomStream& RandStream)
{
	// 2D circular distribution (top-down view like Stellaris)
	// Z is always 0 for proper 2D map display
	float Angle = RandStream.FRandRange(0.0f, 2.0f * PI);
	float Radius = RandStream.FRandRange(GalacticRadius * 0.2f, GalacticRadius);

	// Vary density - tighter near center, sparse at edges (more realistic galaxy)
	Radius = FMath::Pow(Radius / GalacticRadius, 0.7f) * GalacticRadius;

	float X = Radius * FMath::Cos(Angle);
	float Y = Radius * FMath::Sin(Angle);
	float Z = 0.0f; // Flat 2D plane

	return FVector(X, Y, Z);
}

int32 UUniverseGenerator::FindClosestSystem(const FVector& Position, const TArray<FStarSystemData>& Systems, const TSet<int32>& Exclude)
{
	int32 ClosestId = -1;
	float ClosestDistance = MAX_flt;

	for (const FStarSystemData& System : Systems)
	{
		if (Exclude.Contains(System.SystemId))
			continue;

		float Distance = GetDistance(Position, System.Coordinates);
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestId = System.SystemId;
		}
	}

	return ClosestId;
}

float UUniverseGenerator::GetDistance(const FVector& A, const FVector& B)
{
	// 2D distance only (ignore Z axis for top-down map)
	return FVector2D(A.X - B.X, A.Y - B.Y).Size();
}

TArray<int32> UUniverseGenerator::FindPath(int32 StartId, int32 EndId, const TArray<FStarSystemData>& Systems)
{
	// BFS pathfinding
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

		for (int32 ConnectedId : Systems[CurrentId].ConnectedSystemIds)
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

int32 UUniverseGenerator::CalculateJumpDistance(int32 SystemA, int32 SystemB, const TArray<FStarSystemData>& Systems)
{
	TArray<int32> Path = FindPath(SystemA, SystemB, Systems);
	return Path.Num() > 0 ? Path.Num() - 1 : MAX_int32;
}

FString UUniverseGenerator::GenerateCelestialBodyName(int32 SystemId, int32 BodyIndex, ECelestialBodyType BodyType, FRandomStream& RandStream)
{
	static const TArray<FString> StarNames = {
		TEXT("Sol"), TEXT("Helios"), TEXT("Astra"), TEXT("Stellaris"), TEXT("Lumen")
	};

	static const TArray<FString> PlanetPrefixes = {
		TEXT("Terra"), TEXT("Aqua"), TEXT("Pyro"), TEXT("Cryo"), TEXT("Verdant"),
		TEXT("Barren"), TEXT("Azure"), TEXT("Crimson"), TEXT("Amber"), TEXT("Obsidian")
	};

	static const TArray<FString> MoonNames = {
		TEXT("Luna"), TEXT("Selene"), TEXT("Phobos"), TEXT("Deimos"), TEXT("Io"),
		TEXT("Europa"), TEXT("Ganymede"), TEXT("Callisto"), TEXT("Titan"), TEXT("Enceladus")
	};

	FString BaseName;

	switch (BodyType)
	{
	case ECelestialBodyType::Star:
		BaseName = StarNames[RandStream.RandRange(0, StarNames.Num() - 1)];
		return FString::Printf(TEXT("%s-%d"), *BaseName, SystemId);

	case ECelestialBodyType::Moon:
		BaseName = MoonNames[RandStream.RandRange(0, MoonNames.Num() - 1)];
		return FString::Printf(TEXT("%s %d"), *BaseName, BodyIndex);

	case ECelestialBodyType::AsteroidField:
		return FString::Printf(TEXT("Asteroid Belt %d"), SystemId);

	case ECelestialBodyType::Anomaly:
		return FString::Printf(TEXT("Anomaly-%d-%d"), SystemId, BodyIndex);

	case ECelestialBodyType::GasGiant:
		BaseName = PlanetPrefixes[RandStream.RandRange(0, PlanetPrefixes.Num() - 1)];
		return FString::Printf(TEXT("%s Giant %d"), *BaseName, BodyIndex);

	case ECelestialBodyType::IceGiant:
		return FString::Printf(TEXT("Cryo Giant %d"), BodyIndex);

	default:
		// Planet, DwarfPlanet
		BaseName = PlanetPrefixes[RandStream.RandRange(0, PlanetPrefixes.Num() - 1)];
		return FString::Printf(TEXT("%s %d"), *BaseName, BodyIndex);
	}
}

FString UUniverseGenerator::GenerateLocationName(int32 SystemId, int32 LocationIndex, ELocationType LocationType, FRandomStream& RandStream)
{
	static const TArray<FString> StationPrefixes = {
		TEXT("Starport"), TEXT("Outpost"), TEXT("Hub"), TEXT("Station"), TEXT("Terminal"),
		TEXT("Gateway"), TEXT("Nexus"), TEXT("Haven"), TEXT("Port"), TEXT("Dock")
	};

	static const TArray<FString> ColonyNames = {
		TEXT("New Hope"), TEXT("Pioneer"), TEXT("Frontier"), TEXT("Settlement"), TEXT("Colony"),
		TEXT("Homestead"), TEXT("Outreach"), TEXT("Horizon"), TEXT("Haven"), TEXT("Landing")
	};

	static const TArray<FString> MilitaryNames = {
		TEXT("Fortress"), TEXT("Bastion"), TEXT("Stronghold"), TEXT("Citadel"), TEXT("Arsenal"),
		TEXT("Garrison"), TEXT("Command"), TEXT("Watch"), TEXT("Sentinel"), TEXT("Bulwark")
	};

	FString BaseName;

	switch (LocationType)
	{
	case ELocationType::Station:
		BaseName = StationPrefixes[RandStream.RandRange(0, StationPrefixes.Num() - 1)];
		return FString::Printf(TEXT("%s %d"), *BaseName, SystemId);

	case ELocationType::TradeHub:
		return FString::Printf(TEXT("Trade Hub %d"), SystemId);

	case ELocationType::MiningColony:
		return FString::Printf(TEXT("Mining Colony %d-%d"), SystemId, LocationIndex);

	case ELocationType::ResearchFacility:
		return FString::Printf(TEXT("Research Station %d"), SystemId);

	case ELocationType::MilitaryBase:
		BaseName = MilitaryNames[RandStream.RandRange(0, MilitaryNames.Num() - 1)];
		return FString::Printf(TEXT("%s %d"), *BaseName, SystemId);

	case ELocationType::PirateOutpost:
		return FString::Printf(TEXT("Pirate Den %d"), SystemId);

	case ELocationType::AbandonedFacility:
		return FString::Printf(TEXT("Derelict %d-%d"), SystemId, LocationIndex);

	case ELocationType::Shipyard:
		return FString::Printf(TEXT("Shipyard %d"), SystemId);

	case ELocationType::RefuelingDepot:
		return FString::Printf(TEXT("Fuel Depot %d"), SystemId);

	case ELocationType::Colony:
		BaseName = ColonyNames[RandStream.RandRange(0, ColonyNames.Num() - 1)];
		return FString::Printf(TEXT("%s %d"), *BaseName, SystemId);

	default:
		return FString::Printf(TEXT("Location %d-%d"), SystemId, LocationIndex);
	}
}

