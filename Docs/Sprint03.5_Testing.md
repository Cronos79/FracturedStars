# Sprint 3.5: Fog of War Testing Guide

## Overview

This document provides test scenarios and validation procedures for the fog-of-war system implemented in Sprint 3.5. The fog-of-war system maintains server-authoritative truth while providing players with filtered, visibility-based knowledge of the universe.

## Architecture Summary

- **Server Authority**: `UUniverseSubsystem` owns the real universe state
- **Player Knowledge**: `UFogOfWarSubsystem` tracks per-player filtered knowledge
- **Client Filtering**: Clients receive only visibility-approved data
- **Visibility Levels**: Hidden → Known → Surveyed → ActiveObservation

## Important: Player Registration Required

**Before calling any fog-of-war functions, you MUST register the player:**

```blueprint
// FIRST: Register the player (SERVER ONLY)
FogOfWarSubsystem->RegisterPlayer(PlayerId);

// THEN: Call any other fog-of-war functions
FogOfWarSubsystem->DiscoverSystem(PlayerId, SystemId);
FogOfWarSubsystem->GetSystemVisibility(PlayerId, SystemId);
// etc.
```

**Why?** The fog-of-war subsystem maintains per-player state that must be initialized before any operations can succeed. Without registration, all queries will return "Player has no fog-of-war state."

**Authority Check**: `RegisterPlayer` automatically checks if running on server and will log a warning if called on client.

**Lazy Initialization**: The fog-of-war subsystem uses lazy initialization for its UniverseSubsystem reference, so it will work correctly regardless of subsystem initialization order.

## Debug/Test Functions

The following BlueprintCallable debug functions are available for testing:

| Function | Purpose | Category |
|----------|---------|----------|
| `DebugPrintPlayerFogState` | Log complete fog-of-war state for a player | Inspection |
| `DebugSetVisibility` | Force-set visibility level for testing transitions | State Manipulation |
| `DebugSimulateInformationAge` | Backdate timestamps to test staleness | Time Simulation |
| `DebugCompareRealVsKnown` | Side-by-side comparison of real vs known data | Validation |
| `DebugGetVisibilitySummary` | Get formatted count string by visibility level | Quick Status |

## Test Scenarios

### 1. Visibility Progression (Hidden → Known → Surveyed → Active)

**Objective**: Verify visibility level transitions work correctly.

**Setup**:
- Start with Player 1 in a fresh game
- **CRITICAL**: Call `RegisterPlayer(1)` FIRST
- Identify a system that is Hidden (never visited)

**Test Steps**:
1. **Call `RegisterPlayer(1)` - REQUIRED FIRST STEP**
2. Call `DebugPrintPlayerFogState(1)` - Confirm system is not in known list
3. Call `GetSystemVisibility(1, SystemId)` - Should return `Hidden`
4. Call `DiscoverSystem(1, SystemId)` - Player learns of the system
5. Call `GetSystemVisibility(1, SystemId)` - Should return `Known`
6. Call `CompleteSurvey(1, SystemId)` - Player surveys the system
7. Call `GetSystemVisibility(1, SystemId)` - Should return `Surveyed`
8. Call `OnPlayerFocusSystem(1, SystemId)` - Player enters the system
9. Call `GetSystemVisibility(1, SystemId)` - Should return `ActiveObservation`
10. Call `OnPlayerUnfocusSystem(1, SystemId)` - Player leaves
11. Call `GetSystemVisibility(1, SystemId)` - Should return `Surveyed`

**Expected Results**:
- Visibility progresses: Hidden → Known → Surveyed → ActiveObservation
- ActiveObservation downgrades to Surveyed when focus is lost
- State persists correctly between calls

**Validation**:
- Use `DebugPrintPlayerFogState(1)` to inspect final state
- Use `DebugCompareRealVsKnown(1, SystemId)` to verify data accuracy

---

### 2. Information Staleness & Aging

**Objective**: Verify information ages correctly and queries reflect outdated data.

**Setup**:
- **CRITICAL**: Call `RegisterPlayer(1)` FIRST
- Player 1 has Surveyed a system (use `CompleteSurvey(1, SystemId)`)
- System has known population and market prices

**Test Steps**:
1. **Call `RegisterPlayer(1)` - REQUIRED FIRST STEP**
2. Call `CompleteSurvey(1, SystemId)` - Ensure system is surveyed
3. Call `DebugCompareRealVsKnown(1, SystemId)` - Note current state
2. Call `DebugSimulateInformationAge(1, SystemId, 30.0)` - Age data by 30 days
3. Call `DebugCompareRealVsKnown(1, SystemId)` - Verify "Age: 30.0 days"
4. Call `GetVisiblePrice(1, SystemId, GoodType)` - Should return 30-day-old price
5. Call `DebugSimulateInformationAge(1, SystemId, 180.0)` - Age to 180 days
6. Call `DebugCompareRealVsKnown(1, SystemId)` - Verify "Age: 180.0 days"

