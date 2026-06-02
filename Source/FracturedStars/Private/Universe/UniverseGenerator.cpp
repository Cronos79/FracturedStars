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
