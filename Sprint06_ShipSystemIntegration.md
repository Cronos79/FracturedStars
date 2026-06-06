# Sprint 06: Integrating Existing Ship System with Faction Logistics

## Summary

**DISCOVERED**: The complete component-based ship framework from Sprint 4 / Sprint 5.5 already exists in the codebase!

**LOCATION**: Commit `6ce7a2c` - Sprint 5.5: Component-Based Ship Framework & Faction Architecture

**KEY FILES**:
- `Source/FracturedStars/Public/Ship/ShipData.h` - FShipData struct
- `Source/FracturedStars/Public/Ship/ShipTypes.h` - Component system definitions
- `Source/FracturedStars/Public/Ship/ShipAssemblyLibrary.h` - Assembly & stat calculation

---

## What Already Exists

### FShipData - The Complete Ship Model
```cpp
struct FShipData
{
	// Identity
	int32 ShipId;
	FString ShipName;

	// OWNERSHIP (ready for factions!)
	int32 OwnerId;           // Faction ID, Player ID, NPC ID, etc.
	FName OwnerType;         // "Faction", "Player", "NPC", "Pirate"

	// LOCATION
	int32 CurrentSystemId;
	int32 CurrentLocationId;
	EShipStatus Status;      // Docked, InTransit, Active, Disabled, Destroyed

	// FRAME & COMPONENTS
	FName FrameId;
	float HullCondition;     // 0-100% hull integrity
	TArray<FInstalledComponent> InstalledComponents;

	// CARGO & FUEL (already implemented!)
	int32 CargoCapacity;
	int32 CurrentCargoUsed;
	TMap<EGoodType, int32> CargoInventory;  // Actual cargo goods

	int32 FuelCapacity;
	int32 CurrentFuel;

	// CREW
	int32 CrewCapacity;
	TArray<int32> AssignedCrewIds;

	// POWER SYSTEM
	int32 PowerGeneration;
	int32 PowerConsumption;
	bool bIsOperational;     // Has required components + valid power

	// PERFORMANCE STATS
	float SpeedModifier;
	int32 ShieldCapacity;
	int32 CurrentShields;
	int32 WeaponDamage;
	float MiningEfficiency;
	int32 SensorRange;
}
```

### Component System with Wear & Tear

**FInstalledComponent** already has:
- `CurrentCondition` (0-100%)
- Condition degrades over time/use
- Components can be damaged/repaired
- Power consumption/generation tracked

**FShipComponentDefinition** includes:
- `MaxCondition`
- `PowerGeneration` / `PowerConsumption`
- `CargoBonus`, `FuelBonus`
- `Mass`, `BaseManufacturingCost`
- Tiered quality (TierI through TierV)

### Faction-Specific Frames

**FShipFrameDefinition** already supports:
- `FactionId` field (e.g., "Human", "Khral", "Syndicate")
- Different frame classes per faction
- Slot layouts vary by faction
- `EconomicGoodType` for manufacturing/trading frames

### Assembly & Validation

**UShipAssemblyLibrary** provides:
- `CalculateShipStats()` - Frame + components → final stats
- `UpdateShipStats()` - Recalculate when components change
- `IsShipOperational()` - Validate required components & power
- `CanInstallComponent()` - Slot/size/type validation

---

## Current State: Logistics Ships vs Real Ships

### PROBLEM: Duplicate Ship Models

**Current Logistics Implementation** (`LogisticsTypes.h`):
```cpp
struct FCargoShip  // PROVISIONAL - should be replaced
{
	int32 ShipId;
	FName ShipName;
	int32 OwnerId;             // ← Duplicates FShipData.OwnerId
	FName OwnerType;           // ← Duplicates FShipData.OwnerType
	int32 HomeSystemId;
	int32 CurrentSystemId;     // ← Duplicates FShipData.CurrentSystemId
	int32 CurrentLocationId;   // ← Duplicates FShipData.CurrentLocationId
	int32 CargoCapacity;       // ← Duplicates FShipData.CargoCapacity
	int32 CurrentCargoUsed;    // ← Duplicates FShipData.CurrentCargoUsed
	int32 FuelCapacity;        // ← Duplicates FShipData.FuelCapacity
	int32 CurrentFuel;         // ← Duplicates FShipData.CurrentFuel
	float HullIntegrity;       // ← Duplicates FShipData.HullCondition
	float MaintenanceLevel;
	ECargoShipStatus Status;   // ← Duplicates FShipData.Status
	// ... and more!
}
```

