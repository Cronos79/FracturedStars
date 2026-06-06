// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Universe/LogisticsSubsystem.h"
#include "Universe/UniverseSubsystem.h"
#include "Universe/EconomySubsystem.h"

// ============================================================================
// SUBSYSTEM LIFECYCLE
// ============================================================================

void ULogisticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Initialized"));
}

void ULogisticsSubsystem::Deinitialize()
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LogisticsTickTimer);
	}

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Deinitialized"));
}

void ULogisticsSubsystem::InitializeLogistics(const FUniverseData& UniverseData)
{
	UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Initializing logistics network..."));

	// Clear existing data
	TradeRequests.Empty();
	ExportOpportunities.Empty();
	TradeRoutes.Empty();
	LogisticsCompanies.Empty();
	FleetRegistry.Empty();
	Statistics = FLogisticsStatistics();

	// Reset ID counters
	NextRequestId = 0;
	NextOpportunityId = 0;
	NextRouteId = 0;
	NextCompanyId = 0;
	NextShipId = 0;

	// ========================================================================
	// SPRINT 6-7: FACTION-OWNED LOGISTICS FLEETS
	// Each faction gets ONE logistics company that ONLY serves that faction
	// Ships cannot be borrowed between factions - each must manage own capacity
	// ========================================================================

	// Create ONE logistics company per faction home system
	// In Sprint 7, these will be fully integrated with faction economy/budgets
	int32 NumFactions = UniverseData.FactionHomeSystems.Num();

	UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Creating faction-owned logistics fleets..."));
	UE_LOG(LogTemp, Log, TEXT("  Found %d faction home systems"), NumFactions);

	for (int32 FactionIdx = 0; FactionIdx < NumFactions; ++FactionIdx)
	{
		int32 HomeSystemId = UniverseData.FactionHomeSystems[FactionIdx];

		if (!UniverseData.Systems.IsValidIndex(HomeSystemId))
		{
			UE_LOG(LogTemp, Warning, TEXT("  Invalid faction home system ID: %d"), HomeSystemId);
			continue;
		}

		const FStarSystemData& HomeSystem = UniverseData.Systems[HomeSystemId];

		// Create faction logistics division
		FString CompanyName = FString::Printf(TEXT("%s Logistics Division"), *HomeSystem.SystemName);
		FLogisticsCompany Company(NextCompanyId++, CompanyName);

		// FACTION OWNERSHIP
		Company.OwningFactionId = FactionIdx; // Actual faction ID assignment
		Company.HomeSystemId = HomeSystemId;
		Company.Credits = 500000.0f; // Initial faction budget allocation

		LogisticsCompanies.Add(Company);

		// Fleet size based on faction home system importance
		// Major factions get more ships, minor factions get fewer
		int32 BaseShips = 3;
		int32 PopulationBonus = 0;

		// Count total population in home system
		int32 TotalPopulation = 0;
		for (const FLocationData& Location : HomeSystem.Locations)
		{
			TotalPopulation += Location.Population;
		}

		// Larger populations need bigger fleets
		if (TotalPopulation > 1000000)
			PopulationBonus = 2;
		else if (TotalPopulation > 500000)
			PopulationBonus = 1;

		int32 ShipsForFaction = BaseShips + PopulationBonus;

		// Create faction's ships at their home system
		for (int32 ShipNum = 0; ShipNum < ShipsForFaction; ++ShipNum)
		{
			FString ShipName = FString::Printf(TEXT("%s-FL-%d"), 
				*HomeSystem.SystemName.Left(3).ToUpper(), 
				ShipNum + 1);

			// Vary cargo capacity (3000-7000 units)
			int32 CargoCapacity = 3000 + (ShipNum * 1000) + FMath::RandRange(0, 1000);

			int32 ShipId = CreateCargoShip(Company.CompanyId, HomeSystemId, ShipName, CargoCapacity);

			if (ShipId != -1)
			{
				Company.ActiveShipCount++;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("  Faction %d: %s - %d ships deployed at %s (System %d)"),
			FactionIdx,
			*CompanyName,
			Company.ActiveShipCount,
			*HomeSystem.SystemName,
			HomeSystemId);
	}

	// Create independent logistics companies (neutral traders)
	// These handle cross-faction trade, contraband, lawless/neutral space trade
	// Smaller fleets but more flexible - can trade anywhere
	int32 IndependentCompanyCount = 2; // Create 2 independent traders

	for (int32 i = 0; i < IndependentCompanyCount; ++i)
	{
		FString IndependentName = FString::Printf(TEXT("Independent Traders %d"), i + 1);
		FLogisticsCompany IndependentCompany(NextCompanyId++, IndependentName);
		IndependentCompany.OwningFactionId = -1; // No faction affiliation
		IndependentCompany.HomeSystemId = -1; // No fixed home (operate from hubs)
		IndependentCompany.Credits = 300000.0f; // Smaller initial capital

		LogisticsCompanies.Add(IndependentCompany);

		// Independent traders get 2-3 ships each
		int32 IndependentShips = 2 + (i % 2);

		// Deploy ships across different neutral/trade hub systems
		for (int32 ShipNum = 0; ShipNum < IndependentShips; ++ShipNum)
		{
			// Find a good neutral hub system (high trade, neutral/lawless)
			int32 DeploySystemId = 0;
			for (int32 SysIdx = 0; SysIdx < UniverseData.Systems.Num(); ++SysIdx)
			{
				const FStarSystemData& Sys = UniverseData.Systems[SysIdx];
				if (Sys.RegionType == ERegionType::Neutral || Sys.RegionType == ERegionType::Lawless)
				{
					DeploySystemId = SysIdx;
					break;
				}
			}

			FString ShipName = FString::Printf(TEXT("IND-%d-FL-%d"), i + 1, ShipNum + 1);
			int32 CargoCapacity = 4000 + FMath::RandRange(0, 2000); // Medium capacity

			int32 ShipId = CreateCargoShip(IndependentCompany.CompanyId, DeploySystemId, ShipName, CargoCapacity);

			if (ShipId != -1)
			{
				IndependentCompany.ActiveShipCount++;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("  Independent: %s - %d ships (neutral trader)"),
			*IndependentName,
			IndependentCompany.ActiveShipCount);
	}

	// Sprint 6 fallback: If no factions exist yet, create one independent company for testing
	if (LogisticsCompanies.Num() == IndependentCompanyCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("  No factions found - creating additional test company"));

		FLogisticsCompany TestCompany(NextCompanyId++, TEXT("Independent Test Logistics"));
		TestCompany.OwningFactionId = -1; // Independent
		TestCompany.HomeSystemId = 0;
		TestCompany.Credits = 500000.0f;
		LogisticsCompanies.Add(TestCompany);

		// Create 3 test ships
		for (int32 i = 0; i < 3; ++i)
		{
			FString ShipName = FString::Printf(TEXT("TEST-SHIP-%d"), i + 1);
			int32 ShipId = CreateCargoShip(TestCompany.CompanyId, 0, ShipName, 5000);
			if (ShipId != -1)
			{
				TestCompany.ActiveShipCount++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Initialized %d companies (%d faction + %d independent) with %d total ships"), 
		LogisticsCompanies.Num(), LogisticsCompanies.Num() - IndependentCompanyCount, IndependentCompanyCount, FleetRegistry.Num());

	// Set up periodic tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LogisticsTickTimer,
			[this]()
			{
				// Get universe data from UniverseSubsystem
				if (UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>())
				{
					if (UniverseSys->IsUniverseGenerated())
					{
						FUniverseData& UniData = const_cast<FUniverseData&>(UniverseSys->GetUniverseData());
						TickLogistics(UniData, LogisticsTickRate);
					}
				}
			},
			LogisticsTickRate,
			true);

		UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Tick timer started (%.1fs interval)"), 
			LogisticsTickRate);
	}
}

void ULogisticsSubsystem::TickLogistics(FUniverseData& UniverseData, float DeltaTime)
{
	// Main logistics update loop
	// Called periodically to process trade network

	double CurrentGameTime = UniverseData.CurrentTime.TotalElapsedSeconds;

	// 0. Catch up trade network systems (ensure nearby systems are economically active)
	CatchUpTradeNetwork(UniverseData);

	// 1. Update ship transit
	TickShipTransit(UniverseData, CurrentGameTime);

	// 2. Clean up stale trade data
	CleanupStaleTradeData(UniverseData);

	// 3. Generate new trade requests and export opportunities
	GenerateTradeRequests(UniverseData);
	GenerateExportOpportunities(UniverseData);

	// 4. Match trades and create new routes
	MatchTrades(UniverseData);

	// 5. Update statistics
	Statistics.ActiveRouteCount = 0;
	for (const FTradeRoute& Route : TradeRoutes)
	{
		if (Route.Status == ETradeRouteStatus::Active)
		{
			Statistics.ActiveRouteCount++;
		}
	}

	Statistics.ActiveShipCount = 0;
	for (const auto& ShipPair : FleetRegistry)
	{
		if (ShipPair.Value.Status == EShipStatus::InTransit)
		{
			Statistics.ActiveShipCount++;
		}
	}
}

// ============================================================================
// LOGISTICS COMPANY MANAGEMENT
// ============================================================================

bool ULogisticsSubsystem::GetCompanyById(int32 CompanyId, FLogisticsCompany& OutCompany) const
{
	for (const FLogisticsCompany& Company : LogisticsCompanies)
	{
		if (Company.CompanyId == CompanyId)
		{
			OutCompany = Company;
			return true;
		}
	}
	return false;
}

int32 ULogisticsSubsystem::GetBestAvailableCompany() const
{
	int32 BestCompanyId = -1;
	int32 MostIdleShips = 0;

	for (const FLogisticsCompany& Company : LogisticsCompanies)
	{
		// Count docked ships for this company
		int32 IdleShips = 0;
		for (int32 ShipId : Company.OwnedShipIds)
		{
			const FShipData* Ship = FleetRegistry.Find(ShipId);
			if (Ship && Ship->Status == EShipStatus::Docked)
			{
				IdleShips++;
			}
		}

		if (IdleShips > MostIdleShips)
		{
			MostIdleShips = IdleShips;
			BestCompanyId = Company.CompanyId;
		}
	}

	return BestCompanyId;
}

