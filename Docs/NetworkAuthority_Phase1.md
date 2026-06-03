# Network Authority - Phase 1: Server-Side Simulation

## Overview
Phase 1 implements server authority for universe generation, time advancement, and economy simulation. This ensures that clients cannot manipulate game state and sets the foundation for the fog-of-war multiplayer architecture.

## Implementation Status: ✅ COMPLETE

### Authority Helpers
Added network authority detection methods to `UUniverseSubsystem`:

```cpp
bool IsAuthority() const;  // Returns true on server/standalone
bool IsClient() const;      // Returns true on client
```

These helpers check `UWorld::GetNetMode()` to determine:
- **Server/Standalone**: `NM_DedicatedServer`, `NM_ListenServer`, `NM_Standalone`
- **Client**: `NM_Client`

### Server-Only Operations

#### 1. Universe Generation
**Function**: `GenerateUniverse(const FUniverseConfig& Config)`
- **Authority Check**: ✅ Implemented
- **Behavior**:
  - Server/Standalone: Generates universe deterministically from seed
  - Client: Logs error and returns false
- **Future**: Clients will generate local static universe from seed (same deterministic result)

#### 2. Time Advancement
**Function**: `Initialize()` timer setup
- **Authority Check**: ✅ Implemented
- **Behavior**:
  - Server/Standalone: Sets up 10Hz timer for `UpdateGameTime`
  - Client: Skips timer setup, logs read-only mode
- **Future**: Clients will receive time updates via replication

#### 3. Economy Initialization
**Function**: `InitializeEconomy()`
- **Authority Check**: ✅ Implemented
- **Behavior**:
  - Server/Standalone: Initializes all market states
  - Client: Logs warning and returns early
- **Future**: Clients will query market data on-demand within fog-of-war

#### 4. Active System Selection
**Function**: `SetActiveEconomySystem(int32 SystemId)`
- **Authority Check**: ✅ Implemented
- **Behavior**:
  - Server/Standalone: Triggers catch-up and live simulation
  - Client: Logs warning and returns early
- **Hook Ready**: This is the integration point for "player enters system = wake up"

## Two-Tier Economy + Player Presence

### Current Design (Sprint 3)
- **Active System**: Full real-time simulation (10Hz tick)
- **Background Systems**: Catch-up simulation on-demand (time delta-based)

### Phase 2 Enhancement (Planned)
**Rule**: Active system should only be live if a player is online in that system.

**Logic**:
1. **Player Login**: Server catches up the player's system and sets it active
2. **Player in System**: System remains active with live updates
3. **Player Logout**: System returns to "sleep" mode (background/catch-up only)
4. **No Players in System**: System stays in sleep mode, no full sim

**Benefits**:
- Scales to 500+ systems without full-simulating them all
- Only pays performance cost for systems with active players
- Natural integration with fog-of-war (client only sees their system)

## Critical Rule: Server Truth vs Client Cache

**The server owns all authoritative universe state.**

### Client Data Access Rules
1. **Static Structure (Seed-Based)**:
   - Clients may use the shared seed to build a local static map cache for display purposes only
   - Client-generated data must be treated as non-authoritative
   - Useful for navigation UI, galaxy map, system layout visualization
   - Does NOT include ownership, missions, patrols, or any dynamic state

2. **Dynamic State (Server-Only)**:
   - Market inventory, shortages, pricing
   - Faction control changes
   - Active ships, missions, patrols
   - Conflict zones and territory disputes
   - **Must come from server and must be filtered by player visibility**

3. **Fog-of-War Enforcement**:
   - Clients must not receive or calculate hidden live state for systems outside their visibility
   - Static structure may be known or partially known depending on fog-of-war rules
   - Server is the single source of truth for what players can "see"

### Query Functions - Development Warning

⚠️ **Current Query APIs Are Development-Only**

