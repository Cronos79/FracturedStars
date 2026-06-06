# Sprint 06: Ship Framework Migration Complete

## Overview
Successfully migrated Sprint 06 logistics system from provisional `FCargoShip` model to the real `FShipData` component-based ship framework from Sprint 4 and Sprint 5.5.

## What Changed

### Removed Legacy Code
- **FCargoShip struct** - Replaced with `FShipData` from `Ship/ShipData.h`
- **ECargoShipStatus enum** - Replaced with `EShipStatus` (Docked, InTransit, Active, Disabled, Destroyed)
- **CargoShips array** - Replaced with `FleetRegistry` (TMap<int32, FShipData>)
- **FCargoManifestEntry** - Replaced with `FShipData.CargoInventory` (TMap<EGoodType, int32>)

### Updated to Ship Framework
All logistics code now uses:
- **FShipData** - Complete ship model with frame, components, cargo, fuel, condition
- **FleetRegistry** - Central ship registry keyed by ShipId
- **FLogisticsCompany.OwnedShipIds** - Faction ownership via ship ID references
- **Ship status tracking** - Uses EShipStatus::Docked and EShipStatus::InTransit

## Architecture

### Faction-Owned Fleets
Ships belong to factions via logistics companies:
```cpp
struct FLogisticsCompany
{
	int32 OwningFactionId;           // Which faction owns this company
	int32 HomeSystemId;              // Company home base
	TArray<int32> OwnedShipIds;      // References to ships in FleetRegistry
	// ... stats and accounting
};
```

### Cross-Faction Trade
**Important:** Ships are faction-owned but trade is NOT restricted by faction boundaries.

- Ships can travel anywhere and trade with any system
- Inter-faction trade drives economy, diplomacy, conflict, escorts, piracy
- Faction ownership means:
  - Faction is responsible for ship maintenance and upkeep
  - If all faction ships are busy, faction must produce more or wait
  - Ships wear down and require faction resources to repair

### Ship Creation
Ships are now created using real framework:
```cpp
FShipData NewShip;
NewShip.ShipId = NextShipId++;
NewShip.ShipName = GeneratedName;
NewShip.OwnerId = FactionId;
NewShip.OwnerType = TEXT("Faction");
NewShip.CurrentSystemId = HomeSystemId;
NewShip.Status = EShipStatus::Docked;
NewShip.FrameId = TEXT("StandardFreighter");
NewShip.CargoCapacity = 1000;
NewShip.CargoInventory.Empty();
NewShip.CurrentCargoUsed = 0;
NewShip.bIsOperational = true;
NewShip.HullCondition = 100.0f;

FleetRegistry.Add(NewShip.ShipId, NewShip);
Company.OwnedShipIds.Add(NewShip.ShipId);
```

## Key Functions Updated

### Ship Management
- **CreateCargoShip()** - Now creates `FShipData` and registers in `FleetRegistry`
- **FindIdleShip()** - Searches company `OwnedShipIds` in `FleetRegistry`
- **FindIdleShipNearSystem()** - Same pattern with distance check
- **AssignShipToRoute()** - Uses `FleetRegistry.Find(ShipId)` lookup

### Cargo Operations
- **LoadCargo()** - Uses `Ship->CargoInventory.FindOrAdd()` and `CurrentCargoUsed`
- **DeliverCargo()** - Removes from `CargoInventory`, decreases `CurrentCargoUsed`
- **TickShipTransit()** - Iterates `FleetRegistry` instead of `CargoShips` array
- **AdvanceShipAlongRoute()** - Takes `FShipData&` parameter

### Diagnostics
- **PrintCargoShipStatistics()** - Now "LOGISTICS FLEET STATISTICS"
  - Reports by EShipStatus (Docked, InTransit, Disabled)
  - Shows fleet composition by faction
  - Displays hull condition percentage
- **PrintLogisticsSystemStatus()** - Uses `FleetRegistry.Num()`
- **GetBestAvailableCompany()** - Iterates `OwnedShipIds` instead of linear ship search