**This is redundant!** The real ship system already has everything needed.

---

## Integration Strategy

### Phase 1: Use FShipData in Logistics (NEXT STEP)

**Replace** `FCargoShip` with direct `FShipData` usage:

```cpp
// LogisticsTypes.h - UPDATED
struct FLogisticsCompany
{
	int32 CompanyId;
	int32 OwningFactionId;    // Company belongs to a faction
	int32 HomeSystemId;       // Home base location

	TArray<int32> OwnedShipIds;  // ← Reference FShipData by ID

	int32 Credits;
	// ... company stats
};

// LogisticsSubsystem.h - UPDATED
class ULogisticsSubsystem
{
	// Store actual ships
	UPROPERTY()
	TMap<int32, FShipData> FleetRegistry;  // ShipId → FShipData

	// Logistics companies reference ships by ID
	UPROPERTY()
	TArray<FLogisticsCompany> Companies;
};
```

### Phase 2: Faction-Owned Fleets

**Each faction's homeworld spawns its own logistics company:**

```cpp
void ULogisticsSubsystem::InitializeLogistics(FUniverseData& UniverseData)
{
	// For each faction homeworld
	for (auto& Pair : UniverseData.FactionHomeSystems)
	{
		int32 FactionId = Pair.Key;
		int32 HomeSystemId = Pair.Value;

		// Create faction logistics company
		FLogisticsCompany Company;
		Company.OwningFactionId = FactionId;
		Company.HomeSystemId = HomeSystemId;

		// Spawn initial fleet of FShipData
		for (int i = 0; i < InitialFleetSize; ++i)
		{
			FShipData Ship = CreateFactionFreighter(FactionId, HomeSystemId);
			FleetRegistry.Add(Ship.ShipId, Ship);
			Company.OwnedShipIds.Add(Ship.ShipId);
		}

		Companies.Add(Company);
	}
}
```

### Phase 3: Component Wear & Breakdown

**Use existing component condition system:**

```cpp
void ULogisticsSubsystem::SimulateShipWear(FShipData& Ship, float DeltaTime)
{
	// Degrade component condition over time/distance
	for (FInstalledComponent& Component : Ship.InstalledComponents)
	{
		Component.CurrentCondition -= WearRate * DeltaTime;

		if (Component.CurrentCondition <= 0.0f)
		{
			// Component failed!
			Ship.Status = EShipStatus::Disabled;
			Ship.bIsOperational = false;

			// Create maintenance request in home system
			CreateMaintenanceRequest(Ship);
		}
	}

	// Recalculate ship stats when components degrade
	UpdateShipStatsFromComponents(Ship);
}
```

### Phase 4: Maintenance Economy Loop

**Ships return home for repairs, consuming resources:**

```cpp
void ULogisticsSubsystem::ProcessMaintenanceRequest(FShipData& Ship, FUniverseData& UniverseData)
{
	// Ship must dock at home location
	if (Ship.CurrentSystemId != GetHomeSystem(Ship.OwnerId))
	{
		// Create route home
		CreateMaintenanceRoute(Ship);
		return;
	}

	// Consume repair goods from market
	FMarketState& Market = GetLocationMarket(Ship.CurrentLocationId, UniverseData);

	for (FInstalledComponent& Component : Ship.InstalledComponents)
	{
		if (Component.CurrentCondition < 100.0f)
		{
			// Consume repair materials
			EGoodType RepairGood = GetRepairGoodForComponent(Component);
			int32 RepairCost = CalculateRepairCost(Component);

			if (Market.Goods.Contains(RepairGood) && 
				Market.Goods[RepairGood].Stock >= RepairCost)
			{
				Market.Goods[RepairGood].Stock -= RepairCost;
				Component.CurrentCondition = 100.0f;
			}
			else
			{
				// Can't repair - market shortage!
				// Ship stays broken until goods arrive
			}
		}
	}

	// Recalculate operational status
	UpdateShipStatsFromComponents(Ship);
}
```

