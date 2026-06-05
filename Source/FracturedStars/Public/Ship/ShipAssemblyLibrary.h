// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Ship/ShipTypes.h"
#include "Ship/ShipData.h"
#include "ShipAssemblyLibrary.generated.h"

/**
 * Ship Assembly and Stat Calculation Library
 * 
 * Provides functions for:
 * - Calculating final ship stats from frame + components
 * - Validating component installations
 * - Ship assembly and component management
 * - Debug/testing utilities
 * 
 * Sprint 5.5: Foundation for component-based ship system
 */
UCLASS()
class FRACTUREDSTARS_API UShipAssemblyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// STAT CALCULATION
	// ========================================================================

	/**
	 * Calculate final ship stats from frame and installed components
	 * Call this whenever components are added/removed/damaged
	 * 
	 * @param FrameDef Ship frame definition
	 * @param InstalledComponents List of installed components
	 * @param ComponentDefinitions Map of component definitions (ComponentId -> Definition)
	 * @return Calculated stats structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Assembly")
	static FShipCalculatedStats CalculateShipStats(
		const FShipFrameDefinition& FrameDef,
		const TArray<FInstalledComponent>& InstalledComponents,
		const TMap<FName, FShipComponentDefinition>& ComponentDefinitions);

	/**
	 * Update ship data with recalculated stats
	 * Modifies ShipData in-place with new calculated values
	 * 
	 * @param ShipData Ship data to update (modified in-place)
	 * @param FrameDef Ship frame definition
	 * @param ComponentDefinitions Map of component definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Assembly")
	static void UpdateShipStats(
		UPARAM(ref) FShipData& ShipData,
		const FShipFrameDefinition& FrameDef,
		const TMap<FName, FShipComponentDefinition>& ComponentDefinitions);

	// ========================================================================
	// COMPONENT VALIDATION
	// ========================================================================

	/**
	 * Check if a component can be installed in a specific slot
	 * Validates slot type, size, and availability
	 * 
	 * @param SlotDef Slot definition
	 * @param ComponentDef Component to install
	 * @param OutReason Human-readable reason if validation fails
	 * @return True if component can be installed
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Assembly")
	static bool CanInstallComponent(
		const FComponentSlot& SlotDef,
		const FShipComponentDefinition& ComponentDef,
		FString& OutReason);

	/**
	 * Check if a ship has required components and is operational
	 * Required: Engine + Power Plant + valid power budget
	 * 
	 * @param Stats Calculated ship stats
	 * @param OutReason Human-readable status/reason
	 * @return True if ship is operational
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Assembly")
	static bool IsShipOperational(
		const FShipCalculatedStats& Stats,
		FString& OutReason);

	/**
	 * Validate power budget (generation >= consumption)
	 * 
	 * @param Stats Calculated ship stats
	 * @return True if power budget is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Assembly")
	static bool IsPowerBudgetValid(const FShipCalculatedStats& Stats);

	// ========================================================================
	// COMPONENT MANAGEMENT
	// ========================================================================

	/**
	 * Install a component into a ship
	 * 
	 * @param ShipData Ship data to modify
	 * @param SlotId Slot to install into
	 * @param ComponentDefId Component definition to install
	 * @param ComponentCondition Initial condition (0-100)
	 * @return True if installation succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Assembly")
	static bool InstallComponent(
		UPARAM(ref) FShipData& ShipData,
		FName SlotId,
		FName ComponentDefId,
		int32 ComponentCondition = 100);

	/**
	 * Remove a component from a ship
	 * 
	 * @param ShipData Ship data to modify
	 * @param SlotId Slot to remove component from
	 * @return Component definition ID that was removed (NAME_None if slot was empty)
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Assembly")
	static FName RemoveComponent(
		UPARAM(ref) FShipData& ShipData,
		FName SlotId);

	/**
	 * Get installed component in a specific slot
	 * 
	 * @param ShipData Ship data to query
	 * @param SlotId Slot to check
	 * @param OutComponent Installed component data (if found)
	 * @return True if slot has a component installed
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Assembly")
	static bool GetInstalledComponent(
		const FShipData& ShipData,
		FName SlotId,
		FInstalledComponent& OutComponent);

	// ========================================================================
	// DEBUG / TESTING
	// ========================================================================

	/**
	 * Create a test ship with specific frame and default components
	 * Used for testing and debugging
	 * 
	 * @param FrameId Frame to use
	 * @param ShipName Display name
	 * @param OwnerPlayerId Owner player
	 * @param SystemId Starting system
	 * @param LocationId Starting location
	 * @return Configured test ship data
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Debug")
	static FShipData CreateTestShip(
		FName FrameId,
		const FString& ShipName,
		int32 OwnerPlayerId,
		int32 SystemId,
		int32 LocationId);

	/**
	 * Print ship stats to log
	 * 
	 * @param ShipData Ship to print
	 * @param Stats Calculated stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Debug")
	static void PrintShipStats(
		const FShipData& ShipData,
		const FShipCalculatedStats& Stats);

	/**
	 * Print power usage breakdown to log
	 * 
	 * @param Stats Calculated stats
	 * @param InstalledComponents Installed components
	 * @param ComponentDefinitions Component definitions map
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Debug")
	static void PrintPowerUsage(
		const FShipCalculatedStats& Stats,
		const TArray<FInstalledComponent>& InstalledComponents,
		const TMap<FName, FShipComponentDefinition>& ComponentDefinitions);

	/**
	 * Print installed components list to log
	 * 
	 * @param ShipData Ship to print
	 * @param ComponentDefinitions Component definitions map
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Debug")
	static void PrintInstalledComponents(
		const FShipData& ShipData,
		const TMap<FName, FShipComponentDefinition>& ComponentDefinitions);

	/**
	 * Print ship economic data (components as goods, manufacturing costs)
	 * 
	 * @param FrameDef Ship frame
	 * @param InstalledComponents Installed components
	 * @param ComponentDefinitions Component definitions map
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship|Debug")
	static void PrintShipEconomyData(
		const FShipFrameDefinition& FrameDef,
		const TArray<FInstalledComponent>& InstalledComponents,
		const TMap<FName, FShipComponentDefinition>& ComponentDefinitions);

private:
	// ========================================================================
	// INTERNAL HELPERS
	// ========================================================================

	/**
	 * Get component type from slot type
	 */
	static EShipComponentType GetComponentTypeFromSlotType(ESlotType SlotType);

	/**
	 * Check if component type matches slot type
	 */
	static bool IsComponentTypeCompatible(ESlotType SlotType, EShipComponentType ComponentType);

	/**
	 * Calculate component efficiency based on condition
	 * Degraded components provide reduced stats
	 */
	static float CalculateComponentEfficiency(int32 CurrentCondition, int32 MaxCondition);
};
