# Sprint 3: Economy Core - Testing Guide

## Overview
This guide covers testing the Sprint 3 economy implementation with two-tier simulation (active vs background).

## Prerequisites
- Universe must be generated (Sprint 1/2 complete)
- Economy subsystem initialized
- At least one system set as active

## Test Sequence

### 1. Basic Economy Initialization

**Steps:**
1. Start PIE (Play in Editor)
2. Open Output Log
3. Run Blueprint command or C++:
   ```cpp
   UUniverseSubsystem* US = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
   US->GenerateUniverse(FUniverseConfig()); // Uses defaults
   US->InitializeEconomy();
   ```

**Expected Results:**
- Log: `[EconomySubsystem] Initialized with 14 goods`
- Log: `[EconomySubsystem] Economy initialized: X locations, Y active markets`
- No errors or warnings
- Every location with production/consumption should have market goods

**Validation:**
```cpp
US->PrintEconomyStats();
```
- Should show total locations, active markets, initial shortages/surpluses
- Active markets > 0
- Some shortages/surpluses expected (initial variance)

---

### 2. Active System Simulation

**Steps:**
1. Set active system (e.g., faction core):
   ```cpp
   US->SetActiveEconomySystem(0); // System ID 0
   ```
2. Wait 10-15 seconds (real time)
3. Check economy stats again:
   ```cpp
   US->PrintEconomyStats();
   ```

**Expected Results:**
- Log: `[EconomySubsystem] Active system set to: [SystemName] (0)`
- Log: `[EconomySubsystem] Caught up system [SystemName]: 0.0 hours in 0 chunks` (first time)
- Game time advances (check with `US->GetCurrentGameTime()`)
- Active system ID matches: `US->GetActiveEconomySystemId()` == 0
- Stock levels change at active system locations
- Prices adjust based on supply/demand
- Shortage/surplus counts may change

**Validation:**
- Query a specific market:
  ```cpp
  FMarketState Market = US->GetMarketState(0, LocationId);
  // Check Market.Goods array for stock/price changes
  ```
- Check shortage status:
  ```cpp
  bool HasShortage = US->HasShortage(0, LocationId, EGoodType::Food);
  ```

---

### 3. Background System Catch-Up

**Steps:**
1. Set system 0 active and wait 30 seconds (game time advances)
2. Switch to system 1:
   ```cpp
   US->SetActiveEconomySystem(1);
   ```
3. Check logs

**Expected Results:**
- Log: `[EconomySubsystem] Caught up system [System1Name]: X hours in Y chunks`
  - X should be approximately (30 seconds of active time)
  - Y chunks = X / 24 (default chunk size)
- System 1 locations now have updated stock/prices matching current game time
- System 0 stops real-time ticking (no longer active)

**Validation:**
- Check last update times:
  ```cpp
  FMarketState Market = US->GetMarketState(1, LocationId);
  // Market.LastUpdateTime should equal current game time
  ```

---

### 4. Shortage/Surplus Detection

**Steps:**
1. Find a location with consumption but no production of Food:
   ```cpp
   TArray<FLocationData> Locations = US->GetLocationsInSystem(ActiveSystemId);
   // Look for location with ConsumptionRate > 0 but ProductionRate = 0 for Food
   ```
2. Wait for stock to deplete (several minutes of game time)
3. Check shortage flag:
   ```cpp
   bool HasShortage = US->HasShortage(SystemId, LocationId, EGoodType::Food);
   ```

**Expected Results:**
- `HasShortage` should return `true` when stock < 25% of target
- Price should increase (2x-4x base price)
- Visualizer (if enabled) should show red when `bShowShortages` is on

**Find All Shortages:**
```cpp
TArray<int32> ShortageLocations = US->FindShortageLocations(EGoodType::Food);
// Returns all location IDs with Food shortages across entire universe
```

---

### 5. Dynamic Pricing

**Steps:**
1. Query price at a location before shortage:
   ```cpp
   float InitialPrice = US->GetGoodPrice(SystemId, LocationId, EGoodType::Food);
   ```
2. Wait for shortage to develop
3. Query price again:
   ```cpp
   float ShortagePrice = US->GetGoodPrice(SystemId, LocationId, EGoodType::Food);
   ```