// ============================================================================
// CARGO SHIP MANAGEMENT
// ============================================================================

int32 ULogisticsSubsystem::CreateCargoShip(int32 CompanyId, int32 StartSystemId, const FString& ShipName, int32 CargoCapacity)
{
	// Validate company exists and find it
	FLogisticsCompany* Company = nullptr;
	for (FLogisticsCompany& Comp : LogisticsCompanies)
	{
		if (Comp.CompanyId == CompanyId)
		{
			Company = &Comp;
			break;
		}
	}

	if (!Company)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Cannot create ship - company %d not found"), 
			CompanyId);
		return -1;
	}

	// Create FShipData using real ship framework
	int32 ShipId = NextShipId++;
	FShipData Ship;

	// Identity
	Ship.ShipId = ShipId;
	Ship.ShipName = ShipName;

	// Faction ownership (THIS IS KEY - ships belong to factions!)
	Ship.OwnerId = Company->OwningFactionId;
	Ship.OwnerType = FName(TEXT("Faction"));

	// Location (start at faction home system, docked)
	Ship.CurrentSystemId = StartSystemId;
	Ship.CurrentLocationId = -1; // -1 = space/generic dock
	Ship.Status = EShipStatus::Docked;

	// Frame & components - set frame type first
	Ship.FrameId = FName(TEXT("Freighter_Basic")); // Default medium freighter

	// Get cargo & fuel capacity from frame definition
	int32 FrameCargoCapacity = 0;
	int32 FrameFuelCapacity = 0;
	GetFrameBaseStats(Ship.FrameId, FrameCargoCapacity, FrameFuelCapacity);

	Ship.CargoCapacity = FrameCargoCapacity;
	Ship.CurrentCargoUsed = 0;
	Ship.CargoInventory.Empty();

	Ship.FuelCapacity = FrameFuelCapacity;
	Ship.CurrentFuel = FrameFuelCapacity;  // Start fully fueled

	// Ship operational state
	Ship.bIsOperational = true;
	Ship.HullCondition = 100.0f;

	// Add to fleet registry
	FleetRegistry.Add(ShipId, Ship);

	// Add ship ID to company's owned ships
	Company->OwnedShipIds.Add(ShipId);

	UE_LOG(LogTemp, Log, TEXT("    Created cargo ship: %s (ID: %d, Faction: %d, Frame: %s, Capacity: %d, System: %d)"),
		*ShipName, ShipId, Ship.OwnerId, *Ship.FrameId.ToString(), Ship.CargoCapacity, StartSystemId);

	return ShipId;
}



int32 ULogisticsSubsystem::FindIdleShip(int32 CompanyId) const
{
	// Find the company
	const FLogisticsCompany* Company = nullptr;
	for (const FLogisticsCompany& Comp : LogisticsCompanies)
	{
		if (Comp.CompanyId == CompanyId)
		{
			Company = &Comp;
			break;
		}
	}

	if (!Company)
		return -1;

	// Find any idle ship owned by this company
	for (int32 ShipId : Company->OwnedShipIds)
	{
		const FShipData* Ship = FleetRegistry.Find(ShipId);
		if (Ship && Ship->Status == EShipStatus::Docked && Ship->bIsOperational)
		{
			// Check if ship is actually idle (not assigned to a route)
			bool bIsAssigned = false;
			for (const FTradeRoute& Route : TradeRoutes)
			{
				if (Route.AssignedShipId == ShipId && 
					(Route.Status == ETradeRouteStatus::Active || Route.Status == ETradeRouteStatus::Pending))
				{
					bIsAssigned = true;
					break;
				}
			}

			if (!bIsAssigned)
			{
				return ShipId;
			}
		}
	}
	return -1;
}

int32 ULogisticsSubsystem::FindIdleShipNearSystem(int32 CompanyId, int32 SystemId, const FUniverseData& UniverseData) const
{
	// Smart ship selection - prefer ships at or near the source system
	// This reduces empty transit time and makes distributed fleets useful

	int32 BestShipId = -1;
	int32 BestDistance = INT_MAX;

	UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	if (!UniverseSys)
		return FindIdleShip(CompanyId); // Fallback

	// Find the company
	const FLogisticsCompany* Company = nullptr;
	for (const FLogisticsCompany& Comp : LogisticsCompanies)
	{
		if (Comp.CompanyId == CompanyId)
		{
			Company = &Comp;
			break;
		}
	}

	if (!Company)
		return -1;

	// Check all ships owned by this company
	for (int32 ShipId : Company->OwnedShipIds)
	{
		const FShipData* Ship = FleetRegistry.Find(ShipId);
		if (Ship && Ship->Status == EShipStatus::Docked && Ship->bIsOperational)
		{
			// Check if ship is idle (not assigned to active route)
			bool bIsAssigned = false;
			for (const FTradeRoute& Route : TradeRoutes)
			{
				if (Route.AssignedShipId == ShipId && 
					(Route.Status == ETradeRouteStatus::Active || Route.Status == ETradeRouteStatus::Pending))
				{
					bIsAssigned = true;
					break;
				}
			}

			if (!bIsAssigned)
			{
				// Calculate distance from ship to source system
				int32 Distance = UniverseSys->GetJumpDistance(Ship->CurrentSystemId, SystemId);

				if (Distance >= 0 && Distance < BestDistance)
				{
					BestDistance = Distance;
					BestShipId = ShipId;

					// Perfect match - ship already at source
					if (Distance == 0)
						break;
				}
			}
		}
	}

	return BestShipId;
}

bool ULogisticsSubsystem::AssignShipToRoute(int32 ShipId, int32 RouteId)
{
	// Find ship in fleet registry
	FShipData* Ship = FleetRegistry.Find(ShipId);

	if (!Ship)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Cannot assign ship %d to route %d - ship not found"),
			ShipId, RouteId);
		return false;
	}

	// Check if ship is operational and docked
	if (!Ship->bIsOperational || Ship->Status != EShipStatus::Docked)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Cannot assign ship %d - not operational or not docked"), 
			ShipId);
		return false;
	}

	// Find route
	FTradeRoute* Route = nullptr;
	for (FTradeRoute& R : TradeRoutes)
	{
		if (R.RouteId == RouteId)
		{
			Route = &R;
			break;
		}
	}

	if (!Route)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Cannot assign ship %d - route %d not found"), 
			ShipId, RouteId);
		return false;
	}

	// Assign ship to route
	// Note: FShipData doesn't track route assignment - that's managed by FTradeRoute
	Route->AssignedShipId = ShipId;
	Route->Status = ETradeRouteStatus::Active;

	UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Assigned ship %s (Faction %d) to route %d"), 
		*Ship->ShipName, Ship->OwnerId, RouteId);

	return true;
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

ETradeRequestPriority ULogisticsSubsystem::CalculateRequestPriority(const FLocationData& Location, EGoodType GoodType) const
{
	// Find market entry for this good
	for (const FMarketGoodEntry& Entry : Location.Market.Goods)
	{
		if (Entry.GoodType == GoodType)
		{
			// Calculate shortage severity
			float ShortagePercent = 0.0f;
			if (Entry.TargetStock > 0)
			{
				ShortagePercent = 1.0f - ((float)Entry.Stock / (float)Entry.TargetStock);
			}

			// Essential goods get priority boost
			bool bIsEssential = (GoodType == EGoodType::Food || 
								 GoodType == EGoodType::Water || 
								 GoodType == EGoodType::Medicine || 
								 GoodType == EGoodType::Fuel);

			if (bIsEssential)
			{
				if (ShortagePercent > 0.8f)
					return ETradeRequestPriority::Critical;
				else if (ShortagePercent > 0.5f)
					return ETradeRequestPriority::High;
				else if (ShortagePercent > 0.2f)
					return ETradeRequestPriority::Normal;
				else
					return ETradeRequestPriority::Low;
			}
			else
			{
				// Non-essential goods
				if (ShortagePercent > 0.9f)
					return ETradeRequestPriority::High;
				else if (ShortagePercent > 0.6f)
					return ETradeRequestPriority::Normal;
				else
					return ETradeRequestPriority::Low;
			}
		}
	}

	return ETradeRequestPriority::Normal;
}

float ULogisticsSubsystem::CalculateEconomicImportance(EGoodType GoodType) const
{
	// Essential survival goods
	if (GoodType == EGoodType::Food || 
		GoodType == EGoodType::Water || 
		GoodType == EGoodType::Medicine || 
		GoodType == EGoodType::Fuel)
	{
		return 1.0f;
	}

	// Industrial and ship-related goods
	if (GoodType == EGoodType::Machinery || 
		GoodType == EGoodType::Electronics || 
		GoodType == EGoodType::IndustrialParts ||
		GoodType == EGoodType::RefinedMetals ||
		GoodType == EGoodType::Ore)
	{
		return 0.7f;
	}

	// Advanced and research goods
	if (GoodType == EGoodType::AdvancedComponents || 
		GoodType == EGoodType::ResearchMaterials)
	{
		return 0.8f;
	}

	// Consumer goods and luxury
	if (GoodType == EGoodType::ConsumerGoods)
	{
		return 0.5f;
	}

	// Weapons and contraband
	if (GoodType == EGoodType::Weapons || 
		GoodType == EGoodType::Contraband)
	{
		return 0.4f;
	}

	// Ship frames and components (moderate importance)
	// These support ship production which is economically important
	// but not survival-critical
	return 0.6f;
}

