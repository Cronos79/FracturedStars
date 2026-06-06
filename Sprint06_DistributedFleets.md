# Sprint 06: Distributed Fleet Placement (SUPERSEDED)

## ⚠️ ARCHITECTURAL CHANGE
**This approach was replaced by Sprint 07's faction-owned logistics system.**

**Original Plan**: Distribute independent companies across strategic hubs  
**Actual Implementation**: One logistics company per faction, ships only serve owning faction

**See**: `Sprint07_FactionOwnedLogistics.md` for the correct implementation

---

## What This Sprint Originally Did (Now Replaced)
**Feature**: Logistics companies are now based in specific systems, and ships spawn at their home locations instead of a single universal origin.

**Motivation**: 
- Realistic interstellar economy where factions control their own logistics infrastructure
- Prepares for Sprint 07 faction ownership, maintenance, and upkeep
- Reduces unrealistic scenarios where all ships start in System 0 while trade happens across the universe

---

## What Changed

### 1. Company Home Systems (`FLogisticsCompany`)
Added two new fields to `LogisticsTypes.h`:

```cpp
// Sprint 6: Home system where company is based (ships start here)
int32 HomeSystemId = 0;

// Sprint 7: Owning faction ID (-1 = independent, 0+ = faction-owned)
int32 OwningFactionId = -1;
```

**Purpose**:
- `HomeSystemId`: Where the company's fleet is based and where new ships spawn
- `OwningFactionId`: Future faction integration (Sprint 07)

---

### 2. Smart Fleet Distribution (`FindLogisticsHubSystems`)
New helper function finds the best systems for company headquarters:

**Priority Order**:
1. **Faction home systems** - Capital worlds from `FUniverseData.FactionHomeSystems`
2. **Trade hubs** - Systems with `ELocationType::TradeHub` locations
3. **High-population systems** - Economic activity centers
4. **Fallback** - Any system with locations

**Result**: Each company gets its own strategic home base instead of all starting in System 0.

---

### 3. Distributed Ship Creation
Fleet initialization now:
1. Calls `FindLogisticsHubSystems()` to get N hub systems for N companies
2. Assigns each company to a different hub
3. Creates all ships **at the company's home system**
4. Logs where each company is based

**Log Example**:
```
Colonial Freight Corporation based in Sol (System 0) - 3 ships deployed
Universal Cargo Services based in Alpha Centauri (System 12) - 3 ships deployed
Frontier Logistics based in Vega (System 47) - 3 ships deployed
```

---

### 4. Smart Ship Selection (`FindIdleShipNearSystem`)
When matching trade routes, the system now:
1. Finds the **closest idle ship** to the route source using jump distance
2. Prefers ships already at the source system (distance = 0)
3. Falls back to any idle ship if needed

**Benefits**:
- Reduces empty repositioning transit
- Ships work closer to their home systems
- More realistic fleet behavior (local operations)

**Code**:
```cpp
int32 ShipId = FindIdleShipNearSystem(CompanyId, Route.SourceSystemId, UniverseData);
```

---

## Testing

### Verify Fleet Distribution
1. Initialize universe and logistics
2. Check logs for company home systems:
   ```
   Colonial Freight Corporation based in Sol (System 0) - 3 ships deployed
   Universal Cargo Services based in Kepler-442 (System 73) - 3 ships deployed
   ```
3. Verify ships are NOT all in System 0

### Verify Smart Ship Selection
1. Wait for trade routes to form
2. Check that routes originating from a company's home system use local ships
3. Monitor log messages about ship selection

### Expected Behavior
- Companies spread across universe (not all in System 0)
- Ships prefer local routes (less empty repositioning)
- Trade network still forms properly despite distributed fleets

---

## Next Steps (Sprint 07: Faction-Owned Logistics)

### 1. Faction Ownership
- Link companies to their home system's dominant faction
- Set `OwningFactionId` during initialization
- Faction budgets fund ship maintenance and expansion

### 2. Ship Maintenance & Wear
- **Wear and Tear**: Ships accumulate damage per jump/delivery
- **Breakdown**: Ships can fail and require repair
- **Maintenance Costs**: Factions pay upkeep per ship per cycle

### 3. Dynamic Fleet Management
- Factions buy new ships when profitable
- Replace broken-down ships
- Expand/contract fleets based on economy

### 4. Multiplayer Considerations
- Ships spawn at local stations (no universe-crossing delays)
- Factions manage regional supply chains
- Players interact with local logistics instead of distant monopolies

---

## Technical Notes

### Files Modified
- `Source/FracturedStars/Public/Universe/LogisticsTypes.h`
  - Added `HomeSystemId` and `OwningFactionId` to `FLogisticsCompany`
- `Source/FracturedStars/Public/Universe/LogisticsSubsystem.h`
  - Added `FindLogisticsHubSystems()` declaration
  - Added `FindIdleShipNearSystem()` declaration
- `Source/FracturedStars/Private/Universe/LogisticsSubsystem.cpp`
  - Implemented `FindLogisticsHubSystems()` with prioritized hub selection
  - Implemented `FindIdleShipNearSystem()` for proximity-based ship selection
  - Updated `InitializeLogistics()` to distribute companies across hubs
  - Updated `MatchTrades()` to use smart ship selection

### Performance Impact
- Minimal: One BFS pathfinding call per ship selection (only when idle ships exist)
- Hub selection runs once at initialization
- No impact on existing catch-up or economy systems

### Compatibility
- Existing `FindIdleShip(CompanyId)` still works (backward compatible)
- New `FindIdleShipNearSystem()` is optional enhancement
- Blueprint callable for debugging/testing

---

## Summary
Sprint 06 distributed fleets transform the logistics system from a centralized monopoly into a realistic interstellar network where companies operate from strategic hubs. This sets the foundation for Sprint 07's faction-owned infrastructure, ship maintenance, and dynamic fleet management. The economy now behaves like a real distributed system where geography and ownership matter.
