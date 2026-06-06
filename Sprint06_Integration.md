# Sprint 06: NPC Logistics & Trade Network - Integration Guide

## Overview
Sprint 06 implements the first living-universe behavior: NPC cargo ships that physically move goods through the jump network in response to economic shortages and surpluses.

**Core Principle**: Goods never teleport. All movement is physical via ships.

## Architecture

### Components Implemented

#### 1. LogisticsTypes.h
Defines all data structures for the logistics system:

**Trade System:**
- `FTradeRequest` - Import demands from locations with shortages
- `FExportOpportunity` - Surplus goods available for export
- `FTradeRoute` - Matched trade with jump path and profitability
- `ETradeRequestPriority` - Urgency levels (Critical/High/Normal/Low)

**Entities:**
- `FLogisticsCompany` - NPC cargo hauling organizations
- `FCargoShip` - Abstract ships with cargo, location, route progress
- `FCargoManifestEntry` - Goods in ship cargo hold
- `ECargoShipStatus` - Ship states (Idle/Loading/InTransit/Unloading)

**Statistics:**
- `FLogisticsStatistics` - Universe-wide trade metrics
- `FTradeRouteStatistics` - Per-lane shipment tracking
- `FGoodTradeStatistics` - Per-good volume and pricing

#### 2. LogisticsSubsystem
Game instance subsystem managing entire logistics simulation:

**Lifecycle:**
- Initializes 4-6 logistics companies with 2-4 ships each
- Ships start in faction core systems
- Runs periodic tick (default 10s interval)

**Trade Flow:**
1. `GenerateTradeRequests()` - Scans markets for shortages
2. `GenerateExportOpportunities()` - Scans markets for surpluses
3. `MatchTrades()` - Pairs requests with opportunities by profitability
4. `GenerateTradeRoute()` - Uses UniverseSubsystem pathfinding for jump path
5. Ship assignment and execution

**Cargo Flow:**
1. `LoadCargo()` - Decrease source inventory, increase ship cargo
2. `TickShipTransit()` - Move ship through systems over time
3. `AdvanceShipAlongRoute()` - Progress through jump path
4. `DeliverCargo()` - Increase destination inventory, clear ship cargo

**Statistics:**
- `UpdateStatistics()` - Track deliveries, volume, routes, goods
- `GetTopTradeRoutes()` - Query busiest lanes
- `GetMostTradedGoods()` - Query highest volume goods

**Debug Commands:** (see Sprint06_ValidationGuide.md)

## Integration with Existing Systems

### UniverseSubsystem Integration

**Pathfinding:**
```cpp
TArray<int32> JumpPath = UniverseSys->FindPath(SourceSystemId, DestSystemId);
int32 Distance = UniverseSys->GetJumpDistance(SystemA, SystemB);
```

**State Access:**
```cpp
const FUniverseData& Universe = UniverseSys->GetUniverseData();
const FStarSystemData& System = Universe.Systems[SystemId];
const FLocationData& Location = System.Locations[LocationId];
```

**Market Modification:**
```cpp
// Via economy subsystem or direct access
FMarketGoodEntry& Entry = Location.Market.Goods[GoodIndex];
Entry.Stock += DeliveryQuantity;
Entry.bIsShortage = (Entry.Stock < Entry.TargetStock * 0.5f);
```

### EconomySubsystem Integration

**Shortage Detection:**
```cpp
// Economy subsystem sets these flags during simulation
MarketEntry.bIsShortage = true;  // Stock < 50% of target
MarketEntry.bIsSurplus = true;   // Stock > 150% of target
```

**Logistics Response:**
```cpp
// LogisticsSubsystem reads these flags
if (MarketEntry.bIsShortage)
{
	// Generate trade request
}
if (MarketEntry.bIsSurplus)
{
	// Generate export opportunity
}
```

**Market Updates:**
```cpp
// After delivery, economy subsystem will:
// - Recalculate prices based on new stock levels
// - Update shortage/surplus flags
// - Adjust economic stress
```

### Coordination

Both subsystems run independently:

**EconomySubsystem**: 
- Ticks every 5-20 seconds (active systems)
- Simulates production, consumption
- Updates prices, sets shortage/surplus flags

**LogisticsSubsystem**:
- Ticks every 10 seconds
- Reads shortage/surplus flags
- Moves cargo between locations
- Updates inventories

Result: **Emergent trade network** based on actual economic needs.

## Configuration