TArray<int32> ULogisticsSubsystem::FindLogisticsHubSystems(const FUniverseData& UniverseData, int32 Count) const
{
	TArray<int32> HubSystems;

	// Priority 1: Faction home systems (ideal for company headquarters)
	for (int32 FactionHomeId : UniverseData.FactionHomeSystems)
	{
		if (FactionHomeId >= 0 && FactionHomeId < UniverseData.Systems.Num())
		{
			HubSystems.Add(FactionHomeId);
			if (HubSystems.Num() >= Count)
				return HubSystems;
		}
	}

	// Priority 2: Systems with Trade Hubs (major commerce centers)
	if (HubSystems.Num() < Count)
	{
		for (int32 SysIdx = 0; SysIdx < UniverseData.Systems.Num(); ++SysIdx)
		{
			if (HubSystems.Contains(SysIdx))
				continue;

			const FStarSystemData& System = UniverseData.Systems[SysIdx];
			for (const FLocationData& Location : System.Locations)
			{
				if (Location.LocationType == ELocationType::TradeHub)
				{
					HubSystems.Add(SysIdx);
					if (HubSystems.Num() >= Count)
						return HubSystems;
					break; // One hub per system is enough
				}
			}
		}
	}

	// Priority 3: High-population systems (economic activity)
	if (HubSystems.Num() < Count)
	{
		TArray<TPair<int32, int32>> SystemsByPop; // <SystemId, TotalPopulation>

		for (int32 SysIdx = 0; SysIdx < UniverseData.Systems.Num(); ++SysIdx)
		{
			if (HubSystems.Contains(SysIdx))
				continue;

			const FStarSystemData& System = UniverseData.Systems[SysIdx];
			int32 TotalPopulation = 0;
			for (const FLocationData& Location : System.Locations)
			{
				TotalPopulation += Location.Population;
			}

			if (TotalPopulation > 0)
			{
				SystemsByPop.Add(TPair<int32, int32>(SysIdx, TotalPopulation));
			}
		}

		// Sort by population descending
		SystemsByPop.Sort([](const TPair<int32, int32>& A, const TPair<int32, int32>& B) {
			return A.Value > B.Value;
		});

		// Take top systems
		for (const auto& Pair : SystemsByPop)
		{
			HubSystems.Add(Pair.Key);
			if (HubSystems.Num() >= Count)
				return HubSystems;
		}
	}

	// Priority 4: Fallback - any system with locations
	if (HubSystems.Num() < Count)
	{
		for (int32 SysIdx = 0; SysIdx < UniverseData.Systems.Num(); ++SysIdx)
		{
			if (HubSystems.Contains(SysIdx))
				continue;

			const FStarSystemData& System = UniverseData.Systems[SysIdx];
			if (System.Locations.Num() > 0)
			{
				HubSystems.Add(SysIdx);
				if (HubSystems.Num() >= Count)
					return HubSystems;
			}
		}
	}

	return HubSystems;
}

int32 ULogisticsSubsystem::GetLocationControllingFaction(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId) const
{
	// Sprint 7: Determine which faction controls a specific location
	// For now, use the system's controlling faction
	// Future: Locations could have individual faction ownership (contested systems, etc.)

	if (!UniverseData.Systems.IsValidIndex(SystemId))
		return -1;

	const FStarSystemData& System = UniverseData.Systems[SystemId];
	return System.ControllingFactionId; // -1 = independent/uncontrolled
}

float ULogisticsSubsystem::CalculateEstimatedProfit(float ExportPrice, float ImportPrice, int32 Quantity, int32 Distance) const
{
	// Revenue from sale at destination
	float Revenue = ImportPrice * Quantity;

	// Cost of purchase at source
	float PurchaseCost = ExportPrice * Quantity;

	// Operating costs (fuel, crew, maintenance)
	// Base cost + distance-based cost
	float OperatingCost = 1000.0f + (Distance * 200.0f);

	// Estimated profit
	float Profit = Revenue - PurchaseCost - OperatingCost;

	return Profit;
}

FString ULogisticsSubsystem::GetSystemName(const FUniverseData& UniverseData, int32 SystemId) const
{
	if (SystemId >= 0 && SystemId < UniverseData.Systems.Num())
	{
		return UniverseData.Systems[SystemId].SystemName;
	}
	return FString::Printf(TEXT("Unknown System %d"), SystemId);
}

FString ULogisticsSubsystem::GetLocationName(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId) const
{
	if (SystemId >= 0 && SystemId < UniverseData.Systems.Num())
	{
		const FStarSystemData& System = UniverseData.Systems[SystemId];
		if (LocationId >= 0 && LocationId < System.Locations.Num())
		{
			return System.Locations[LocationId].LocationName;
		}
	}
	return FString::Printf(TEXT("Unknown Location %d"), LocationId);
}

void ULogisticsSubsystem::GetFrameBaseStats(FName FrameId, int32& OutCargoCapacity, int32& OutFuelCapacity) const
{
	// Sprint 6: Hardcoded frame stats for common freighter types
	// Future: Look up from FShipFrameDefinition registry

	FString FrameIdStr = FrameId.ToString();

	if (FrameIdStr.Contains(TEXT("Freighter_Basic")) || FrameIdStr.Contains(TEXT("MediumFreighter")))
	{
		OutCargoCapacity = 5000;  // Medium freighter: 5000 units
		OutFuelCapacity = 1000;
	}
	else if (FrameIdStr.Contains(TEXT("Freighter_Heavy")) || FrameIdStr.Contains(TEXT("HeavyFreighter")))
	{
		OutCargoCapacity = 10000;  // Heavy freighter: 10000 units
		OutFuelCapacity = 1500;
	}
	else if (FrameIdStr.Contains(TEXT("Freighter_Light")) || FrameIdStr.Contains(TEXT("StarterTrade")))
	{
		OutCargoCapacity = 2000;  // Light/starter: 2000 units
		OutFuelCapacity = 500;
	}
	else
	{
		// Default fallback
		OutCargoCapacity = 5000;
		OutFuelCapacity = 1000;
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Unknown frame type '%s', using default stats"), *FrameIdStr);
	}
}

// ============================================================================
// TRADE REQUEST & EXPORT DETECTION
// ============================================================================

void ULogisticsSubsystem::GenerateTradeRequests(const FUniverseData& UniverseData)
{
	// Scan all locations for shortages and generate trade requests
	// Only create requests for goods that don't already have an active request

	int32 NewRequests = 0;

	for (const FStarSystemData& System : UniverseData.Systems)
	{
		for (int32 LocIdx = 0; LocIdx < System.Locations.Num(); ++LocIdx)
		{
			const FLocationData& Location = System.Locations[LocIdx];

			// Check each good in the market
			for (const FMarketGoodEntry& MarketEntry : Location.Market.Goods)
			{
				// Only process shortages
				if (!MarketEntry.bIsShortage)
					continue;

				// Check if we already have an active request for this location/good
				bool bHasActiveRequest = false;
				for (const FTradeRequest& Request : TradeRequests)
				{
					if (Request.SystemId == System.SystemId &&
						Request.LocationId == LocIdx &&
						Request.GoodType == MarketEntry.GoodType &&
						!Request.bIsMatched)
					{
						bHasActiveRequest = true;
						break;
					}
				}

				if (bHasActiveRequest)
					continue;

				// Calculate shortage quantity
				int32 ShortageAmount = FMath::Max(0, MarketEntry.TargetStock - MarketEntry.Stock);
				if (ShortageAmount <= 0)
					continue;

				// Create trade request
				FTradeRequest Request;
				Request.RequestId = NextRequestId++;
				Request.SystemId = System.SystemId;
				Request.LocationId = LocIdx;
				Request.GoodType = MarketEntry.GoodType;
				Request.QuantityNeeded = ShortageAmount;
				Request.MaxPrice = MarketEntry.CurrentPrice * 1.5f; // Willing to pay 50% premium
				Request.Priority = CalculateRequestPriority(Location, MarketEntry.GoodType);
				Request.RequestAge = 0.0;
				Request.EconomicImportance = CalculateEconomicImportance(MarketEntry.GoodType);
				Request.bIsMatched = false;

				TradeRequests.Add(Request);
				NewRequests++;
			}
		}
	}

	if (NewRequests > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Generated %d new trade requests (total: %d)"),
			NewRequests, TradeRequests.Num());
	}
}

void ULogisticsSubsystem::GenerateExportOpportunities(const FUniverseData& UniverseData)
{
	// Scan all locations for surpluses and generate export opportunities
	// Only create opportunities for goods that don't already have one

	int32 NewOpportunities = 0;

	for (const FStarSystemData& System : UniverseData.Systems)
	{
		for (int32 LocIdx = 0; LocIdx < System.Locations.Num(); ++LocIdx)
		{
			const FLocationData& Location = System.Locations[LocIdx];

			// Check each good in the market
			for (const FMarketGoodEntry& MarketEntry : Location.Market.Goods)
			{
				// Only process surpluses
				if (!MarketEntry.bIsSurplus)
					continue;

				// Check if we already have an active opportunity for this location/good
				bool bHasActiveOpportunity = false;
				for (const FExportOpportunity& Opp : ExportOpportunities)
				{
					if (Opp.SystemId == System.SystemId &&
						Opp.LocationId == LocIdx &&
						Opp.GoodType == MarketEntry.GoodType)
					{
						bHasActiveOpportunity = true;
						break;
					}
				}

				if (bHasActiveOpportunity)
					continue;

				// Calculate surplus quantity available for export
				int32 SurplusAmount = FMath::Max(0, MarketEntry.Stock - MarketEntry.TargetStock);
				if (SurplusAmount <= 0)
					continue;

				// Don't export everything - keep some buffer
				int32 ExportableQuantity = FMath::FloorToInt(SurplusAmount * 0.8f);
				if (ExportableQuantity <= 10) // Minimum threshold
					continue;

				// Create export opportunity
				FExportOpportunity Opportunity;
				Opportunity.OpportunityId = NextOpportunityId++;
				Opportunity.SystemId = System.SystemId;
				Opportunity.LocationId = LocIdx;
				Opportunity.GoodType = MarketEntry.GoodType;
				Opportunity.QuantityAvailable = ExportableQuantity;
				Opportunity.MarketPrice = MarketEntry.CurrentPrice;
				Opportunity.QuantityReserved = 0;
				Opportunity.ExportPriority = (float)SurplusAmount / (float)MarketEntry.TargetStock;

				ExportOpportunities.Add(Opportunity);
				NewOpportunities++;
			}
		}
	}

	if (NewOpportunities > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Generated %d new export opportunities (total: %d)"),
			NewOpportunities, ExportOpportunities.Num());
	}
}