**Expected Results**:
- Information age increases correctly
- Queries return stale data, not current real data
- Age is calculated from `LastObservationTime` vs current game time

**Validation**:
- Log output from `DebugCompareRealVsKnown` shows age and data drift
- Market price queries return frozen historical values

---

### 3. Multi-Player Visibility Independence

**Objective**: Verify each player has independent fog-of-war state.

**Setup**:
- Two players (Player 1 and Player 2)
- Same system visible to both players but at different levels

**Test Steps**:
1. Call `DebugSetVisibility(1, SystemId, Known)` - Player 1 knows system
2. Call `DebugSetVisibility(2, SystemId, Hidden)` - Player 2 does not
3. Call `GetSystemVisibility(1, SystemId)` - Should return `Known`
4. Call `GetSystemVisibility(2, SystemId)` - Should return `Hidden`
5. Call `DebugGetVisibilitySummary(1)` - Note Player 1 counts
6. Call `DebugGetVisibilitySummary(2)` - Note Player 2 counts
7. Call `CompleteSurvey(2, SystemId)` - Player 2 surveys system
8. Call `GetSystemVisibility(1, SystemId)` - Should still return `Known` (unchanged)
9. Call `GetSystemVisibility(2, SystemId)` - Should return `Surveyed`

**Expected Results**:
- Player states are independent
- Changes to one player's visibility do not affect others
- Each player maintains separate known-state snapshots

**Validation**:
- `DebugPrintPlayerFogState` for each player shows different systems/levels
- Cross-player queries confirm isolation

---

### 4. Active Observation & Real-Time Snapshots

**Objective**: Verify ActiveObservation provides current real-time data.

**Setup**:
- Player 1 enters a system (ActiveObservation)
- System has dynamic market prices/population

**Test Steps**:
1. Call `OnPlayerFocusSystem(1, SystemId)` - Player enters system
2. Call `DebugCompareRealVsKnown(1, SystemId)` - Should show Age: 0.0 days, no drift
3. Wait for universe tick (or manually advance time)
4. Call `DebugCompareRealVsKnown(1, SystemId)` - Should still show Age: 0.0 days
5. Call `OnPlayerUnfocusSystem(1, SystemId)` - Player leaves system
6. Wait for time to pass
7. Call `DebugCompareRealVsKnown(1, SystemId)` - Age should increase, drift appears

**Expected Results**:
- During ActiveObservation, age stays at 0.0 and data matches real state
- Snapshot is updated each tick while player focused
- After unfocus, data becomes stale

**Validation**:
- Real vs Known comparison shows zero drift during ActiveObservation
- After leaving, age increases and data freezes

---

### 5. Filtered Economy Queries

**Objective**: Verify economy queries respect visibility and return stale data when appropriate.

**Setup**:
- Player 1 has Surveyed several systems at different staleness levels
- Systems have varying market prices

**Test Steps**:
1. Survey System A, then age data by 10 days
2. Survey System B, then age data by 60 days
3. Survey System C, keep fresh (age 0 days)
4. Call `GetVisiblePrice(1, SystemA, Food)` - Should return 10-day-old price
5. Call `GetVisiblePrice(1, SystemB, Food)` - Should return 60-day-old price
6. Call `GetVisiblePrice(1, SystemC, Food)` - Should return current price
7. Call `GetVisibleShortage(1, SystemA)` - Should return 10-day-old shortage list
8. Call `GetVisibleMarketPrices(1, SystemB, LocationId)` - Should return 60-day-old prices

**Expected Results**:
- Each system returns data from its last observation time
- Queries never return real-time data unless ActiveObservation
- Visibility levels below Known return empty/invalid data

**Validation**:
- Use `DebugCompareRealVsKnown` to confirm drift matches age
- Market queries return frozen historical values

---

### 6. Server Authority & Client Filtering

**Objective**: Verify server maintains truth and clients receive filtered data.

**Setup**:
- Multiplayer session with dedicated server
- Player 1 connected as client