### LogisticsSubsystem Settings

```cpp
// In LogisticsSubsystem.h (protected section):
float LogisticsTickRate = 10.0f;         // Seconds between ticks
float JumpTimePerSystem = 3600.0f;       // Game time per jump (1 hour)
float LoadingTime = 1800.0f;             // Game time to load (30 min)
float UnloadingTime = 1800.0f;           // Game time to unload (30 min)
```

**Tuning Recommendations:**
- **Faster Trade**: Decrease JumpTimePerSystem, LoadingTime, UnloadingTime
- **More Routes**: Increase number of companies/ships in InitializeLogistics
- **Profitability**: Adjust CalculateEstimatedProfit operating costs
- **Priority**: Tweak CalculateRequestPriority thresholds

### Company Configuration

Companies created in `InitializeLogistics()`:
- 4-6 companies based on universe size (1 per 100 systems)
- 2-4 ships per company based on universe size (1 per 150 systems)
- Starting capital: 500,000 credits
- Ship capacity: 3,000-7,000 units (varied per ship)

## Startup Integration

### GameMode or GameInstance Setup

```cpp
void AMyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 1. Generate universe
	UUniverseSubsystem* UniverseSys = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	FUniverseConfig Config;
	Config.SystemCount = 500;
	Config.FactionCount = 5;
	UniverseSys->GenerateUniverse(Config);

	// 2. Initialize economy
	UniverseSys->InitializeEconomy();

	// 3. Initialize logistics (AFTER economy)
	ULogisticsSubsystem* LogisticsSys = GetGameInstance()->GetSubsystem<ULogisticsSubsystem>();
	LogisticsSys->InitializeLogistics(UniverseSys->GetUniverseData());

	// Systems now running:
	// - Universe time advancement
	// - Economy simulation (production, consumption, pricing)
	// - Logistics simulation (trade matching, cargo movement)
}
```

### Important: Initialization Order

1. **UniverseSubsystem::GenerateUniverse()** - Creates systems, locations
2. **UniverseSubsystem::InitializeEconomy()** - Sets up markets, profiles
3. **LogisticsSubsystem::InitializeLogistics()** - Creates companies, ships

Logistics **requires** economy to be initialized first (needs market data).

## File Structure

```
Source/FracturedStars/
├── Public/Universe/
│   ├── LogisticsTypes.h         [NEW] - Data structures
│   ├── LogisticsSubsystem.h     [NEW] - Subsystem interface
│   ├── UniverseTypes.h          [EXISTING] - EGoodType, markets, etc.
│   ├── UniverseSubsystem.h      [EXISTING] - Pathfinding, state
│   └── EconomySubsystem.h       [EXISTING] - Production, pricing
│
└── Private/Universe/
	├── LogisticsSubsystem.cpp   [NEW] - Implementation
	├── UniverseSubsystem.cpp    [EXISTING] - FindPath, etc.
	└── EconomySubsystem.cpp     [EXISTING] - Market simulation

[Project Root]/
├── Sprint06_ValidationGuide.md  [NEW] - Testing procedures
└── Sprint06_Integration.md      [NEW] - This file
```

## Goods Support

All goods from `EGoodType` enum are supported:

**Base Economy Goods:**
- Food, Water, Fuel, Ore, RefinedMetals
- Medicine, Machinery, Electronics, Weapons
- ConsumerGoods, IndustrialParts, AdvancedComponents
- ResearchMaterials, Contraband

**Ship Frames:** (Sprint 5.5)
- StarterMining, StarterTrade, StarterFighter
- MediumFreighter, MediumMiner, Escort
- HeavyFreighter, Cruiser, Carrier, Battleship

**Ship Components:** (Sprint 5.5)
- Engines: Titan I/II/III (Small/Medium/Large)
- Power Plants: Nova I/II/III (Small/Medium/Large)
- Shields: Atlas I/II (Small/Medium)
- Weapons: Lasers, Cannons, Missile Launchers
- Utility: Mining Lasers, Cargo Expansions, Fuel Tanks, Sensors

**Industrial Supply Chains Example:**
1. Mining Colony produces Ore
2. Refinery needs Ore → Trade Request generated
3. Refinery produces RefinedMetals
4. Industrial World needs RefinedMetals → Trade Request generated
5. Industrial World produces IndustrialParts
6. Shipyard needs IndustrialParts + Engine components → Trade Requests
7. Shipyard produces Ship Frames

Logistics system handles all stages automatically.

