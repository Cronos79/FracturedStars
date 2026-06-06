// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LogisticsTypes.h"
#include "UniverseTypes.h"
#include "Ship/ShipData.h"
#include "Ship/ShipTypes.h"
#include "LogisticsSubsystem.generated.h"

/**
 * Logistics Subsystem
 * Manages NPC trade network, cargo ships, and physical goods movement
 * 
 * Sprint 6: NPC Logistics & Trade Network
 * 
 * Core Responsibilities:
 * - Generate trade requests from location shortages
 * - Detect export opportunities from surpluses
 * - Match importers with exporters to create trade routes
 * - Manage NPC logistics companies
 * - Simulate cargo ship movement through jump network
 * - Handle cargo loading and delivery
 * - Track trade statistics and performance
 * 
 * Core Principle: Goods never teleport. All movement is physical via ships.
 */
UCLASS()
class FRACTUREDSTARS_API ULogisticsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================================================
	// SUBSYSTEM LIFECYCLE
	// ========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Initialize logistics system after universe generation
	 * Creates starting logistics companies and initial fleet
	 * @param UniverseData - Reference to universe state
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void InitializeLogistics(const FUniverseData& UniverseData);

	/**
	 * Main logistics tick - called periodically to process trade network
	 * @param UniverseData - Reference to universe state
	 * @param DeltaTime - Real-world delta time (seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void TickLogistics(FUniverseData& UniverseData, float DeltaTime);

	// ========================================================================
	// TRADE REQUEST & EXPORT DETECTION
	// ========================================================================

	/**
	 * Generate trade requests from all locations with shortages
	 * Scans universe for locations needing goods
	 * @param UniverseData - Universe state to scan
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Trade")
	void GenerateTradeRequests(const FUniverseData& UniverseData);

	/**
	 * Generate export opportunities from locations with surpluses
	 * Scans universe for locations offering goods
	 * @param UniverseData - Universe state to scan
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Trade")
	void GenerateExportOpportunities(const FUniverseData& UniverseData);

	/**
	 * Clear old/stale trade requests and export opportunities
	 * Removes requests that have been fulfilled or are no longer valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Trade")
	void CleanupStaleTradeData(const FUniverseData& UniverseData);

	// ========================================================================
	// TRADE MATCHING & ROUTE GENERATION
	// ========================================================================

	/**
	 * Match trade requests with export opportunities
	 * Creates trade routes for profitable matches
	 * @param UniverseData - Universe state for pathfinding
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Trade")
	void MatchTrades(const FUniverseData& UniverseData);

	/**
	 * Generate a trade route between two locations
	 * Uses universe pathfinding to create jump path
	 * @param UniverseData - Universe state for pathfinding
	 * @param SourceSystemId - Export location system
	 * @param SourceLocationId - Export location ID
	 * @param DestSystemId - Import location system
	 * @param DestLocationId - Import location ID
	 * @param GoodType - Good being transported
	 * @param Quantity - Amount to transport
	 * @param EstimatedProfit - Expected profit
	 * @return Generated trade route (RouteId will be -1 if failed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Trade")
	FTradeRoute GenerateTradeRoute(
		const FUniverseData& UniverseData,
		int32 SourceSystemId,
		int32 SourceLocationId,
		int32 DestSystemId,
		int32 DestLocationId,
		EGoodType GoodType,
		int32 Quantity,
		float EstimatedProfit);

	// ========================================================================
	// LOGISTICS COMPANY MANAGEMENT
	// ========================================================================

	/**
	 * Get all logistics companies
	 */
	UFUNCTION(BlueprintPure, Category = "Logistics|Company")
	const TArray<FLogisticsCompany>& GetLogisticsCompanies() const { return LogisticsCompanies; }

	/**
	 * Get company by ID
	 * @param CompanyId - Company identifier
	 * @param OutCompany - Output company data
	 * @return True if company found
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Company")
	bool GetCompanyById(int32 CompanyId, FLogisticsCompany& OutCompany) const;

	/**
	 * Get company with most available ships (for route assignment)
	 * @return Company ID, or -1 if no companies available
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Company")
	int32 GetBestAvailableCompany() const;

	// ========================================================================
	// CARGO SHIP MANAGEMENT
	// ========================================================================

	/**
	 * Create a new cargo ship for a logistics company
	 * @param CompanyId - Owning company
	 * @param StartSystemId - Initial system location
	 * @param ShipName - Ship designation
	 * @param CargoCapacity - Cargo hold size
	 * @return Created ship ID, or -1 if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Ships")
	int32 CreateCargoShip(int32 CompanyId, int32 StartSystemId, const FString& ShipName, int32 CargoCapacity = 5000);

	/**
	 * Get ship by ID from fleet registry
	 * @param ShipId - Ship identifier
	 * @return Pointer to ship data, or nullptr if not found
	 */
	FShipData* GetShip(int32 ShipId)
	{
		return FleetRegistry.Find(ShipId);
	}

	/**
	 * Get ship by ID (const version)
	 */
	const FShipData* GetShip(int32 ShipId) const
	{
		return FleetRegistry.Find(ShipId);
	}

	/**
	 * Find an idle ship owned by a specific company
	 * @param CompanyId - Company to search
	 * @return Ship ID, or -1 if no idle ships available
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Ships")
	int32 FindIdleShip(int32 CompanyId) const;

	/**
	 * Find an idle ship near a specific system (Sprint 6: distributed fleet optimization)
	 * Prefers ships at or close to the source system to reduce empty transit
	 * @param CompanyId - Company to search
	 * @param SystemId - Target system (usually route source)
	 * @param UniverseData - Universe state for pathfinding
	 * @return Ship ID, or -1 if no idle ships available
	 */
	int32 FindIdleShipNearSystem(int32 CompanyId, int32 SystemId, const FUniverseData& UniverseData) const;

	/**
	 * Assign a ship to a trade route
	 * @param ShipId - Ship to assign
	 * @param RouteId - Route to assign to ship
	 * @return True if assignment successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Ships")
	bool AssignShipToRoute(int32 ShipId, int32 RouteId);

	// ========================================================================
	// CARGO OPERATIONS
	// ========================================================================

	/**
	 * Load cargo from source location onto ship
	 * Decreases source inventory, increases ship cargo
	 * @param UniverseData - Universe state to modify
	 * @param ShipId - Ship loading cargo
	 * @param RouteId - Associated trade route
	 * @return True if loading successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Cargo")
	bool LoadCargo(FUniverseData& UniverseData, int32 ShipId, int32 RouteId);

	/**
	 * Deliver cargo from ship to destination location
	 * Increases destination inventory, clears ship cargo
	 * @param UniverseData - Universe state to modify
	 * @param ShipId - Ship delivering cargo
	 * @param RouteId - Associated trade route
	 * @return True if delivery successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Cargo")
	bool DeliverCargo(FUniverseData& UniverseData, int32 ShipId, int32 RouteId);

	// ========================================================================
	// SHIP TRANSIT SIMULATION
	// ========================================================================

	/**
	 * Update all active ships in transit
	 * Processes ship movement through jump routes
	 * @param UniverseData - Universe state
	 * @param CurrentGameTime - Current universe time in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Ships")
	void TickShipTransit(FUniverseData& UniverseData, double CurrentGameTime);

	/**
	 * Move a ship to the next system in its route
	 * @param Ship - Ship to move (FShipData reference)
	 * @param Route - Associated trade route (mutable to track RouteProgress)
	 * @param CurrentGameTime - Current universe time
	 */
	void AdvanceShipAlongRoute(FShipData& Ship, FTradeRoute& Route, double CurrentGameTime);

	// ========================================================================
	// STATISTICS & REPORTING
	// ========================================================================

	/**
	 * Get current logistics statistics
	 */
	UFUNCTION(BlueprintPure, Category = "Logistics|Statistics")
	const FLogisticsStatistics& GetStatistics() const { return Statistics; }

	/**
	 * Update statistics after a successful delivery
	 * @param Route - Completed trade route
	 * @param DeliveryTime - Time taken to complete delivery (seconds)
	 */
	void UpdateStatistics(const FTradeRoute& Route, float DeliveryTime);

	/**
	 * Get top N trade routes by volume
	 * @param TopN - Number of routes to return
	 * @return Sorted array of route statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Statistics")
	TArray<FTradeRouteStatistics> GetTopTradeRoutes(int32 TopN = 10) const;

	/**
	 * Get top N most traded goods
	 * @param TopN - Number of goods to return
	 * @return Sorted array of good statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Statistics")
	TArray<FGoodTradeStatistics> GetMostTradedGoods(int32 TopN = 10) const;

	// ========================================================================
	// DEBUG & VALIDATION
	// ========================================================================

	/**
	 * Print all active trade requests
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintTradeRequests() const;

	/**
	 * Print all export opportunities
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintExportOpportunities() const;

	/**
	 * Print all active trade routes
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintActiveRoutes() const;

	/**
	 * Print logistics company statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintCompanyStatistics() const;

	/**
	 * Print cargo ship status and locations
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintCargoShipStatistics() const;

	/**
	 * Print economic dependencies (location import/export patterns)
	 * @param UniverseData - Universe state to analyze
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintEconomicDependencies(const FUniverseData& UniverseData) const;

	/**
	 * Print top importers by volume
	 * @param TopN - Number of locations to show
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintTopImporters(int32 TopN = 10) const;

	/**
	 * Print top exporters by volume
	 * @param TopN - Number of locations to show
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintTopExporters(int32 TopN = 10) const;

	/**
	 * Print most traded goods by volume
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintMostTradedGoods() const;

	/**
	 * Print comprehensive logistics system status
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintLogisticsSystemStatus() const;

	/**
	 * Print diagnostic information about economy/logistics health
	 * Helps identify why trades aren't happening
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics|Debug")
	void PrintSystemHealthDiagnostic() const;

protected:
	// ========================================================================
	// INTERNAL STATE
	// ========================================================================

	// Active trade requests (import demands)
	UPROPERTY()
	TArray<FTradeRequest> TradeRequests;

	// Active export opportunities (surplus offers)
	UPROPERTY()
	TArray<FExportOpportunity> ExportOpportunities;

	// All trade routes (pending, active, completed)
	UPROPERTY()
	TArray<FTradeRoute> TradeRoutes;

	// All logistics companies
	UPROPERTY()
	TArray<FLogisticsCompany> LogisticsCompanies;

	// Fleet registry: All cargo ships using real FShipData
	// Ships are owned by companies via FLogisticsCompany::OwnedShipIds
	UPROPERTY()
	TMap<int32, FShipData> FleetRegistry;

	// System-wide statistics
	UPROPERTY()
	FLogisticsStatistics Statistics;

	// Timer handle for periodic logistics tick
	FTimerHandle LogisticsTickTimer;

	// Next unique IDs for entities
	int32 NextRequestId = 0;
	int32 NextOpportunityId = 0;
	int32 NextRouteId = 0;
	int32 NextCompanyId = 0;
	int32 NextShipId = 0;

	// Configuration
	float LogisticsTickRate = 10.0f; // Seconds between logistics updates
	float JumpTimePerSystem = 3600.0f; // Game time seconds per jump (1 hour default)
	float LoadingTime = 1800.0f; // Game time seconds to load cargo (30 minutes default)
	float UnloadingTime = 1800.0f; // Game time seconds to unload cargo (30 minutes default)
	int32 TradeNetworkRadius = 3; // How many jumps from active system to catch up for trade
	double LastTradeNetworkCatchupTime = 0.0; // Track when we last did catch-up

	// Track which systems are caught up for trade network
	TSet<int32> TradeNetworkActiveSystems;

	// ========================================================================
	// INTERNAL HELPERS
	// ========================================================================

	/**
	 * Calculate trade request priority based on shortage severity
	 * @param Location - Location with shortage
	 * @param GoodType - Good in shortage
	 * @return Priority level
	 */
	ETradeRequestPriority CalculateRequestPriority(const FLocationData& Location, EGoodType GoodType) const;

	/**
	 * Calculate economic importance of a good
	 * Essential goods (food, water, medicine, fuel) = 1.0
	 * Industrial goods = 0.7
	 * Luxury goods = 0.3
	 */
	float CalculateEconomicImportance(EGoodType GoodType) const;

	/**
	 * Calculate estimated profit for a trade route
	 * @param ExportPrice - Price at source
	 * @param ImportPrice - Price at destination
	 * @param Quantity - Cargo quantity
	 * @param Distance - Jump distance
	 * @return Estimated profit in credits
	 */
	float CalculateEstimatedProfit(float ExportPrice, float ImportPrice, int32 Quantity, int32 Distance) const;

	/**
	 * Find best logistics hub systems for company home bases
	 * Prioritizes faction capitals, trade hubs, and high-population systems
	 * Sprint 6: Enables distributed fleet placement
	 * Sprint 7: Will become faction-owned infrastructure
	 * @param UniverseData - Universe state
	 * @param Count - Number of hub systems needed
	 * @return Array of system IDs suitable for logistics hubs
	 */
	TArray<int32> FindLogisticsHubSystems(const FUniverseData& UniverseData, int32 Count) const;

	/**
	 * Get the faction ID that controls a specific location
	 * Sprint 7: Used to enforce faction-only trade boundaries
	 * @param UniverseData - Universe state
	 * @param SystemId - System containing the location
	 * @param LocationId - Location within the system
	 * @return Faction ID, or -1 if uncontrolled/independent
	 */
	int32 GetLocationControllingFaction(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId) const;

	/**
	 * Get system name for logging
	 */
	FString GetSystemName(const FUniverseData& UniverseData, int32 SystemId) const;

	/**
	 * Get location name for logging
	 */
	FString GetLocationName(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId) const;

	/**
	 * Get ship frame base stats by frame ID
	 * Returns default freighter stats for now
	 * Future: will look up from frame definition registry
	 * @param FrameId - Frame identifier (e.g., "Freighter_Basic")
	 * @param OutCargoCapacity - Output cargo capacity
	 * @param OutFuelCapacity - Output fuel capacity
	 */
	void GetFrameBaseStats(FName FrameId, int32& OutCargoCapacity, int32& OutFuelCapacity) const;

	/**
	 * Catch up economy for systems within trade network radius
	 * Ensures nearby systems have active economies for trade opportunities
	 * @param UniverseData - Universe state to update
	 */
	void CatchUpTradeNetwork(FUniverseData& UniverseData);

	/**
	 * Find all systems within N jumps of active system
	 * @param UniverseData - Universe state for pathfinding
	 * @param CenterSystemId - System to search from
	 * @param MaxJumps - Maximum jump distance
	 * @return Set of system IDs within range
	 */
	TSet<int32> FindSystemsWithinRange(const FUniverseData& UniverseData, int32 CenterSystemId, int32 MaxJumps) const;
};