void ULogisticsSubsystem::CleanupStaleTradeData(const FUniverseData& UniverseData)
{
	// Remove trade requests and export opportunities that are no longer valid

	int32 RemovedRequests = 0;
	int32 RemovedOpportunities = 0;

	// Clean up trade requests
	for (int32 i = TradeRequests.Num() - 1; i >= 0; --i)
	{
		const FTradeRequest& Request = TradeRequests[i];

		// Skip matched requests (they're tied to active routes)
		if (Request.bIsMatched)
			continue;

		// Check if location still has shortage
		bool bStillValid = false;
		if (Request.SystemId >= 0 && Request.SystemId < UniverseData.Systems.Num())
		{
			const FStarSystemData& System = UniverseData.Systems[Request.SystemId];
			if (Request.LocationId >= 0 && Request.LocationId < System.Locations.Num())
			{
				const FLocationData& Location = System.Locations[Request.LocationId];
				for (const FMarketGoodEntry& Entry : Location.Market.Goods)
				{
					if (Entry.GoodType == Request.GoodType && Entry.bIsShortage)
					{
						bStillValid = true;
						break;
					}
				}
			}
		}

		if (!bStillValid)
		{
			TradeRequests.RemoveAt(i);
			RemovedRequests++;
		}
	}

	// Clean up export opportunities
	for (int32 i = ExportOpportunities.Num() - 1; i >= 0; --i)
	{
		const FExportOpportunity& Opp = ExportOpportunities[i];

		// Check if location still has surplus
		bool bStillValid = false;
		if (Opp.SystemId >= 0 && Opp.SystemId < UniverseData.Systems.Num())
		{
			const FStarSystemData& System = UniverseData.Systems[Opp.SystemId];
			if (Opp.LocationId >= 0 && Opp.LocationId < System.Locations.Num())
			{
				const FLocationData& Location = System.Locations[Opp.LocationId];
				for (const FMarketGoodEntry& Entry : Location.Market.Goods)
				{
					if (Entry.GoodType == Opp.GoodType && Entry.bIsSurplus)
					{
						bStillValid = true;
						break;
					}
				}
			}
		}

		if (!bStillValid)
		{
			ExportOpportunities.RemoveAt(i);
			RemovedOpportunities++;
		}
	}

	if (RemovedRequests > 0 || RemovedOpportunities > 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("LogisticsSubsystem: Cleaned up %d stale requests, %d stale opportunities"),
			RemovedRequests, RemovedOpportunities);
	}
}

// ============================================================================
// TRADE MATCHING & ROUTE GENERATION
// ============================================================================

void ULogisticsSubsystem::MatchTrades(const FUniverseData& UniverseData)
{
	// Match trade requests with export opportunities to create profitable trade routes
	// Priority: Critical requests first, then by economic importance and profitability

	int32 NewRoutes = 0;

	// Sort trade requests by priority and importance
	TArray<FTradeRequest*> SortedRequests;
	for (FTradeRequest& Request : TradeRequests)
	{
		if (!Request.bIsMatched)
		{
			SortedRequests.Add(&Request);
		}
	}

	SortedRequests.Sort([](const FTradeRequest& A, const FTradeRequest& B)
	{
		// Critical priority first
		if (A.Priority != B.Priority)
			return (int32)A.Priority > (int32)B.Priority;

		// Then economic importance
		if (A.EconomicImportance != B.EconomicImportance)
			return A.EconomicImportance > B.EconomicImportance;

		// Then age (older requests first)
		return A.RequestAge > B.RequestAge;
	});

	// Process each request
	for (FTradeRequest* Request : SortedRequests)
	{
		// Find matching export opportunities
		TArray<FExportOpportunity*> MatchingExports;
		for (FExportOpportunity& Opp : ExportOpportunities)
		{
			if (Opp.GoodType == Request->GoodType &&
				(Opp.QuantityAvailable - Opp.QuantityReserved) > 0)
			{
				// Don't export from same location
				if (Opp.SystemId == Request->SystemId && Opp.LocationId == Request->LocationId)
					continue;

				// CROSS-FACTION TRADE ENABLED
				// Core vision: Economy drives everything. Earth needs Mars engines,
				// Mars needs Earth food, medicine comes from specialized factions, etc.
				// 
				// Future enhancement: Add faction relations/embargo system
				// to restrict trade based on diplomacy (war, sanctions, etc.)
				// For now, all trade is allowed if economically viable.

				MatchingExports.Add(&Opp);
			}
		}

		if (MatchingExports.Num() == 0)
			continue;

		// Find best export opportunity (by profitability)
		FExportOpportunity* BestExport = nullptr;
		float BestProfit = -999999.0f;
		int32 BestDistance = 0;

		for (FExportOpportunity* Opp : MatchingExports)
		{
			// Calculate route distance (will use pathfinding)
			// For now, use a temporary lookup via UniverseSubsystem
			int32 Distance = 0;
			if (UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>())
			{
				Distance = UniverseSys->GetJumpDistance(Opp->SystemId, Request->SystemId);
			}

			if (Distance <= 0)
				continue; // No path

			// Calculate available quantity for this trade
			int32 AvailableQuantity = Opp->QuantityAvailable - Opp->QuantityReserved;
			int32 TradeQuantity = FMath::Min(AvailableQuantity, Request->QuantityNeeded);

			// Calculate profit
			float Profit = CalculateEstimatedProfit(
				Opp->MarketPrice,
				Request->MaxPrice,
				TradeQuantity,
				Distance);

			// Prefer profitable routes, but allow critical requests even at slight loss
			bool bAcceptable = (Profit > 0.0f) || 
							   (Request->Priority == ETradeRequestPriority::Critical && Profit > -5000.0f);

			if (bAcceptable && Profit > BestProfit)
			{
				BestProfit = Profit;
				BestExport = Opp;
				BestDistance = Distance;
			}
		}

		if (!BestExport)
			continue;

		// Find logistics company to serve this route
		// Priority: Export faction's company (closest to source) > Any faction with idle ships > Independent traders
		// This enables cross-faction trade while keeping faction fleets as primary trade mechanism

		int32 ExportFactionId = GetLocationControllingFaction(UniverseData, BestExport->SystemId, BestExport->LocationId);
		int32 RequestFactionId = GetLocationControllingFaction(UniverseData, Request->SystemId, Request->LocationId);

		// Try 1: Exporting faction's company (most efficient - ship starts near cargo)
		int32 CompanyId = -1;
		for (const FLogisticsCompany& Company : LogisticsCompanies)
		{
			if (Company.OwningFactionId == ExportFactionId)
			{
				CompanyId = Company.CompanyId;
				break;
			}
		}

		// Try 2: Importing faction's company (second choice)
		if (CompanyId == -1 && RequestFactionId != ExportFactionId)
		{
			for (const FLogisticsCompany& Company : LogisticsCompanies)
			{
				if (Company.OwningFactionId == RequestFactionId)
				{
					CompanyId = Company.CompanyId;
					break;
				}
			}
		}

		// Try 3: Any faction or independent trader with idle ships
		if (CompanyId == -1)
		{
			CompanyId = GetBestAvailableCompany();
		}

		if (CompanyId == -1)
		{
			// No logistics company has available ships
			UE_LOG(LogTemp, Verbose, TEXT("LogisticsSubsystem: No available ships for trade - all fleets busy"));
			continue;
		}

		// Calculate trade quantity
		int32 AvailableQuantity = BestExport->QuantityAvailable - BestExport->QuantityReserved;
		int32 TradeQuantity = FMath::Min(AvailableQuantity, Request->QuantityNeeded);

		// Create trade route
		FTradeRoute Route = GenerateTradeRoute(
			UniverseData,
			BestExport->SystemId,
			BestExport->LocationId,
			Request->SystemId,
			Request->LocationId,
			Request->GoodType,
			TradeQuantity,
			BestProfit);

		if (Route.RouteId == -1)
		{
			UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Failed to generate route for request %d"), 
				Request->RequestId);
			continue;
		}

		// Find ship near the route source from the selected company
		int32 ShipId = FindIdleShipNearSystem(CompanyId, Route.SourceSystemId, UniverseData);
		if (ShipId == -1)
		{
			// Company has no idle ships - trade must wait
			UE_LOG(LogTemp, Verbose, TEXT("LogisticsSubsystem: Company %d has no idle ships - trade delayed"), 
				CompanyId);
			continue;
		}

		if (!AssignShipToRoute(ShipId, Route.RouteId))
		{
			UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Failed to assign ship %d to route %d"), 
				ShipId, Route.RouteId);
			continue;
		}

		// Mark request as matched
		Request->bIsMatched = true;

		// Reserve export quantity
		BestExport->QuantityReserved += TradeQuantity;

		NewRoutes++;

		UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Created route %d: %s -> %s (%s x%d, profit: %.0f)"),
			Route.RouteId,
			*GetLocationName(UniverseData, Route.SourceSystemId, Route.SourceLocationId),
			*GetLocationName(UniverseData, Route.DestinationSystemId, Route.DestinationLocationId),
			*UEnum::GetValueAsString(Route.GoodType),
			Route.CargoQuantity,
			Route.EstimatedProfit);
	}

	if (NewRoutes > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Created %d new trade routes"), NewRoutes);
	}
}

FTradeRoute ULogisticsSubsystem::GenerateTradeRoute(
	const FUniverseData& UniverseData,
	int32 SourceSystemId,
	int32 SourceLocationId,
	int32 DestSystemId,
	int32 DestLocationId,
	EGoodType GoodType,
	int32 Quantity,
	float EstimatedProfit)
{
	FTradeRoute Route;

	// Find path using universe pathfinding
	TArray<int32> JumpPath;
	if (UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>())
	{
		JumpPath = UniverseSys->FindPath(SourceSystemId, DestSystemId);
	}

	if (JumpPath.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: No path found from system %d to %d"),
			SourceSystemId, DestSystemId);
		return Route; // Invalid route (RouteId = -1)
	}

	// Create route
	Route.RouteId = NextRouteId++;
	Route.SourceSystemId = SourceSystemId;
	Route.SourceLocationId = SourceLocationId;
	Route.DestinationSystemId = DestSystemId;
	Route.DestinationLocationId = DestLocationId;
	Route.GoodType = GoodType;
	Route.CargoQuantity = Quantity;
	Route.JumpPath = JumpPath;
	Route.EstimatedProfit = EstimatedProfit;
	Route.RouteDistance = JumpPath.Num() - 1; // Number of jumps
	Route.EstimatedTransitTime = (Route.RouteDistance * JumpTimePerSystem) + LoadingTime + UnloadingTime;
	Route.Status = ETradeRouteStatus::Pending;
	Route.AssignedCompanyId = -1;
	Route.AssignedShipId = -1;
	Route.CreationTime = UniverseData.CurrentTime.TotalElapsedSeconds;
	Route.CompletionTime = 0.0;

	// Add to routes list
	TradeRoutes.Add(Route);

	return Route;
}

