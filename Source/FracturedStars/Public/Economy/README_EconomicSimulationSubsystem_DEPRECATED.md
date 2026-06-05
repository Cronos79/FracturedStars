# UEconomicSimulationSubsystem - DEPRECATED

## Status: DEPRECATED

This subsystem has been **deprecated** and replaced by the existing `UEconomySubsystem`.

## Reason for Deprecation

During Sprint 5 implementation, we discovered that `UEconomySubsystem` already had a sophisticated two-tier active/inactive system architecture integrated with:
- `UniverseSubsystem::OnPlayerEnterSystem/OnPlayerLeaveSystem` for player presence tracking
- `UFogOfWarSubsystem` for observation capability tracking
- Timer-based real-time ticking for active systems
- Time-compressed catch-up simulation for inactive systems

Rather than duplicating this infrastructure, the Sprint 5 production/consumption logic was integrated directly into `UEconomySubsystem`.

## Migration Path

### Old Approach (UEconomicSimulationSubsystem)
```cpp
UEconomicSimulationSubsystem* EconSim = GetGameInstance()->GetSubsystem<UEconomicSimulationSubsystem>();
EconSim->StartEconomicSimulation();
EconSim->ActivateSystem(SystemId);
```

### New Approach (UEconomySubsystem)
```cpp
// Economic simulation is automatically managed by UEconomySubsystem
// Player presence tracking triggers activation via UniverseSubsystem

// The system activates automatically when first player enters:
UniverseSubsystem->OnPlayerEnterSystem(PlayerController, SystemId);
// This calls: EconomySubsystem->SetActiveSystem(SystemId)
// Which triggers: CatchUpSystemEconomy + TickActiveSystemEconomy timer
```

## What Remains

### Still Valid and In Use:
- **`UEconomicProfileLibrary`** - Location-type economic templates (Agricultural, Mining, Industrial, etc.)
  - Used by `UEconomySubsystem` for production/consumption behavior
  - Remains the authoritative source for location economic profiles

### Deprecated:
- **`UEconomicSimulationSubsystem`** - Redundant subsystem with duplicate two-tier tracking
  - All production/consumption logic moved to `UEconomySubsystem`
  - System activation tracking now uses existing `UniverseSubsystem` infrastructure
  - Timer management now uses existing `UEconomySubsystem` timer

## Current Architecture

```
UniverseSubsystem (Player Presence)
		|
		| OnPlayerEnterSystem/LeaveSystem
		v
	EconomySubsystem (Two-Tier Simulation)
		|
		+-- SetActiveSystem()
		|     |
		|     +-- CatchUpSystemEconomy() [Time-compressed batch]
		|     +-- TickActiveSystemEconomy() [Real-time timer]
		|
		+-- SimulateLocationEconomy()
			  |
			  +-- UpdateProductionEfficiency() [Sprint 5]
			  +-- TickProduction() [Sprint 5]
			  +-- TickConsumption() [Sprint 5]
			  +-- UpdateShortagesAndSurpluses()
			  +-- UpdateMarketPrices()
			  +-- CalculateEconomicStress() [Sprint 5]
```

## Files to Keep

✅ **Keep**:
- `Source/FracturedStars/Public/Universe/EconomyTypes.h` - Sprint 5 data types
- `Source/FracturedStars/Public/Economy/EconomicProfileLibrary.h/.cpp` - Location templates
- `Source/FracturedStars/Public/Universe/EconomySubsystem.h/.cpp` - Main economy manager

❌ **Can be removed** (but kept for reference):
- `Source/FracturedStars/Public/Economy/EconomicSimulationSubsystem.h/.cpp`

## Lessons Learned

When adding new features:
1. **Check existing infrastructure first** before creating parallel systems
2. The FogOfWar and Economy subsystems already had player presence integration
3. Two-tier simulation (active real-time / inactive catch-up) was already implemented
4. Reusing existing systems reduces complexity and maintains architectural consistency

## Future Reference

If you need to understand how Sprint 5 economic simulation works, refer to:
- `UEconomySubsystem::SimulateLocationEconomy()` - Main simulation loop
- `UEconomicProfileLibrary` - Location-type economic behavior
- `UniverseSubsystem::OnPlayerEnterSystem()` - Activation trigger
- Sprint 5 spec: `Sprint05_EconomicSimulation_Spec.md`