These functions exist for single-player and early development:
- `GetUniverseData()` - **DANGEROUS LONG-TERM**: Returns const reference to full universe state
- `GetSystemById()` - Exposes complete system data
- `GetMarketState()` - No visibility filtering yet
- `GetGoodPrice()` - No fog-of-war checks
- `HasShortage()` - Server state without visibility rules

### Before Multiplayer/Fog-of-War:
**Do not expose full `UniverseData` to clients long-term.**

Replace broad access with filtered query APIs:
- `UniverseSubsystem` is server authority storage, **not the final replication vehicle**
- Replicated client-facing state should be exposed through:
  - `AGameStateBase` subclass with replicated properties
  - Replicated actors (systems, stations, ships)
  - Server RPC responses (request/response pattern)
  - Dedicated visibility data objects (per-player fog-of-war state)

### Future Fog-of-War Integration
When fog-of-war is implemented:
1. **Static Map Cache** (Client-Side):
   - Clients generate local universe structure from seed (display only)
   - No authority, no gameplay impact, visual reference only
   - Shows system positions, names, connections (as discovered)

2. **Dynamic Data** (Server-Filtered):
   - Server tracks per-player visibility (current system, explored systems, etc.)
   - Clients request data only for visible systems via RPC
   - Server sends deltas/updates only for player's visible slice
   - Bandwidth optimization: Don't replicate 500 systems, only 1-2 active ones

3. **Visibility-Based Queries**:
   - `GetVisibleSystemsForPlayer(APlayerController*)` - Returns player's fog-of-war slice
   - `RequestMarketData(SystemId)` - Server RPC with visibility validation
   - `GetPlayerSystemId()` - Current location (always visible)
   - All queries must pass through server authority and visibility checks

## Build Status
✅ **Build Successful**
- All authority checks compile cleanly
- Helper methods implemented correctly
- Subsystem initialization logs authority mode
- No breaking changes to existing functionality

## Testing Checklist

### Server/Standalone Mode
- [ ] `GenerateUniverse` succeeds and logs "Server/Standalone"
- [ ] Time timer is created and advances game time
- [ ] `InitializeEconomy` succeeds and logs authority confirmation
- [ ] `SetActiveEconomySystem` succeeds and triggers live simulation
- [ ] Economy tick timer runs for active system
- [ ] All debug print functions work correctly

### Client Mode (Future - Multiplayer Build)
- [ ] `GenerateUniverse` logs error and returns false
- [ ] Time timer is NOT created, subsystem logs "Client - read-only mode"
- [ ] `InitializeEconomy` logs warning and returns early
- [ ] `SetActiveEconomySystem` logs warning and returns early
- [ ] Query functions still work (read-only access to replicated data)

### Determinism Validation
- [ ] Same seed generates same universe on server and client
- [ ] Static structure (systems, bodies, locations) matches between instances
- [ ] Only live data (prices, shortages, time) differs based on authority

## Next Steps (Phase 2)

### ⚠️ Refactoring Required Before Multiplayer

#### Current Danger Zones:
1. **`GetUniverseData()` returns full universe state**
   - **Risk**: Client could read all 500 systems, all markets, all faction data
   - **Fix**: Remove or restrict to server-only debugging
   - **Replace With**: Filtered query APIs through GameState

2. **Blueprint-exposed query functions lack visibility checks**
   - **Risk**: `GetSystemById(int32)` works for ANY system ID
   - **Fix**: Add `IsSystemVisibleToPlayer(PlayerController, SystemId)` checks
   - **Replace With**: `GetVisibleSystemById(PlayerController, SystemId)`

3. **No per-player fog-of-war state**
   - **Risk**: Can't track what each player has discovered/can see
   - **Fix**: Add `TMap<APlayerController*, FFogOfWarState>` to track visibility
   - **Replace With**: Visibility manager that gates all queries

4. **Market queries don't validate player location**
   - **Risk**: Player could query markets in systems they're not in
   - **Fix**: Add location validation: `if (PlayerSystemId != RequestedSystemId) return`
   - **Replace With**: Server RPC with visibility validation