// ============================================================================
// CARGO OPERATIONS
// ============================================================================

bool ULogisticsSubsystem::LoadCargo(FUniverseData& UniverseData, int32 ShipId, int32 RouteId)
{
	// Find ship in fleet registry
	FShipData* Ship = FleetRegistry.Find(ShipId);
	if (!Ship)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: LoadCargo - Ship %d not found"), ShipId);
		return false;
	}

	// Find route
	FTradeRoute* Route = nullptr;
	for (FTradeRoute& R : TradeRoutes)
	{
		if (R.RouteId == RouteId)
		{
			Route = &R;
			break;
		}
	}

	if (!Route)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: LoadCargo - Route %d not found"), RouteId);
		return false;
	}

	// Validate ship is at source location
	if (Ship->CurrentSystemId != Route->SourceSystemId)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Ship %d not at source system %d (currently at %d)"),
			ShipId, Route->SourceSystemId, Ship->CurrentSystemId);
		return false;
	}

	// Get source location
	if (Route->SourceSystemId < 0 || Route->SourceSystemId >= UniverseData.Systems.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("LogisticsSubsystem: Invalid source system %d"), Route->SourceSystemId);
		return false;
	}

	FStarSystemData& SourceSystem = UniverseData.Systems[Route->SourceSystemId];
	if (Route->SourceLocationId < 0 || Route->SourceLocationId >= SourceSystem.Locations.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("LogisticsSubsystem: Invalid source location %d"), Route->SourceLocationId);
		return false;
	}

	FLocationData& SourceLocation = SourceSystem.Locations[Route->SourceLocationId];

	// Find market entry for the good
	FMarketGoodEntry* MarketEntry = nullptr;
	for (FMarketGoodEntry& Entry : SourceLocation.Market.Goods)
	{
		if (Entry.GoodType == Route->GoodType)
		{
			MarketEntry = &Entry;
			break;
		}
	}

	if (!MarketEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Good type not found in source market"));
		return false;
	}

	// Check if enough stock is available
	int32 QuantityToLoad = FMath::Min(Route->CargoQuantity, MarketEntry->Stock);
	if (QuantityToLoad <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: No stock available at source location"));
		return false;
	}

	// Check ship capacity (FShipData tracks CurrentCargoUsed and CargoCapacity)
	int32 AvailableCapacity = Ship->CargoCapacity - Ship->CurrentCargoUsed;
	if (AvailableCapacity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Ship has no cargo capacity"));
		return false;
	}

	if (QuantityToLoad > AvailableCapacity)
	{
		// Partial load - take what fits
		QuantityToLoad = AvailableCapacity;
		UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Partial load - ship capacity limit reached"));
	}

	// Update source inventory (decrease stock)
	MarketEntry->Stock -= QuantityToLoad;

	// Update ship cargo inventory (FShipData.CargoInventory is TMap<EGoodType, int32>)
	int32& CargoAmount = Ship->CargoInventory.FindOrAdd(Route->GoodType, 0);
	CargoAmount += QuantityToLoad;
	Ship->CurrentCargoUsed += QuantityToLoad;

	// Update route if partial load
	if (QuantityToLoad < Route->CargoQuantity)
	{
		Route->CargoQuantity = QuantityToLoad;
	}

	// Update ship status and location
	Ship->Status = EShipStatus::InTransit;
	Ship->CurrentLocationId = Route->SourceLocationId;

	UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Ship %s (Faction %d) loaded %d units of %s from %s"),
		*Ship->ShipName,
		Ship->OwnerId,
		QuantityToLoad,
		*UEnum::GetValueAsString(Route->GoodType),
		*GetLocationName(UniverseData, Route->SourceSystemId, Route->SourceLocationId));

	return true;
}

bool ULogisticsSubsystem::DeliverCargo(FUniverseData& UniverseData, int32 ShipId, int32 RouteId)
{
	// Find ship in fleet registry
	FShipData* Ship = FleetRegistry.Find(ShipId);
	if (!Ship)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: DeliverCargo - Ship %d not found"), ShipId);
		return false;
	}

	// Find route
	FTradeRoute* Route = nullptr;
	for (FTradeRoute& R : TradeRoutes)
	{
		if (R.RouteId == RouteId)
		{
			Route = &R;
			break;
		}
	}

	if (!Route)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: DeliverCargo - Route %d not found"), RouteId);
		return false;
	}

	// Validate ship is at destination location
	if (Ship->CurrentSystemId != Route->DestinationSystemId)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Ship %d not at destination system %d (currently at %d)"),
			ShipId, Route->DestinationSystemId, Ship->CurrentSystemId);
		return false;
	}

	// Get destination location
	if (Route->DestinationSystemId < 0 || Route->DestinationSystemId >= UniverseData.Systems.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("LogisticsSubsystem: Invalid destination system %d"), Route->DestinationSystemId);
		return false;
	}

	FStarSystemData& DestSystem = UniverseData.Systems[Route->DestinationSystemId];
	if (Route->DestinationLocationId < 0 || Route->DestinationLocationId >= DestSystem.Locations.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("LogisticsSubsystem: Invalid destination location %d"), Route->DestinationLocationId);
		return false;
	}

	FLocationData& DestLocation = DestSystem.Locations[Route->DestinationLocationId];

	// Find cargo in ship inventory (FShipData.CargoInventory is TMap<EGoodType, int32>)
	int32* CargoPtr = Ship->CargoInventory.Find(Route->GoodType);
	if (!CargoPtr || *CargoPtr <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LogisticsSubsystem: Ship %d has no cargo of type %s"),
			ShipId, *UEnum::GetValueAsString(Route->GoodType));
		return false;
	}

	int32 CargoQuantity = *CargoPtr;

	// Remove cargo from ship
	Ship->CargoInventory.Remove(Route->GoodType);
	Ship->CurrentCargoUsed -= CargoQuantity;

	// Find or create market entry for the good
	FMarketGoodEntry* MarketEntry = nullptr;
	for (FMarketGoodEntry& Entry : DestLocation.Market.Goods)
	{
		if (Entry.GoodType == Route->GoodType)
		{
			MarketEntry = &Entry;
			break;
		}
	}

	if (!MarketEntry)
	{
		// Create new market entry if doesn't exist
		FMarketGoodEntry NewEntry(Route->GoodType, 0, 10.0f);
		DestLocation.Market.Goods.Add(NewEntry);
		MarketEntry = &DestLocation.Market.Goods.Last();
	}

	// Update destination inventory (increase stock)
	MarketEntry->Stock += CargoQuantity;

	// Recalculate shortage/surplus flags
	// Note: Full price/shortage recalculation happens in EconomySubsystem
	// but we update the shortage flag here for immediate feedback
	if (MarketEntry->Stock < MarketEntry->TargetStock * 0.5f)
	{
		MarketEntry->bIsShortage = true;
		MarketEntry->bIsSurplus = false;
	}
	else if (MarketEntry->Stock > MarketEntry->TargetStock * 1.5f)
	{
		MarketEntry->bIsShortage = false;
		MarketEntry->bIsSurplus = true;
	}
	else
	{
		MarketEntry->bIsShortage = false;
		MarketEntry->bIsSurplus = false;
	}

	// Update ship status
	Ship->Status = EShipStatus::Docked;
	Ship->CurrentLocationId = Route->DestinationLocationId;

	// Update route status
	Route->Status = ETradeRouteStatus::Completed;
	Route->CompletionTime = UniverseData.CurrentTime.TotalElapsedSeconds;

	// Calculate actual delivery time
	float DeliveryTime = static_cast<float>(Route->CompletionTime - Route->CreationTime);

	// Update statistics
	UpdateStatistics(*Route, DeliveryTime);

	// Update company statistics
	for (FLogisticsCompany& Company : LogisticsCompanies)
	{
		if (Company.OwnedShipIds.Contains(ShipId))
		{
			Company.TotalCargoDelivered += CargoQuantity;
			Company.TotalRevenue += Route->EstimatedProfit;
			Company.TotalDeliveries++;

			// Update average delivery time
			if (Company.TotalDeliveries == 1)
			{
				Company.AverageDeliveryTime = DeliveryTime;
			}
			else
			{
				Company.AverageDeliveryTime = 
					(Company.AverageDeliveryTime * (Company.TotalDeliveries - 1) + DeliveryTime) / 
					Company.TotalDeliveries;
			}
			break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Ship %s (Faction %d) delivered %d units of %s to %s (delivery time: %.1f hours)"),
		*Ship->ShipName,
		Ship->OwnerId,
		CargoQuantity,
		*UEnum::GetValueAsString(Route->GoodType),
		*GetLocationName(UniverseData, Route->DestinationSystemId, Route->DestinationLocationId),
		DeliveryTime / 3600.0f);

	return true;
}

// ============================================================================
// SHIP TRANSIT SIMULATION
// ============================================================================