## Scope Reminder: Logistics-Only

Sprint 06 does NOT create trade requests. The economy subsystem (Sprint 5) already:
- Generates shortage/surplus flags (`bIsShortage`, `bIsSurplus`)
- Exposes market state via `GetMarketState()`, `HasShortage()`, `FindShortageLocations()`
- Provides pricing via `GetGoodPrice()`

Sprint 06 logistics only:
- Queries existing economy state
- Matches ships to profitable opportunities
- Physically moves cargo through the jump network
- Updates market stock when cargo loads/unloads

## Future Integration Opportunities

### Sprint 5.5 Component Framework
Ships are ready for:
- **Component-based stats** - `FShipFrameDefinition`, `FShipComponentDefinition`
- **Power budgets** - Via `ShipAssemblyLibrary::CalculateShipStats()`
- **Condition tracking** - `HullCondition`, `InstalledComponents` condition
- **Repair/maintenance** - Framework exists, needs logistics hookup

### Faction Fleet Management (Future Sprint)
- Ship production at faction home systems
- Maintenance costs and repair schedules
- Fleet expansion based on trade demand
- Ship wear from jump travel and cargo operations
- Breakdown events requiring rescue/repair

### Physical Ship Spawning (Future Sprint)
- Visual ship actors in active system
- Player interaction with NPC cargo ships
- Convoy/escort missions
- Piracy encounters
- Visible docking/undocking at stations

## Validation

✅ **Build Status:** Successful
✅ **Code Migration:** Complete - all `FCargoShip` references eliminated
✅ **Faction Ownership:** Implemented via `OwnedShipIds`
✅ **Cross-Faction Trade:** Enabled - no faction boundary restrictions
✅ **Cargo System:** Uses `CargoInventory` TMap
✅ **Ship Status:** Uses real `EShipStatus` enum
✅ **Diagnostics:** Updated to report `FleetRegistry` state

## Next Steps

1. **Runtime Testing**
   - Verify ships spawn at faction home systems
   - Confirm cargo loading/unloading updates `CargoInventory`
   - Check route assignment and transit behavior
   - Validate company ownership statistics

2. **Economy Integration**
   - Ensure logistics reads economy shortage/surplus correctly
   - Verify market stock updates affect economy pricing
   - Test catch-up trade network with real ships

3. **Documentation**
   - Update Sprint06_Integration.md with ship framework details
   - Document faction fleet initialization pattern
   - Add ship framework architecture diagram

## Files Changed

### Core Implementation
- `Source/FracturedStars/Public/Universe/LogisticsTypes.h`
  - Removed `FCargoShip` and `ECargoShipStatus`
  - Added `OwnedShipIds` to `FLogisticsCompany`
  - Documented FleetRegistry pattern

- `Source/FracturedStars/Public/Universe/LogisticsSubsystem.h`
  - Added `#include "Ship/ShipData.h"`
  - Added `#include "Ship/ShipTypes.h"`
  - Replaced `CargoShips` with `FleetRegistry`
  - Added `GetShip()` helper methods
  - Updated `AdvanceShipAlongRoute()` signature

- `Source/FracturedStars/Private/Universe/LogisticsSubsystem.cpp`
  - Updated all ship creation/lookup/management functions
  - Migrated cargo operations to `CargoInventory`
  - Updated transit simulation to use `FleetRegistry`
  - Converted diagnostics to report faction fleet state

### Framework Dependencies (Existing)
- `Source/FracturedStars/Public/Ship/ShipData.h` - Ship model
- `Source/FracturedStars/Public/Ship/ShipTypes.h` - Components/frames
- `Source/FracturedStars/Public/Ship/ShipAssemblyLibrary.h` - Stats calculation

## Conclusion

Sprint 06 logistics now fully integrates with the existing ship framework. Faction-owned ships execute cross-faction trade, the economy drives the universe, and the component-based architecture is ready for future maintenance, wear, and visual spawning features.