**Expected Results:**
- `ShortagePrice > InitialPrice`
- Severe shortage (<25% stock): 2x-4x base price
- Moderate shortage (25-75%): 1x-2x base price
- Surplus (>125%): 0.4x-0.8x base price
- Prices change smoothly (30% adjustment per tick, not instant)

---

### 6. Debug Visualizer Economy Modes

**Steps:**
1. Place `AUniverseDebugVisualizer` actor in level
2. Enable visualization: `bShowVisualization = true`
3. Test economy modes:
   - `bShowShortages = true`, `EconomyDisplayGood = Food`
   - `bShowSurpluses = true`, `EconomyDisplayGood = Fuel`
   - `bShowPriceVariance = true`, `EconomyDisplayGood = Medicine`

**Expected Results:**
- **Shortages mode**: Red systems have Food shortages, gray systems don't
- **Surpluses mode**: Green systems have Fuel surpluses, gray systems don't
- **Price variance mode**: 
  - Blue systems = prices below base
  - Red systems = prices above base
  - Intensity = deviation magnitude
- Only one mode active at a time (last enabled wins)

---

### 7. Performance Validation

**Objective:** Confirm only active system simulates in real-time.

**Steps:**
1. Generate large universe (500 systems, ~1000+ locations)
2. Set one system active
3. Monitor frame rate and CPU usage
4. Wait 60 seconds

**Expected Results:**
- Frame rate stable (economy tick every 5 seconds, not every frame)
- CPU usage acceptable (<10% for economy on modern CPU)
- Only active system locations have `Market.bIsActiveSimulation = true`
- All other systems have `bIsActiveSimulation = false`
- Background systems only update when switched to (catch-up simulation)

**Verification:**
```cpp
const FUniverseData& Universe = US->GetUniverseData();
for (const FStarSystemData& System : Universe.Systems)
{
	for (const FLocationData& Location : System.Locations)
	{
		bool IsActive = Location.Market.bIsActiveSimulation;
		// Only active system should have true
	}
}
```

---

### 8. Determinism Test

**Steps:**
1. Generate universe with seed 12345
2. Initialize economy
3. Set system 0 active, wait 60 seconds
4. Record stock/price at specific location
5. Exit PIE
6. Repeat steps 1-3 with same seed
7. Compare results

**Expected Results:**
- Initial market state identical (same stock, prices)
- After same duration, stock/prices should be nearly identical
- Minor variance acceptable due to tick timing, but trends should match

---

## Common Issues

### No Markets After Initialization
- **Cause:** Universe not generated before `InitializeEconomy()`
- **Fix:** Call `GenerateUniverse()` first

### Economy Not Ticking
- **Cause:** No active system set
- **Fix:** Call `SetActiveEconomySystem(SystemId)` after initialization

### Prices Not Changing
- **Cause:** No production/consumption at location, or stock stable
- **Fix:** Wait longer, or check a location with imbalanced production/consumption

### Shortages Never Develop
- **Cause:** Location has production matching consumption
- **Fix:** Look for colonies (high Food consumption, no production) or military bases (high Fuel/Weapons consumption)

---

## Debug Commands Reference

```cpp
// Get subsystem
UUniverseSubsystem* US = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();

// Basic economy info
US->PrintEconomyStats();

// Active system
int32 ActiveId = US->GetActiveEconomySystemId();
US->SetActiveEconomySystem(NewSystemId);

// Game time
double GameTime = US->GetCurrentGameTime(); // seconds

// Market queries
FMarketState Market = US->GetMarketState(SystemId, LocationId);
float Price = US->GetGoodPrice(SystemId, LocationId, EGoodType::Food);
bool Shortage = US->HasShortage(SystemId, LocationId, EGoodType::Medicine);
TArray<int32> ShortageList = US->FindShortageLocations(EGoodType::Fuel);

// Print system content (includes locations)
US->PrintSystemContent(SystemId);
```

---

## Success Criteria

✅ Economy initializes without errors  
✅ Active system ticks in real-time (5-20 second intervals)  
✅ Background systems catch up on-demand  
✅ Shortages/surpluses detected correctly  
✅ Prices respond to supply/demand  
✅ Visualizer economy modes display correctly  
✅ Performance acceptable (no per-frame market ticking)  
✅ Deterministic results with same seed  

---

## Next Steps

After validating Sprint 3:
- **Sprint 4**: Trade routes and cargo hauling
- **Sprint 5**: Player trading UI
- **Sprint 6**: AI traders and market manipulation
