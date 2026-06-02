// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Universe/UniverseTypes.h"
#include "UniverseGenerator.generated.h"

/**
 * Universe Generator
 * Generates deterministic universe structure from seed
 */
UCLASS()
class FRACTUREDSTARS_API UUniverseGenerator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Generate a complete universe from configuration
	 * @param Config - Generation parameters
	 * @return Generated universe data
	 */
	UFUNCTION(BlueprintCallable, Category = "Universe")
	static FUniverseData GenerateUniverse(const FUniverseConfig& Config);

private:
	// Generation steps
	static void GenerateSystems(FUniverseData& Universe, FRandomStream& RandStream);
	static void GenerateJumpNetwork(FUniverseData& Universe, FRandomStream& RandStream);
	static void PlaceFactionCores(FUniverseData& Universe, FRandomStream& RandStream);
	static void AssignRegions(FUniverseData& Universe);
	static void CalculateLawfulness(FUniverseData& Universe);
	static void EnsureTraversability(FUniverseData& Universe, FRandomStream& RandStream);
	static void ValidateGeneration(const FUniverseData& Universe);

	// Helper functions
	static FString GenerateSystemName(int32 Index, FRandomStream& RandStream);
	static FVector GenerateCoordinates(int32 Index, float GalacticRadius, FRandomStream& RandStream);
	static int32 FindClosestSystem(const FVector& Position, const TArray<FStarSystemData>& Systems, const TSet<int32>& Exclude);
	static float GetDistance(const FVector& A, const FVector& B);
	static void AddBidirectionalConnection(FStarSystemData& SystemA, FStarSystemData& SystemB);
	static bool AreSystemsConnected(const FStarSystemData& SystemA, const FStarSystemData& SystemB);
	static TArray<int32> FindPath(int32 StartId, int32 EndId, const TArray<FStarSystemData>& Systems);
	static int32 CalculateJumpDistance(int32 SystemA, int32 SystemB, const TArray<FStarSystemData>& Systems);
};
