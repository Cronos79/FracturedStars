# Sprint 06 Follow-Up: Frame Stats Integration

## Status
**Priority:** Medium  
**Type:** Technical Debt / Integration  
**Sprint:** Post-Sprint-06 or Sprint-07  
**Created:** Sprint 06 completion  

## Overview
Replace hardcoded ship frame capacity values in logistics subsystem with real `FShipFrameDefinition` + installed component stat calculations. This will complete the integration between the logistics system and the ship component framework.

## Problem
The logistics subsystem currently uses `GetFrameBaseStats(FName FrameId, ...)` with hardcoded cargo/fuel capacity mappings:

```cpp
// Current implementation (temporary)
void ULogisticsSubsystem::GetFrameBaseStats(FName FrameId, int32& OutCargoCapacity, int32& OutFuelCapacity) const
{
	// Hardcoded frame stats - TODO: Replace with real frame definition lookup
	if (FrameId == "Freighter_Basic" || FrameId == "MediumFreighter")
	{
		OutCargoCapacity = 5000;
		OutFuelCapacity = 1000;
	}
	else if (FrameId == "HeavyFreighter")
	{
		OutCargoCapacity = 10000;
		OutFuelCapacity = 2000;
	}
	// ... etc
}
```

This bypasses the real ship component system that calculates capacity from:
- `FShipFrameDefinition.BaseCargoCapacity`
- Installed cargo modules
- Component bonuses/penalties
- Hull condition modifiers

## Required Changes

### 1. Frame Definition Registry
**File:** `Source/FracturedStars/Private/Universe/LogisticsSubsystem.cpp`

Replace `GetFrameBaseStats()` with proper frame definition lookup:
- Access ship frame registry (if available)
- Look up `FShipFrameDefinition` by `FrameId`
- Use `BaseCargoCapacity` and `BaseFuelCapacity` from definition

### 2. Component Stat Calculation
**File:** `Source/FracturedStars/Private/Universe/LogisticsSubsystem.cpp`

Integrate with ship assembly/component stat calculation:
- Calculate final capacity from base + installed components
- Apply same logic player ships use
- Consider cargo modules, fuel tanks, etc.

### 3. Clean Up CreateCargoShip Signature
**File:** `Source/FracturedStars/Private/Universe/LogisticsSubsystem.cpp`

The `CreateCargoShip()` function currently accepts `CargoCapacity` parameter but ignores it:

```cpp
int32 ULogisticsSubsystem::CreateCargoShip(int32 CompanyId, int32 StartSystemId, 
	const FString& ShipName, int32 CargoCapacity)
{
	// ... frame stats are now derived from FrameId, CargoCapacity param is ignored
}
```

**Options:**
- **Option A:** Remove the `CargoCapacity` parameter entirely (cleaner)
- **Option B:** Use it as a max-capacity constraint/validation
- **Option C:** Use it to select appropriate frame size

**Recommendation:** Option A (remove parameter) since frame selection should be based on company/faction/role, not arbitrary capacity values.

## Files Affected
- `Source/FracturedStars/Private/Universe/LogisticsSubsystem.cpp`
- `Source/FracturedStars/Public/Universe/LogisticsSubsystem.h`
- Potentially: Ship frame registry/lookup utilities

## Dependencies
- Ship frame definition registry system
- Component stat calculation utilities
- Ship assembly integration layer

**Check if these exist:**
- `UShipSubsystem` or similar frame/component registry?
- `CalculateShipStats()` utility functions?
- Frame definition data tables or assets?

## Acceptance Criteria
✅ `GetFrameBaseStats()` removed and replaced with real frame lookup  
✅ Cargo/fuel capacity calculated from `FShipFrameDefinition` + components  
✅ Same stat calculation logic player ships use  
✅ `CreateCargoShip()` signature cleaned up (parameter removed or repurposed)  
✅ NPC cargo ships use real component system  
✅ Build successful, no regressions  
✅ Logistics ship creation logs show real frame-based stats  

## Testing
1. Create NPC cargo ship via logistics initialization
2. Verify cargo capacity matches frame definition + components
3. Verify fuel capacity matches frame definition + components
4. Compare with player ship stat calculation for same frame
5. Test with different frame types (light/medium/heavy freighters)

## Notes
- This is technical debt from Sprint 06 logistics foundation work
- Current hardcoded approach **works correctly** for gameplay
- Priority is medium because it's a design/integration issue, not a functional bug
- Should be done before adding ship wear/tear, upgrades, or component damage systems

## Related Issues
- Sprint 06: Logistics vision corrections (completed)
- Ship framework migration to `FShipData` (completed)
- Future: Ship wear/tear and repair mechanics (depends on real component stats)

---

**Created by:** Sprint 06 completion review  
**Reviewer feedback:** "Still needs later correction" for true frame + component stats  