void ULogisticsSubsystem::TickShipTransit(FUniverseData& UniverseData, double CurrentGameTime)
{
	// Update all ships in fleet registry
	// Process loading, jumping between systems, and delivery

	for (auto& ShipPair : FleetRegistry)
	{
		FShipData& Ship = ShipPair.Value;

		// Find assigned route for this ship (if any)
		FTradeRoute* Route = nullptr;
		for (FTradeRoute& R : TradeRoutes)
		{
			if (R.AssignedShipId == Ship.ShipId && R.Status == ETradeRouteStatus::Active)
			{
				Route = &R;
				break;
			}
		}

		// Handle ships with active routes that are still docked (need to load cargo)
		if (Ship.Status == EShipStatus::Docked && Route != nullptr)
		{
			// Ship has a route but hasn't loaded cargo yet - initiate loading
			if (LoadCargo(UniverseData, Ship.ShipId, Route->RouteId))
			{
				UE_LOG(LogTemp, Verbose, TEXT("Ship %s started transit after loading cargo"), *Ship.ShipName);
				// Ship status is now InTransit (set by LoadCargo)
			}
			else
			{
				// Loading failed - cancel route
				UE_LOG(LogTemp, Warning, TEXT("Ship %s failed to load cargo - canceling route"), *Ship.ShipName);
				Route->Status = ETradeRouteStatus::Failed;
			}
			continue;
		}

		// Skip docked ships without routes
		if (Ship.Status == EShipStatus::Docked)
			continue;

		if (!Route)
		{
			// No active route but ship is not docked - return to docked status
			Ship.Status = EShipStatus::Docked;
			continue;
		}

		// Handle ship transit states
		if (Ship.Status == EShipStatus::InTransit)
		{
			// Ship is jumping between systems
			// For now, use simple time-based progression
			// Future: could track jump fuel consumption, route progress, etc.

			// Check if we've reached destination system
			if (Ship.CurrentSystemId == Route->DestinationSystemId)
			{
				// Arrived at destination - deliver cargo
				if (DeliverCargo(UniverseData, Ship.ShipId, Route->RouteId))
				{
					UE_LOG(LogTemp, Verbose, TEXT("Ship %s finished delivery"), *Ship.ShipName);
				}
				else
				{
					// Delivery failed
					UE_LOG(LogTemp, Warning, TEXT("Ship %s failed to deliver cargo"), *Ship.ShipName);
					Ship.Status = EShipStatus::Docked;
					Route->Status = ETradeRouteStatus::Failed;
				}
			}
			else
			{
				// Advance ship along route toward destination
				AdvanceShipAlongRoute(Ship, *Route, CurrentGameTime);
			}
		}
	}
}

void ULogisticsSubsystem::AdvanceShipAlongRoute(FShipData& Ship, FTradeRoute& Route, double CurrentGameTime)
{
	// Sprint 6: Ship traverses jump network one system at a time
	// Uses RouteProgress to track position in JumpPath

	if (Route.JumpPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Ship %s has empty jump path"), *Ship.ShipName);
		return;
	}

	// Advance to next system in path
	Route.RouteProgress++;

	// Safety check: don't go past destination
	if (Route.RouteProgress >= Route.JumpPath.Num())
	{
		Route.RouteProgress = Route.JumpPath.Num() - 1;
	}

	// Update ship's current system
	Ship.CurrentSystemId = Route.JumpPath[Route.RouteProgress];

	// Log progress (verbose to avoid spam)
	UE_LOG(LogTemp, Verbose, TEXT("Ship %s (Faction %d) jumped to system %d (progress: %d/%d)"),
		*Ship.ShipName,
		Ship.OwnerId,
		Ship.CurrentSystemId,
		Route.RouteProgress + 1,
		Route.JumpPath.Num());

	// Future enhancements:
	// - Deduct jump fuel based on distance
	// - Apply ship wear/tear per jump
	// - Track jump time for realistic transit simulation
	// - Handle jump gate dependencies vs independent FTL
}

// ============================================================================
// STATISTICS & REPORTING
// ============================================================================

void ULogisticsSubsystem::UpdateStatistics(const FTradeRoute& Route, float DeliveryTime)
{
	// Update global statistics
	Statistics.TotalCargoMoved += Route.CargoQuantity;
	Statistics.TotalDeliveries++;
	Statistics.TotalTradeValue += Route.EstimatedProfit;

	// Update average delivery distance
	if (Statistics.TotalDeliveries == 1)
	{
		Statistics.AverageDeliveryDistance = static_cast<float>(Route.RouteDistance);
	}
	else
	{
		Statistics.AverageDeliveryDistance = 
			(Statistics.AverageDeliveryDistance * (Statistics.TotalDeliveries - 1) + Route.RouteDistance) / 
			Statistics.TotalDeliveries;
	}

	// Update route statistics
	FTradeRouteStatistics* RouteStats = nullptr;
	for (FTradeRouteStatistics& Stats : Statistics.TopRoutes)
	{
		if (Stats.SourceSystemId == Route.SourceSystemId && 
			Stats.DestinationSystemId == Route.DestinationSystemId)
		{
			RouteStats = &Stats;
			break;
		}
	}

	if (!RouteStats)
	{
		// Create new route stats entry
		FTradeRouteStatistics NewStats;
		NewStats.SourceSystemId = Route.SourceSystemId;
		NewStats.DestinationSystemId = Route.DestinationSystemId;
		NewStats.TotalShipments = 0;
		NewStats.TotalCargoMoved = 0;
		NewStats.TotalValue = 0.0f;
		Statistics.TopRoutes.Add(NewStats);
		RouteStats = &Statistics.TopRoutes.Last();
	}

	RouteStats->TotalShipments++;
	RouteStats->TotalCargoMoved += Route.CargoQuantity;
	RouteStats->TotalValue += Route.EstimatedProfit;

	// Update good statistics
	FGoodTradeStatistics* GoodStats = nullptr;
	for (FGoodTradeStatistics& Stats : Statistics.MostTradedGoods)
	{
		if (Stats.GoodType == Route.GoodType)
		{
			GoodStats = &Stats;
			break;
		}
	}

	if (!GoodStats)
	{
		// Create new good stats entry
		FGoodTradeStatistics NewStats;
		NewStats.GoodType = Route.GoodType;
		NewStats.TotalUnitsTraded = 0;
		NewStats.TotalValue = 0.0f;
		NewStats.TotalShipments = 0;
		NewStats.AveragePrice = 0.0f;
		Statistics.MostTradedGoods.Add(NewStats);
		GoodStats = &Statistics.MostTradedGoods.Last();
	}

	GoodStats->TotalUnitsTraded += Route.CargoQuantity;
	GoodStats->TotalValue += Route.EstimatedProfit;
	GoodStats->TotalShipments++;

	// Update average price
	if (Route.CargoQuantity > 0)
	{
		float RouteAvgPrice = Route.EstimatedProfit / Route.CargoQuantity;
		if (GoodStats->TotalShipments == 1)
		{
			GoodStats->AveragePrice = RouteAvgPrice;
		}
		else
		{
			GoodStats->AveragePrice = 
				(GoodStats->AveragePrice * (GoodStats->TotalShipments - 1) + RouteAvgPrice) / 
				GoodStats->TotalShipments;
		}
	}
}

TArray<FTradeRouteStatistics> ULogisticsSubsystem::GetTopTradeRoutes(int32 TopN) const
{
	TArray<FTradeRouteStatistics> Sorted = Statistics.TopRoutes;

	// Sort by total cargo moved (descending)
	Sorted.Sort([](const FTradeRouteStatistics& A, const FTradeRouteStatistics& B)
	{
		return A.TotalCargoMoved > B.TotalCargoMoved;
	});

	// Return top N
	if (Sorted.Num() > TopN)
	{
		Sorted.SetNum(TopN);
	}

	return Sorted;
}

TArray<FGoodTradeStatistics> ULogisticsSubsystem::GetMostTradedGoods(int32 TopN) const
{
	TArray<FGoodTradeStatistics> Sorted = Statistics.MostTradedGoods;

	// Sort by total units traded (descending)
	Sorted.Sort([](const FGoodTradeStatistics& A, const FGoodTradeStatistics& B)
	{
		return A.TotalUnitsTraded > B.TotalUnitsTraded;
	});

	// Return top N
	if (Sorted.Num() > TopN)
	{
		Sorted.SetNum(TopN);
	}

	return Sorted;
}

// ============================================================================
// DEBUG & VALIDATION
// ============================================================================

