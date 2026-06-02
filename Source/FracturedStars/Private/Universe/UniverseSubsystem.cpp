// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/UniverseSubsystem.h"
#include "Universe/UniverseGenerator.h"
#include "Containers/Queue.h"

void UUniverseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Initialized"));
}

void UUniverseSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Deinitialized"));
}

bool UUniverseSubsystem::GenerateUniverse(const FUniverseConfig& Config)
{
	UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Generating universe..."));

	// Generate universe
	UniverseData = UUniverseGenerator::GenerateUniverse(Config);
	bIsGenerated = true;

	UE_LOG(LogTemp, Log, TEXT("UniverseSubsystem: Universe ready! (%d systems)"), UniverseData.Systems.Num());
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