## Performance Considerations

### Scalability

**Small Universe (50-100 systems):**
- Default settings work well
- ~10-20 trade routes active
- Minimal overhead

**Medium Universe (200-500 systems):**
- Monitor trade request count
- May see 50-100 active routes
- Consider increasing tick rate to 15-20s

**Large Universe (500+ systems):**
- Throttle trade matching (process top N requests per tick)
- Cache pathfinding results for common routes
- Consider async/background processing for route generation

### Optimization Opportunities

**Trade Matching:**
```cpp
// Current: O(requests × opportunities)
// Future: Spatial indexing, good-type indexing
TMap<EGoodType, TArray<FExportOpportunity*>> ExportsByGood;
```

**Pathfinding:**
```cpp
// Current: BFS every time
// Future: Cache common routes
TMap<FSystemPair, TArray<int32>> PathCache;
```

**Ship Updates:**
```cpp
// Current: Check all ships every tick
// Future: Priority queue by next event time
// Only process ships ready for state change
```

## Debug Workflows

### Monitoring Active Trade

```cpp
// Watch a specific route from start to finish
int32 RouteId = 5;

// 1. Check route details
LogisticsSys->PrintActiveRoutes();

// 2. Find assigned ship
for (const FTradeRoute& Route : LogisticsSys->GetRoutes())
{
	if (Route.RouteId == RouteId)
	{
		int32 ShipId = Route.AssignedShipId;
		// Track this ship through journey
	}
}

// 3. Monitor ship progress
LogisticsSys->PrintCargoShipStatistics();

// 4. Verify delivery
LogisticsSys->PrintCompanyStatistics(); // Check delivery count increased
LogisticsSys->PrintLogisticsSystemStatus(); // Check global stats
```

### Debugging No Trades

If no trade routes are being created:

**Check 1: Are there shortages?**
```cpp
UniverseSys->PrintEconomyStats();
LogisticsSys->PrintTradeRequests();
```

**Check 2: Are there surpluses?**
```cpp
LogisticsSys->PrintExportOpportunities();
```

**Check 3: Is pathfinding working?**
```cpp
int32 Distance = UniverseSys->GetJumpDistance(SystemA, SystemB);
// If -1, no path exists (isolated systems)
```

**Check 4: Are ships available?**
```cpp
LogisticsSys->PrintCargoShipStatistics();
// All ships may be busy
```

## Future Enhancements

### Sprint 7+: Player Integration

When player ship system is implemented:
1. Replace FCargoShip with full ship component architecture
2. Add visual ship actors for active systems
3. Player can see NPC cargo ships in space
4. Player can intercept/scan/interact with cargo ships

### Sprint 8+: Advanced Features

**Piracy:**
- Cargo ships become targets
- Escort ship assignments
- Insurance/risk calculations

**Player Trading:**
- Player accepts cargo hauling contracts
- Bid on trade routes
- Compete with NPC logistics companies

**Diplomacy:**
- Faction embargoes block certain routes
- Tariffs affect profitability
- War disrupts trade networks

**News Integration:**
- Delivery events generate news
- Trade statistics become public information
- Company rankings published

## Acceptance Criteria Status

✓ **All Sprint 06 acceptance criteria met:**

1. ✓ Trade requests generated from shortages
2. ✓ Export opportunities generated from surpluses
3. ✓ Routes generated using jump network
4. ✓ Logistics companies exist
5. ✓ NPC cargo ships exist
6. ✓ Cargo physically moves through simulation
7. ✓ Markets update on delivery
8. ✓ Component trade supported
9. ✓ Industrial supply chains supported
10. ✓ Economic dependencies emerge naturally
11. ✓ Shortages can recover through logistics
12. ✓ Statistics system functions
13. ✓ No goods teleport
14. ✓ NPC ships use same data model (simplified for now, full integration later)

## Support

For issues or questions:
1. Check Sprint06_ValidationGuide.md for testing procedures
2. Use debug print commands to inspect system state
3. Monitor logs for warnings/errors during tick
4. Verify initialization order (Universe → Economy → Logistics)

## Summary

Sprint 06 successfully implements a **living trade network** that:
- Responds to actual economic needs
- Physically moves goods through space
- Creates emergent trade patterns
- Supports all goods including ship components
- Provides comprehensive monitoring and debugging
- Integrates cleanly with existing universe and economy systems

The foundation is in place for player interaction, piracy, diplomacy, and news generation in future sprints.
