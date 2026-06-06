// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Universe/UniverseTypes.h"
#include "FactionSubsystem.generated.h"

/**
 * Faction Subsystem (Sprint 7)
 * 
 * ARCHITECTURE RULE: This subsystem is a CONSUMER, not a creator.
 * 
 * Factions observe and react to:
 * - Economy (markets, shortages, production)
 * - Logistics (trade routes, fleet status)
 * - Universe (territory, systems, locations)
 * - Ship framework (component availability)
 * 
 * Factions DO NOT:
 * - Create parallel economy systems
 * - Manage their own markets/inventories
 * - Run custom logistics simulations
 * - Bypass existing ship/component mechanics
 * 
 * Sprint 7 Scope: Foundation only
 * - Gather economic data from existing systems
 * - Calculate faction-wide stress and dependencies
 * - Store future hooks (news, missions, diplomacy)
 * - NO gameplay implementation yet
 */
UCLASS()
class FRACTUREDSTARS_API UFactionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========== Subsystem Lifecycle ==========
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Initialize faction simulation (called after universe generation)
	 * @param UniverseData - Generated universe with faction data
	 */
	UFUNCTION(BlueprintCallable, Category = "Factions")
	void InitializeFactions(const FUniverseData& UniverseData);

	/**
	 * Tick faction analysis (separate from economy tick)
	 * @param DeltaTime - Time since last tick
	 */
	void TickFactions(float DeltaTime);

	/**
	 * Check if faction system is initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Factions")
	bool IsFactionSystemInitialized() const { return bIsInitialized; }

	// ========== Faction Data Access ==========

	/**
	 * Get faction data by ID
	 * @param FactionId - Faction identifier
	 * @param OutFaction - Faction data (output)
	 * @return True if faction exists
	 */
	UFUNCTION(BlueprintPure, Category = "Factions")
	bool GetFactionById(int32 FactionId, FFactionData& OutFaction) const;

	/**
	 * Get all factions
	 */
	UFUNCTION(BlueprintPure, Category = "Factions")
	TArray<FFactionData> GetAllFactions() const { return Factions; }

	/**
	 * Get faction controlling a system
	 * @param SystemId - System identifier
	 * @return FactionId or -1 if no controller
	 */
	UFUNCTION(BlueprintPure, Category = "Factions")
	int32 GetSystemControllingFaction(int32 SystemId) const;

	// ========== Debug & Diagnostics ==========

	/**
	 * Print faction summary to log
	 */
	UFUNCTION(BlueprintCallable, Category = "Factions|Debug")
	void PrintFactionSummary(int32 FactionId) const;

	/**
	 * Print faction economy status
	 */
	UFUNCTION(BlueprintCallable, Category = "Factions|Debug")
	void PrintFactionEconomy(int32 FactionId) const;

	/**
	 * Print faction dependencies
	 */
	UFUNCTION(BlueprintCallable, Category = "Factions|Debug")
	void PrintFactionDependencies(int32 FactionId) const;

	/**
	 * Print all factions overview
	 */
	UFUNCTION(BlueprintCallable, Category = "Factions|Debug")
	void PrintUniverseFactionReport() const;

private:
	// ========== Internal State ==========

	// Is faction system initialized?
	bool bIsInitialized = false;

	// Faction data (copied from UniverseData for local access)
	TArray<FFactionData> Factions;

	// Tick timer handle
	FTimerHandle FactionTickTimer;

	// Tick rate (seconds)
	float FactionTickRate = 10.0f; // Slower than economy tick

	// ========== Economic Awareness (Sprint 7) ==========

	/**
	 * Gather shortages across faction territory
	 * Queries UEconomySubsystem - does NOT create parallel data
	 */
	void GatherFactionShortages(int32 FactionId);

	/**
	 * Gather surpluses (export opportunities) across faction territory
	 * Queries UEconomySubsystem - does NOT create parallel data
	 */
	void GatherFactionSurpluses(int32 FactionId);

	/**
	 * Calculate faction-wide economic stress
	 * Aggregates shortage severity across territory
	 */
	void CalculateFactionEconomicStress(int32 FactionId);

	// ========== Dependency Tracking (Sprint 7) ==========

	/**
	 * Analyze import/export dependencies between factions
	 * Cross-references trade routes and market data
	 */
	void AnalyzeFactionDependencies(int32 FactionId);

	// ========== Ship Production Awareness (Sprint 7) ==========

	/**
	 * Evaluate ship production capability
	 * Checks component availability (engines, power plants, shields)
	 * Queries market data - does NOT create separate ship economy
	 */
	void EvaluateShipProductionCapability(int32 FactionId);

	// ========== Future Hooks (Sprint 7: data-only) ==========

	/**
	 * Trigger events based on faction analysis (internal helper)
	 * Generates news/mission hooks for high stress, dependencies, etc.
	 */
	void TriggerFactionEvents(int32 FactionId);

	/**
	 * Store notable economic events for future news generation
	 * Sprint 7: Just store strings, no news articles yet
	 */
	void RecordEconomicEvent(int32 FactionId, const FString& EventDescription);

	/**
	 * Store mission opportunities for future mission generation
	 * Sprint 7: Just store strings, no missions yet
	 */
	void RecordMissionOpportunity(int32 FactionId, const FString& OpportunityDescription);
};