---

## Faction Boundaries

**Ships only trade within faction boundaries:**

```cpp
bool ULogisticsSubsystem::CanShipTrade(const FShipData& Ship, 
										const FTradeRequest& Request,
										const FUniverseData& UniverseData)
{
	// Ship's owner faction
	int32 ShipFactionId = Ship.OwnerId;  // When OwnerType == "Faction"

	// Request location's controlling faction
	const FStarSystemData& System = UniverseData.Systems[Request.DestinationSystemId];
	int32 SystemFactionId = System.ControllingFactionId;

	// Can only trade within same faction
	if (ShipFactionId != SystemFactionId)
	{
		return false;  // Faction boundary - no trade
	}

	return true;
}
```

**This creates emergent behavior:**
- Faction A has surplus food but all ships busy → can't help Faction B
- Faction B starves even though Faction A has ships idle
- Faction B must build more ships (costs resources!)
- Creates pressure to maintain fleet size relative to territory

---

## Implementation Checklist

### Step 1: Refactor LogisticsTypes.h
- [ ] Remove `FCargoShip` struct
- [ ] Update `FLogisticsCompany` to use `TArray<int32> OwnedShipIds`
- [ ] Remove redundant ship fields

### Step 2: Refactor LogisticsSubsystem
- [ ] Add `TMap<int32, FShipData> FleetRegistry`
- [ ] Change `TArray<FCargoShip> Fleet` → use FleetRegistry lookup
- [ ] Update all ship references to use FShipData
- [ ] Include `Ship/ShipData.h` and `Ship/ShipTypes.h`

### Step 3: Faction Fleet Initialization
- [ ] Update `InitializeLogistics()` to create faction companies
- [ ] Spawn FShipData ships per faction homeworld
- [ ] Use `OwnerId` + `OwnerType = "Faction"` pattern
- [ ] Set `CurrentSystemId` and `CurrentLocationId` to home

### Step 4: Trade Matching with Faction Boundaries
- [ ] Update `MatchTrades()` to check ship faction vs system faction
- [ ] Filter routes where factions don't match
- [ ] Add diagnostic logs for cross-faction rejections

### Step 5: Component Wear Simulation
- [ ] Add `SimulateShipWear()` function
- [ ] Degrade component condition over time/distance
- [ ] Detect component failures → set ship Disabled
- [ ] Create maintenance requests

### Step 6: Maintenance Economy
- [ ] Add `ProcessMaintenanceRequest()` function
- [ ] Ships route home when broken
- [ ] Consume repair goods from home market
- [ ] Recalculate stats after repair

### Step 7: Testing & Validation
- [ ] Verify each faction gets its own fleet
- [ ] Verify ships only trade within faction boundaries
- [ ] Verify component wear degrades over time
- [ ] Verify broken ships return home for repair
- [ ] Verify repair consumes market goods

---

## Benefits of Using Real Ship System

1. **No Duplication**: One ship model for players, NPCs, logistics, pirates
2. **Component Economy**: Ships consume repair parts, creating trade demand
3. **Wear & Tear**: Already implemented in component condition system
4. **Faction Specialization**: Different frame types per faction
5. **Power Management**: Ships can lose power if components fail
6. **Crew Integration**: Ships track crew assignments (future)
7. **Manufacturing**: Ship frames and components are economic goods
8. **Consistent Architecture**: Everything uses the same ship/component definitions

---

## Next Actions

**YOU ASKED**: "it should be in the git commit history even"

**CONFIRMED**: Yes! The ship system exists in commit `6ce7a2c` (Sprint 5.5) and is currently on `HEAD` of dev branch.

**RECOMMENDATION**: 
1. Start with Step 1: Refactor `LogisticsTypes.h` to remove `FCargoShip`
2. Then Step 2: Update `LogisticsSubsystem` to use `FShipData` directly
3. Build and validate the integration works
4. Then proceed with faction-owned fleet initialization

This will align the logistics system with the existing ship architecture and set the foundation for faction-owned fleets with wear/tear/maintenance.
