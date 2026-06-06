# Sprint 07: Faction-Owned Logistics System

## The Correct Vision

**Ships belong to FACTIONS, not independent companies.**

### Core Principles

1. **One Fleet Per Faction**
   - Each faction has exactly ONE logistics company (their logistics division)
   - Ships in that fleet ONLY serve that faction's economy
   - Faction A's ships will NEVER trade with Faction B's systems

2. **No Cross-Faction Borrowing**
   - If Faction B has no idle ships, they must WAIT or BUILD MORE
   - Even if Faction A has 30 idle ships, Faction B cannot use them
   - This creates real economic pressure and strategic decisions

3. **Fleet Capacity = Economic Bottleneck**
   - Insufficient ships = trade delays = shortages = economic problems
   - Factions must choose: invest in more ships or accept shortages
   - Ship maintenance costs come from faction budget

4. **Wear, Tear & Breakdown**
   - Ships accumulate damage from jumps, loading, and time
   - Ships can break down mid-route (random chance based on condition)
   - Broken ships must be repaired at faction expense
   - Severely damaged ships may need scrapping and replacement

---

## Implementation Status

### ✅ Completed (Sprint 06-07)

#### 1. Faction-Owned Fleet Structure
**File**: `LogisticsTypes.h`

```cpp
struct FLogisticsCompany
{
	int32 OwningFactionId;  // Which faction owns this fleet (-1 = independent)
	int32 HomeSystemId;     // Faction capital where fleet is based
	float Credits;          // Faction budget allocation for logistics
	// ...
}
```

**Behavior**:
- One company created per faction
- Named "{SystemName} Logistics Division"
- Based at faction's home system
- Fleet size scales with home system population

#### 2. Ship Wear & Breakdown Fields
**File**: `LogisticsTypes.h`

```cpp
struct FCargoShip
{
	float HullIntegrity;           // 0.0-1.0, decreases with use
	float MaintenanceLevel;        // 0.0-1.0, degradesover time
	double TimeSinceLastMaintenance;
	float TotalMaintenanceCost;    // Lifetime maintenance spending
	bool bIsBrokenDown;            // Is ship currently broken?
	double BreakdownStartTime;     // When breakdown started
	// ...
}
```

**Ship Status**:
- Added `UnderRepair` - Ship is broken and being fixed
- Added `Scrapped` - Ship was too damaged and decommissioned

#### 3. Faction Trade Boundaries
**File**: `LogisticsSubsystem.cpp` - `MatchTrades()`

**Logic**:
```cpp
// Get faction that controls the requesting location
int32 RequestFactionId = GetLocationControllingFaction(Request->SystemId, Request->LocationId);

// Only match with exports from SAME faction's territory
int32 ExportFactionId = GetLocationControllingFaction(Opp->SystemId, Opp->LocationId);

if (RequestFactionId != ExportFactionId)
	continue; // NO CROSS-FACTION TRADE
```

**Result**: Ships only move goods within their faction's controlled space.

#### 4. Faction-Only Ship Assignment
**File**: `LogisticsSubsystem.cpp` - `MatchTrades()`

**Logic**:
```cpp
// Find the company that belongs to this faction
for (const FLogisticsCompany& Company : LogisticsCompanies)
{
	if (Company.OwningFactionId == RouteFactionId)
	{
		CompanyId = Company.CompanyId;
		break;
	}
}

// If no ships available from THIS faction = trade delayed
// OTHER factions' idle ships CANNOT be borrowed
```

**Result**: Each faction must manage its own fleet capacity.

---

## Example Scenarios

### Scenario 1: Healthy Faction Economy
**Faction A (Sol System)**
- 5 cargo ships
- 3 active routes
- 2 idle ships ready for new trades
- **Result**: New shortages are quickly addressed

### Scenario 2: Overloaded Fleet
**Faction B (Alpha Centauri)**
- 3 cargo ships
- 3 active routes (all ships busy)
- 0 idle ships
- 4 critical shortages waiting
- **Result**: Trade requests queue up, shortages worsen
- **Solution**: Faction must build more ships (costs resources)

### Scenario 3: Cross-Faction Boundaries
**Faction A (Sol)** has food shortage
**Faction C (Vega)** has food surplus
- Faction A's ships search for exports
- Vega is controlled by Faction C
- **Result**: NO MATCH - factions don't share logistics
- Faction A must find food from its OWN systems or suffer shortage

### Scenario 4: Ship Breakdown
**Faction B Ship "Alpha-FL-2"**
- HullIntegrity: 0.4 (poor condition)
- En route with critical medicine
- Random breakdown check: **FAILS**
- **Result**: Ship stops mid-route, medicine delayed
- **Cost**: Emergency repair from faction budget
- **Impact**: Medicine shortage at destination worsens

---

## Next Steps (Future Sprints)

### 1. Ship Wear Mechanics (Not Yet Implemented)
**Per Jump**:
```cpp
HullIntegrity -= 0.001f;  // Wear from jump stress
MaintenanceLevel -= 0.002f;  // Systems degrade
```