void ULogisticsSubsystem::PrintTradeRequests() const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("TRADE REQUESTS"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Total Requests: %d"), TradeRequests.Num());

	if (TradeRequests.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No active trade requests."));
		return;
	}

	// Group by priority
	TArray<const FTradeRequest*> Critical;
	TArray<const FTradeRequest*> High;
	TArray<const FTradeRequest*> Normal;
	TArray<const FTradeRequest*> Low;

	for (const FTradeRequest& Request : TradeRequests)
	{
		if (Request.bIsMatched)
			continue; // Skip matched requests

		switch (Request.Priority)
		{
		case ETradeRequestPriority::Critical:
			Critical.Add(&Request);
			break;
		case ETradeRequestPriority::High:
			High.Add(&Request);
			break;
		case ETradeRequestPriority::Normal:
			Normal.Add(&Request);
			break;
		case ETradeRequestPriority::Low:
			Low.Add(&Request);
			break;
		}
	}

	if (Critical.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("\n[CRITICAL PRIORITY] (%d)"), Critical.Num());
		for (const FTradeRequest* Req : Critical)
		{
			UE_LOG(LogTemp, Log, TEXT("  Request %d: %s x%d (System %d, Location %d) - Max Price: %.1f"),
				Req->RequestId,
				*UEnum::GetValueAsString(Req->GoodType),
				Req->QuantityNeeded,
				Req->SystemId,
				Req->LocationId,
				Req->MaxPrice);
		}
	}

	if (High.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("\n[HIGH PRIORITY] (%d)"), High.Num());
		for (const FTradeRequest* Req : High)
		{
			UE_LOG(LogTemp, Log, TEXT("  Request %d: %s x%d (System %d, Location %d)"),
				Req->RequestId,
				*UEnum::GetValueAsString(Req->GoodType),
				Req->QuantityNeeded,
				Req->SystemId,
				Req->LocationId);
		}
	}

	if (Normal.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("\n[NORMAL PRIORITY] (%d)"), Normal.Num());
		for (const FTradeRequest* Req : Normal)
		{
			UE_LOG(LogTemp, Log, TEXT("  Request %d: %s x%d (System %d, Location %d)"),
				Req->RequestId,
				*UEnum::GetValueAsString(Req->GoodType),
				Req->QuantityNeeded,
				Req->SystemId,
				Req->LocationId);
		}
	}

	if (Low.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("\n[LOW PRIORITY] (%d)"), Low.Num());
		for (const FTradeRequest* Req : Low)
		{
			UE_LOG(LogTemp, Log, TEXT("  Request %d: %s x%d (System %d, Location %d)"),
				Req->RequestId,
				*UEnum::GetValueAsString(Req->GoodType),
				Req->QuantityNeeded,
				Req->SystemId,
				Req->LocationId);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void ULogisticsSubsystem::PrintExportOpportunities() const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("EXPORT OPPORTUNITIES"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Total Opportunities: %d"), ExportOpportunities.Num());

	if (ExportOpportunities.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No active export opportunities."));
		return;
	}

	// Sort by quantity available (descending)
	TArray<const FExportOpportunity*> Sorted;
	for (const FExportOpportunity& Opp : ExportOpportunities)
	{
		Sorted.Add(&Opp);
	}

	Sorted.Sort([](const FExportOpportunity& A, const FExportOpportunity& B)
	{
		return (A.QuantityAvailable - A.QuantityReserved) > (B.QuantityAvailable - B.QuantityReserved);
	});

	for (const FExportOpportunity* Opp : Sorted)
	{
		int32 AvailableNow = Opp->QuantityAvailable - Opp->QuantityReserved;
		UE_LOG(LogTemp, Log, TEXT("  Opportunity %d: %s x%d (Available: %d, Reserved: %d) - System %d, Location %d - Price: %.1f"),
			Opp->OpportunityId,
			*UEnum::GetValueAsString(Opp->GoodType),
			Opp->QuantityAvailable,
			AvailableNow,
			Opp->QuantityReserved,
			Opp->SystemId,
			Opp->LocationId,
			Opp->MarketPrice);
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void ULogisticsSubsystem::PrintActiveRoutes() const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("ACTIVE TRADE ROUTES"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));

	int32 ActiveCount = 0;
	int32 PendingCount = 0;
	int32 CompletedCount = 0;

	for (const FTradeRoute& Route : TradeRoutes)
	{
		if (Route.Status == ETradeRouteStatus::Active)
			ActiveCount++;
		else if (Route.Status == ETradeRouteStatus::Pending)
			PendingCount++;
		else if (Route.Status == ETradeRouteStatus::Completed)
			CompletedCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("Active: %d | Pending: %d | Completed: %d | Total: %d"),
		ActiveCount, PendingCount, CompletedCount, TradeRoutes.Num());

	if (ActiveCount == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No active routes in transit."));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("\n[ACTIVE ROUTES]"));
		for (const FTradeRoute& Route : TradeRoutes)
		{
			if (Route.Status == ETradeRouteStatus::Active)
			{
				// Find assigned ship
				FString ShipInfo = TEXT("Unknown");
				const FShipData* Ship = FleetRegistry.Find(Route.AssignedShipId);
				if (Ship)
				{
					ShipInfo = FString::Printf(TEXT("%s (Faction %d, System %d)"),
						*Ship->ShipName,
						Ship->OwnerId,
						Ship->CurrentSystemId);
				}

				UE_LOG(LogTemp, Log, TEXT("  Route %d: %s x%d | Distance: %d jumps | Ship: %s"),
					Route.RouteId,
					*UEnum::GetValueAsString(Route.GoodType),
					Route.CargoQuantity,
					Route.RouteDistance,
					*ShipInfo);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void ULogisticsSubsystem::PrintCompanyStatistics() const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("LOGISTICS COMPANY STATISTICS"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));

	for (const FLogisticsCompany& Company : LogisticsCompanies)
	{
		UE_LOG(LogTemp, Log, TEXT("\n%s (ID: %d)"), *Company.Name, Company.CompanyId);
		UE_LOG(LogTemp, Log, TEXT("  Credits: %.0f"), Company.Credits);
		UE_LOG(LogTemp, Log, TEXT("  Active Ships: %d"), Company.ActiveShipCount);
		UE_LOG(LogTemp, Log, TEXT("  Total Deliveries: %d"), Company.TotalDeliveries);
		UE_LOG(LogTemp, Log, TEXT("  Total Cargo Delivered: %lld units"), Company.TotalCargoDelivered);
		UE_LOG(LogTemp, Log, TEXT("  Total Revenue: %.0f credits"), Company.TotalRevenue);

		if (Company.TotalDeliveries > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("  Avg Delivery Time: %.1f hours"), 
				Company.AverageDeliveryTime / 3600.0f);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void ULogisticsSubsystem::PrintCargoShipStatistics() const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("LOGISTICS FLEET STATISTICS"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Total Ships: %d"), FleetRegistry.Num());

	// Count by status
	int32 DockedCount = 0;
	int32 InTransitCount = 0;
	int32 DisabledCount = 0;

	for (const auto& ShipPair : FleetRegistry)
	{
		const FShipData& Ship = ShipPair.Value;
		switch (Ship.Status)
		{
		case EShipStatus::Docked:
			DockedCount++;
			break;
		case EShipStatus::InTransit:
			InTransitCount++;
			break;
		case EShipStatus::Disabled:
			DisabledCount++;
			break;
		default:
			break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Docked: %d | In Transit: %d | Disabled: %d"),
		DockedCount, InTransitCount, DisabledCount);

	// Show detailed info for active ships
	if (InTransitCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("\n[SHIPS IN TRANSIT]"));
		for (const auto& ShipPair : FleetRegistry)
		{
			const FShipData& Ship = ShipPair.Value;
			if (Ship.Status == EShipStatus::InTransit)
			{
				UE_LOG(LogTemp, Log, TEXT("  %s (Faction %d): System %d | Cargo: %d/%d | Hull: %.0f%%"),
					*Ship.ShipName,
					Ship.OwnerId,
					Ship.CurrentSystemId,
					Ship.CurrentCargoUsed,
					Ship.CargoCapacity,
					Ship.HullCondition);
			}
		}
	}

	// Show fleet composition by faction
	TMap<int32, int32> FactionFleetCounts;
	for (const auto& ShipPair : FleetRegistry)
	{
		const FShipData& Ship = ShipPair.Value;
		if (Ship.OwnerType == TEXT("Faction"))
		{
			FactionFleetCounts.FindOrAdd(Ship.OwnerId, 0)++;
		}
	}

	if (FactionFleetCounts.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("\n[FLEET BY FACTION]"));
		for (const auto& Pair : FactionFleetCounts)
		{
			UE_LOG(LogTemp, Log, TEXT("  Faction %d: %d ships"), Pair.Key, Pair.Value);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void ULogisticsSubsystem::PrintEconomicDependencies(const FUniverseData& UniverseData) const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("ECONOMIC DEPENDENCIES"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));

	// Analyze completed trade routes to identify dependencies
	if (TradeRoutes.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No trade routes yet - dependencies will emerge over time."));
		UE_LOG(LogTemp, Log, TEXT("========================================"));
		return;
	}

	int32 CompletedCount = 0;
	for (const FTradeRoute& Route : TradeRoutes)
	{
		if (Route.Status == ETradeRouteStatus::Completed)
			CompletedCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("Completed Trades: %d"), CompletedCount);

	if (CompletedCount == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No completed trades yet - dependencies will emerge as deliveries complete."));
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void ULogisticsSubsystem::PrintTopImporters(int32 TopN) const
{
	UE_LOG(LogTemp, Log, TEXT("PrintTopImporters: Analysis available after trade history builds"));
}

void ULogisticsSubsystem::PrintTopExporters(int32 TopN) const
{
	UE_LOG(LogTemp, Log, TEXT("PrintTopExporters: Analysis available after trade history builds"));
}

void ULogisticsSubsystem::PrintMostTradedGoods() const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("MOST TRADED GOODS"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));

	if (Statistics.MostTradedGoods.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No trade history yet."));
		UE_LOG(LogTemp, Log, TEXT("========================================"));
		return;
	}

	TArray<FGoodTradeStatistics> Sorted = GetMostTradedGoods(10);

	for (int32 i = 0; i < Sorted.Num(); ++i)
	{
		const FGoodTradeStatistics& Stats = Sorted[i];
		UE_LOG(LogTemp, Log, TEXT("%d. %s: %lld units (%d shipments) - Avg Price: %.1f - Total Value: %.0f"),
			i + 1,
			*UEnum::GetValueAsString(Stats.GoodType),
			Stats.TotalUnitsTraded,
			Stats.TotalShipments,
			Stats.AveragePrice,
			Stats.TotalValue);
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void ULogisticsSubsystem::PrintLogisticsSystemStatus() const
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("LOGISTICS SYSTEM STATUS"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));

	UE_LOG(LogTemp, Log, TEXT("Companies: %d"), LogisticsCompanies.Num());
	UE_LOG(LogTemp, Log, TEXT("Ships: %d"), FleetRegistry.Num());
	UE_LOG(LogTemp, Log, TEXT("Active Routes: %d"), Statistics.ActiveRouteCount);
	UE_LOG(LogTemp, Log, TEXT("Active Ships: %d"), Statistics.ActiveShipCount);
	UE_LOG(LogTemp, Log, TEXT("Trade Requests: %d"), TradeRequests.Num());
	UE_LOG(LogTemp, Log, TEXT("Export Opportunities: %d"), ExportOpportunities.Num());

	UE_LOG(LogTemp, Log, TEXT("\n[GLOBAL STATISTICS]"));
	UE_LOG(LogTemp, Log, TEXT("Total Deliveries: %d"), Statistics.TotalDeliveries);
	UE_LOG(LogTemp, Log, TEXT("Total Cargo Moved: %lld units"), Statistics.TotalCargoMoved);
	UE_LOG(LogTemp, Log, TEXT("Total Trade Value: %.0f credits"), Statistics.TotalTradeValue);
	UE_LOG(LogTemp, Log, TEXT("Average Delivery Distance: %.1f jumps"), Statistics.AverageDeliveryDistance);

	UE_LOG(LogTemp, Log, TEXT("\n[TOP TRADE ROUTES]"));
	TArray<FTradeRouteStatistics> TopRoutes = GetTopTradeRoutes(5);
	for (int32 i = 0; i < TopRoutes.Num(); ++i)
	{
		const FTradeRouteStatistics& RouteStats = TopRoutes[i];
		UE_LOG(LogTemp, Log, TEXT("  %d. System %d -> System %d: %lld units (%d shipments) - Value: %.0f"),
			i + 1,
			RouteStats.SourceSystemId,
			RouteStats.DestinationSystemId,
			RouteStats.TotalCargoMoved,
			RouteStats.TotalShipments,
			RouteStats.TotalValue);
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void ULogisticsSubsystem::PrintSystemHealthDiagnostic() const
{
	UE_LOG(LogTemp, Warning, TEXT("========================================"));
	UE_LOG(LogTemp, Warning, TEXT("LOGISTICS HEALTH DIAGNOSTIC"));
	UE_LOG(LogTemp, Warning, TEXT("========================================"));

	// Get universe subsystem
	UUniverseSubsystem* UniverseSubsystem = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	if (!UniverseSubsystem || !UniverseSubsystem->IsUniverseGenerated())
	{
		UE_LOG(LogTemp, Error, TEXT("❌ CRITICAL: Universe not generated!"));
		UE_LOG(LogTemp, Warning, TEXT("========================================"));
		return;
	}

	// Get economy subsystem
	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ CRITICAL: Economy subsystem not found!"));
		UE_LOG(LogTemp, Warning, TEXT("========================================"));
		return;
	}

	const FUniverseData& UniverseData = UniverseSubsystem->GetUniverseData();

	// Print current game time
	UE_LOG(LogTemp, Log, TEXT("\n[CURRENT GAME TIME]"));
	UE_LOG(LogTemp, Display, TEXT("Year: %d, Month: %d, Day: %d"), 
		UniverseData.CurrentTime.Year, UniverseData.CurrentTime.Month, UniverseData.CurrentTime.Day);

	// Check 1: Logistics Initialized
	UE_LOG(LogTemp, Log, TEXT("\n[LOGISTICS INITIALIZATION]"));
	if (LogisticsCompanies.Num() > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("✅ Logistics initialized: %d companies, %d ships"), 
			LogisticsCompanies.Num(), FleetRegistry.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ PROBLEM: Logistics not initialized! Call InitializeLogistics() first."));
	}

	// Check 2: Active Economy System
	UE_LOG(LogTemp, Log, TEXT("\n[ECONOMY STATUS]"));
	int32 ActiveSystemId = UniverseSubsystem->GetActiveEconomySystemId();
	if (ActiveSystemId >= 0)
	{
		const FStarSystemData& ActiveSystem = UniverseData.Systems[ActiveSystemId];
		UE_LOG(LogTemp, Display, TEXT("✅ Active economy system: %s (ID: %d)"), 
			*ActiveSystem.SystemName, ActiveSystemId);

		// Count active markets
		int32 ActiveMarkets = 0;
		for (const FLocationData& Location : ActiveSystem.Locations)
		{
			if (Location.Market.bIsActiveSimulation)
			{
				ActiveMarkets++;
			}
		}
		UE_LOG(LogTemp, Display, TEXT("   Active markets: %d"), ActiveMarkets);

		// Show trade network status
		UE_LOG(LogTemp, Display, TEXT("✅ Trade network systems: %d (within %d jumps)"), 
			TradeNetworkActiveSystems.Num(), TradeNetworkRadius);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ PROBLEM: No active economy system!"));
		UE_LOG(LogTemp, Error, TEXT("   The economy is not ticking, so no shortages/surpluses can form."));
		UE_LOG(LogTemp, Error, TEXT("   FIX: Call SetActiveEconomySystem(SystemId) after InitializeEconomy()"));
		UE_LOG(LogTemp, Error, TEXT("   Example: SetActiveEconomySystem(0)  // Activate first faction core"));
	}

	// Check 3: Trade Requests
	UE_LOG(LogTemp, Log, TEXT("\n[TRADE NETWORK]"));
	if (TradeRequests.Num() > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("✅ Trade requests: %d"), TradeRequests.Num());

		// Show a sample
		if (TradeRequests.Num() > 0)
		{
			const FTradeRequest& Sample = TradeRequests[0];
			UE_LOG(LogTemp, Display, TEXT("   Sample: System %d needs %d units of good type %d"), 
				Sample.SystemId, Sample.QuantityNeeded, (int32)Sample.GoodType);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  No trade requests yet"));
		UE_LOG(LogTemp, Warning, TEXT("   This is normal if:"));
		UE_LOG(LogTemp, Warning, TEXT("   - Economy just started (wait 30-60 seconds)"));
		UE_LOG(LogTemp, Warning, TEXT("   - No shortages have formed yet"));
		UE_LOG(LogTemp, Warning, TEXT("   But if you've waited 2+ minutes:"));
		UE_LOG(LogTemp, Warning, TEXT("   - Check that active system is set (see above)"));
		UE_LOG(LogTemp, Warning, TEXT("   - Check that time is advancing (game time should increase)"));
	}

	// Check 4: Export Opportunities
	if (ExportOpportunities.Num() > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("✅ Export opportunities: %d"), ExportOpportunities.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  No export opportunities yet"));
	}

	// Check 5: Active Routes
	if (Statistics.ActiveRouteCount > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("✅ Active trade routes: %d"), Statistics.ActiveRouteCount);
		UE_LOG(LogTemp, Display, TEXT("✅ Ships in transit: %d"), Statistics.ActiveShipCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  No active trade routes yet"));
		if (TradeRequests.Num() > 0 && ExportOpportunities.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("   Requests and exports exist but no routes matched"));
			UE_LOG(LogTemp, Warning, TEXT("   This might indicate pathfinding issues or unprofitable routes"));
		}
	}

	// Check 6: Statistics
	UE_LOG(LogTemp, Log, TEXT("\n[LIFETIME STATISTICS]"));
	if (Statistics.TotalDeliveries > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("✅ System is working! Total deliveries: %d"), Statistics.TotalDeliveries);
		UE_LOG(LogTemp, Display, TEXT("   Total cargo moved: %lld units"), Statistics.TotalCargoMoved);
		UE_LOG(LogTemp, Display, TEXT("   Total trade value: %.0f credits"), Statistics.TotalTradeValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  No deliveries completed yet"));
	}

	// Summary and next steps
	UE_LOG(LogTemp, Log, TEXT("\n[DIAGNOSIS SUMMARY]"));
	if (ActiveSystemId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ PRIMARY ISSUE: No active economy system"));
		UE_LOG(LogTemp, Error, TEXT("   SOLUTION: Add this line to your blueprint after InitializeEconomy():"));
		UE_LOG(LogTemp, Error, TEXT("   SetActiveEconomySystem -> System ID: 0"));
	}
	else if (TradeRequests.Num() == 0 && ExportOpportunities.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  PRIMARY ISSUE: Economy hasn't generated shortages/surpluses yet"));
		UE_LOG(LogTemp, Warning, TEXT("   SOLUTION: Wait 30-60 more seconds for economy to diverge"));
		UE_LOG(LogTemp, Warning, TEXT("   TIP: Increase time scale to speed up: SetTimeScale(240.0)"));
	}
	else if (Statistics.ActiveRouteCount == 0 && TradeRequests.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  PRIMARY ISSUE: Routes not being matched"));
		UE_LOG(LogTemp, Warning, TEXT("   This usually resolves itself as more opportunities appear"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("✅ System is healthy and operating normally!"));
	}

	UE_LOG(LogTemp, Warning, TEXT("========================================"));
}

// ============================================================================
// TRADE NETWORK CATCH-UP SYSTEM
// ============================================================================

void ULogisticsSubsystem::CatchUpTradeNetwork(FUniverseData& UniverseData)
{
	// Sprint 6: Proper Fix - Catch up nearby systems so they can generate trade opportunities
	// This ensures the trade network has active economies to work with

	if (UniverseData.ActiveSystemId < 0)
	{
		return; // No active system, nothing to catch up
	}

	// Get economy subsystem
	UEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UEconomySubsystem>();
	if (!EconomySubsystem)
	{
		return;
	}

	// Find systems within trade network radius
	TSet<int32> NewTradeNetworkSystems = FindSystemsWithinRange(
		UniverseData, 
		UniverseData.ActiveSystemId, 
		TradeNetworkRadius);

	// Add the active system itself
	NewTradeNetworkSystems.Add(UniverseData.ActiveSystemId);

	// Catch up any new systems that entered the trade network
	int32 CaughtUpCount = 0;
	for (int32 SystemId : NewTradeNetworkSystems)
	{
		if (!TradeNetworkActiveSystems.Contains(SystemId))
		{
			// This system just entered the trade network - catch it up
			EconomySubsystem->CatchUpSystemEconomy(UniverseData, SystemId);
			TradeNetworkActiveSystems.Add(SystemId);
			CaughtUpCount++;
		}
	}

	// Remove systems that are no longer in range
	TSet<int32> SystemsToRemove;
	for (int32 SystemId : TradeNetworkActiveSystems)
	{
		if (!NewTradeNetworkSystems.Contains(SystemId))
		{
			SystemsToRemove.Add(SystemId);
		}
	}

	for (int32 SystemId : SystemsToRemove)
	{
		TradeNetworkActiveSystems.Remove(SystemId);
	}

	if (CaughtUpCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("LogisticsSubsystem: Caught up %d systems for trade network (total active: %d)"),
			CaughtUpCount, TradeNetworkActiveSystems.Num());
	}

	LastTradeNetworkCatchupTime = UniverseData.CurrentTime.TotalElapsedSeconds;
}

TSet<int32> ULogisticsSubsystem::FindSystemsWithinRange(
	const FUniverseData& UniverseData, 
	int32 CenterSystemId, 
	int32 MaxJumps) const
{
	TSet<int32> VisitedSystems;
	TArray<int32> CurrentLayer;
	TArray<int32> NextLayer;

	// Start with center system
	CurrentLayer.Add(CenterSystemId);
	VisitedSystems.Add(CenterSystemId);

	// BFS to find all systems within MaxJumps
	for (int32 Jump = 0; Jump < MaxJumps; ++Jump)
	{
		NextLayer.Empty();

		for (int32 SystemId : CurrentLayer)
		{
			if (!UniverseData.Systems.IsValidIndex(SystemId))
				continue;

			const FStarSystemData& System = UniverseData.Systems[SystemId];

			// Add all connected systems
			for (int32 ConnectedSystemId : System.ConnectedSystemIds)
			{
				if (!VisitedSystems.Contains(ConnectedSystemId))
				{
					NextLayer.Add(ConnectedSystemId);
					VisitedSystems.Add(ConnectedSystemId);
				}
			}
		}

		// Move to next layer
		CurrentLayer = NextLayer;

		if (CurrentLayer.Num() == 0)
		{
			break; // No more systems to explore
		}
	}

	// Remove the center system from results (caller will add it separately if needed)
	VisitedSystems.Remove(CenterSystemId);

	return VisitedSystems;
}