**Test Steps**:
1. On **Server**: Call `DebugPrintPlayerFogState(1)` - Note full state
2. On **Client**: Call `DebugPrintPlayerFogState(1)` - Should show same filtered view
3. On **Server**: Call `DebugSetVisibility(1, SystemId, Surveyed)`
4. On **Client**: Query `GetSystemVisibility(1, SystemId)` - Should reflect change
5. On **Server**: Verify `UniverseSubsystem->IsAuthority()` returns true
6. On **Client**: Verify `UniverseSubsystem->IsAuthority()` returns false
7. On **Client**: Attempt to call server-only functions - Should fail gracefully

**Expected Results**:
- Server maintains authoritative fog-of-war state
- Client queries receive filtered data only
- Server-only functions (OnPlayerFocusSystem, etc.) only execute on server
- Client never sees real-time data for non-active systems

**Validation**:
- Check `IsAuthority()` before critical operations
- Verify client cannot bypass visibility restrictions
- Network logs show filtered queries, not full state sync

---

### 7. Future Asset Registration (Crew/Ship/Station Hooks)

**Objective**: Verify placeholder hooks are present and safe to call.

**Setup**:
- Player 1 with a system at Known visibility

**Test Steps**:
1. Call `RegisterCrewMember(1, SystemId, 101)` - Should execute without error
2. Call `RegisterShip(1, SystemId, 201)` - Should execute without error
3. Call `RegisterStation(1, SystemId, 301)` - Should execute without error
4. Call `GetSystemVisibility(1, SystemId)` - Should still return `Known` (no effect yet)
5. Call `UnregisterCrewMember(1, SystemId, 101)` - Should execute without error
6. Call `UnregisterShip(1, SystemId, 201)` - Should execute without error
7. Call `UnregisterStation(1, SystemId, 301)` - Should execute without error

**Expected Results**:
- All registration functions are safe to call
- Functions log registration but do not change visibility yet (future implementation)
- No crashes or assertion failures

**Validation**:
- Check logs for registration messages
- Confirm visibility remains unchanged (hooks are placeholders)

---

## Manual Testing in PIE (Play In Editor)

### Quick Validation Workflow

1. **Start PIE Session**:
   - Play as Client + Dedicated Server
   - Possess player pawn as Player 1

2. **Inspect Initial State**:
   - Open Blueprint or C++ console
   - Call `FogOfWarSubsystem->DebugPrintPlayerFogState(1)`
   - Verify starting visibility state

3. **Test Visibility Progression**:
   - Move player to a new system
   - Call `DebugGetVisibilitySummary(1)` before and after
   - Verify counts change (Hidden → Known → Active)

4. **Test Information Age**:
   - Leave a system (downgrade to Surveyed)
   - Wait in-game time or use `DebugSimulateInformationAge`
   - Call `DebugCompareRealVsKnown` to see drift

5. **Verify Server/Client Split**:
   - On server window: inspect full state
   - On client window: query same data
   - Confirm client sees filtered view only

---

## Automated Test Coverage (Future)

The following automated tests should be implemented in future sprints:

- **Unit Tests**:
  - Visibility level transitions (Hidden → Known → Surveyed → Active)
  - Information aging calculations
  - Per-player state isolation
  - Economy query filtering

- **Integration Tests**:
  - Server authority enforcement
  - Network replication of filtered data
  - Multi-player visibility independence
  - ActiveObservation snapshot updates

- **Performance Tests**:
  - Fog-of-war tick performance with 1000+ known systems
  - Query performance with stale data lookups
  - Memory usage with multiple players

---

## Known Limitations

1. **Future Asset Hooks**: Crew/ship/station registration is currently a placeholder and does not affect visibility
2. **Information Age Display**: UI does not yet show staleness indicators to players
3. **Network Sync**: Full replication of fog-of-war state to clients not yet implemented (queries are local)
4. **Automated Tests**: No automated test suite yet - manual validation required

---

## Success Criteria

Sprint 3.5 fog-of-war is considered complete when:

- ✅ All visibility levels (Hidden/Known/Surveyed/Active) transition correctly
- ✅ Information ages and queries return stale data
- ✅ Per-player state is independent
- ✅ Server maintains authority, clients are filtered
- ✅ Economy queries respect visibility levels
- ✅ Debug/test functions compile and execute
- ✅ Manual testing validates all scenarios above

---

## References

- **Design Spec**: `Sprint03.5_FogOfWar_Spec.md`
- **Network Architecture**: `Docs/NetworkArchitecture_Design.md`
- **Implementation**: `Source/FracturedStars/Public/Universe/FogOfWarSubsystem.h`
- **Implementation**: `Source/FracturedStars/Private/Universe/FogOfWarSubsystem.cpp`

---

**Document Version**: 1.0  
**Last Updated**: Sprint 3.5 completion  
**Author**: AI Assistant (GitHub Copilot)