**Per Loading/Unloading**:
```cpp
HullIntegrity -= 0.0005f;  // Cargo handling wear
```

**Per Time**:
```cpp
TimeSinceLastMaintenance += DeltaTime;
if (TimeSinceLastMaintenance > MaintenanceInterval)
{
	MaintenanceLevel -= 0.01f;  // Lack of maintenance degrades systems
}
```

### 2. Breakdown Chance Calculation
```cpp
float BreakdownChance = (1.0f - HullIntegrity) * (1.0f - MaintenanceLevel);
if (FMath::FRand() < BreakdownChance)
{
	Ship.bIsBrokenDown = true;
	Ship.Status = ECargoShipStatus::UnderRepair;
	// Ship stops, route fails, cargo lost
}
```

### 3. Maintenance & Repair System
**Scheduled Maintenance**:
```cpp
Cost = 5000.0f credits
Time = 24 hours (game time)
Restores: MaintenanceLevel = 1.0f
```

**Emergency Repairs** (after breakdown):
```cpp
Cost = 15000.0f credits (3x normal maintenance)
Time = 48 hours
Restores: HullIntegrity += 0.2f, MaintenanceLevel = 0.8f
```

**Hull Overhaul** (extensive refurbishment):
```cpp
Cost = 50000.0f credits
Time = 7 days
Restores: HullIntegrity = 1.0f, MaintenanceLevel = 1.0f
```

### 4. Ship Scrapping & Replacement
**When to Scrap**:
- HullIntegrity < 0.2 (repair no longer cost-effective)
- After multiple breakdowns in short time
- Faction decides to upgrade fleet

**Scrapping Process**:
```cpp
Ship.Status = ECargoShipStatus::Scrapped;
Company.ActiveShipCount--;
// Faction gets small resource refund (scrap metal/parts)
```

**Building New Ships**:
```cpp
Cost: 100,000 credits + materials
Time: 30 days construction
Location: Must be built at shipyard in faction territory
Result: Brand new ship with HullIntegrity = 1.0f
```

### 5. Dynamic Fleet Management
**Faction AI decides**:
- When to build new ships (if economy is profitable)
- When to scrap old ships (if maintenance too expensive)
- How to balance logistics budget vs other faction expenses

**Strategic Pressure**:
- Faction with insufficient fleet = economic suffering
- Faction that overinvests in ships = resource waste
- Optimal fleet size changes based on territory expansion

---

## Economic Impact

### Before (Sprint 05)
- Unlimited abstract trade
- No capacity constraints
- No fleet management required
- Economy was purely supply/demand prices

### Now (Sprint 07)
- **Physical Logistics Constraint**: Limited ships = limited trade capacity
- **Faction Isolation**: Each faction manages own supply chain
- **Maintenance Costs**: Continuous expense to keep fleet operational
- **Strategic Decisions**: Build more ships vs accept shortages
- **Breakdown Risk**: Ships can fail, causing cascading shortages

### Resulting Gameplay
- **For Factions**: Fleet management becomes critical economic skill
- **For Players**: 
  - Can see which factions have strong/weak logistics
  - Can exploit logistics weaknesses in warfare (destroy cargo ships)
  - Can invest in logistics infrastructure for faction they support
  - Multiplayer: Local logistics matters (not waiting for ships from across universe)

---

## Testing Checklist

### 1. Verify Faction Fleet Creation
```
Expected Log:
"Faction 0: Sol Logistics Division - 5 ships deployed at Sol (System 0)"
"Faction 1: Alpha Centauri Logistics Division - 3 ships deployed at Alpha Centauri (System 12)"
...
```

### 2. Verify Cross-Faction Trade Blocking
- Create shortage in Faction A's system
- Create surplus in Faction B's system
- **Expected**: No trade route forms between them
- **Log**: "No logistics company for faction X - cannot serve request"

### 3. Verify Fleet Capacity Limits
- Make all of Faction A's ships busy
- Create new shortage in Faction A's territory
- **Expected**: Trade request remains unmatched
- **Log**: "Faction X company has no idle ships - trade delayed"

### 4. Verify Same-Faction Trade Works
- Create shortage in Faction A System 1
- Create surplus in Faction A System 2
- **Expected**: Trade route forms using Faction A's ships

### 5. Test Ship Status Persistence
- Spawn ships and check initial status: `HullIntegrity = 1.0f, MaintenanceLevel = 1.0f`
- (Future) Run simulation and verify wear accumulates

---

## Summary

Sprint 07 transforms the logistics system from a universal service into a **realistic faction-owned infrastructure** where:

1. **Ships are faction assets**, not shared resources
2. **Fleet capacity limits economic throughput**
3. **Maintenance and breakdowns create ongoing costs**
4. **Factions must actively manage their logistics** or suffer consequences

This creates real strategic depth:
- Poor logistics = economic weakness
- Good logistics = competitive advantage
- Fleet destruction = crippling blow to faction economy
- Building new ships = major investment decision

The system is now ready for **wear mechanics, breakdown simulation, and dynamic fleet expansion** in future sprints.