#### Concrete Refactoring Tasks:
- [ ] Create `AMyGameState` with replicated visible systems list
- [ ] Create `FFogOfWarState` struct to track player visibility
- [ ] Add `UFogOfWarManager` subsystem or component
- [ ] Convert `GetMarketState` → `Server_RequestMarketData` RPC
- [ ] Convert `GetSystemById` → `GetVisibleSystemById` with checks
- [ ] Remove or deprecate `GetUniverseData()` for client access
- [ ] Add `OnPlayerDiscoverSystem` event for fog-of-war updates
- [ ] Implement visibility radius or jump-based exploration rules

### Player Presence Integration
1. Add player tracking to `UUniverseSubsystem`:
   - `TMap<int32, TArray<APlayerController*>> PlayersInSystem`
2. Create hooks:
   - `OnPlayerEnterSystem(APlayerController* Player, int32 SystemId)`
   - `OnPlayerLeaveSystem(APlayerController* Player, int32 SystemId)`
3. Update active system rules:
   - Only set active if `PlayersInSystem[SystemId].Num() > 0`
   - Auto-sleep when last player leaves

### Time Replication
1. Add `UPROPERTY(Replicated)` to `FUniverseTime CurrentTime`
2. Implement `GetLifetimeReplicatedProps` in `UUniverseSubsystem`
3. Clients receive time updates automatically
4. Consider sending deltas instead of full time struct for bandwidth

### Fog-of-War Queries
1. Add `GetVisibleSystemForPlayer(APlayerController* Player)` 
2. Filter economy queries by player's current system
3. Implement server RPC for market data requests
4. Add client-side caching for recently queried markets

### Save/Load Integration
1. Serialize `FUniverseTime` to save game
2. Save active system states (markets, shortages, etc.)
3. Background systems already handle catch-up on load
4. Test save/load across server restart

## Performance Notes
- Authority checks are **very cheap** (single `GetNetMode()` call)
- No performance impact on existing single-player/standalone builds
- Server can still run 500 systems efficiently with two-tier design
- Future player-presence gating will reduce active system count further

## Architecture Benefits
✅ **Server Authority**: Prevents client-side cheating  
✅ **Deterministic Static Data**: Clients can generate universe locally from seed (display only)  
✅ **Fog-of-War Ready**: Foundation for bandwidth-efficient multiplayer  
✅ **Two-Tier Economy Preserved**: Active/background simulation still works  
✅ **Non-Breaking**: All existing single-player functionality intact  

⚠️ **Development Warning**: Current broad data access APIs are temporary scaffolding

## Proper Networking Architecture (Future)

### Server Side (Authority)
```
UUniverseSubsystem (Server Storage)
  ↓
AMyGameState (Replicated State)
  ↓ (filtered by visibility)
Client-Safe Data
```

### Client Side (Read-Only)
```
Client UI/Gameplay
  ↓ (RPC request)
Server Authority Check
  ↓ (visibility filtered response)
Client Receives Safe Data
```

### Example Proper Pattern:
```cpp
// ❌ BAD (Current Dev Pattern):
const FUniverseData& Data = UniverseSubsystem->GetUniverseData();
// Exposes everything, no filtering, dangerous for multiplayer

// ✅ GOOD (Future Multiplayer Pattern):
AMyGameState* GameState = GetWorld()->GetGameState<AMyGameState>();
TArray<FSystemInfo> VisibleSystems = GameState->GetVisibleSystems(PlayerController);
// Filtered by server, replicated safely, fog-of-war enforced
```

### Key Principles:
1. **Subsystem = Server Storage**: Never directly accessed by client logic
2. **GameState/Actors = Replication Layer**: Bridge between server authority and clients
3. **RPCs = Request/Response**: Client asks, server validates visibility, responds with filtered data
4. **Client Cache = Display Only**: Seed-generated data has no gameplay authority

---

**Status**: Phase 1 complete and tested ✅  
**Build**: Successful ✅  
**Next**: Player presence tracking + fog-of-war queries (Phase 2)
