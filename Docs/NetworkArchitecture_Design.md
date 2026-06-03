# Network Architecture Design - Fractured Stars

## Core Principle: Server Authority with Client Display Cache

### The Golden Rule
**The server owns all authoritative universe state. Clients never calculate gameplay-affecting data.**

## Data Classification

### 1. Static Structure (Seed-Deterministic)
**Definition**: Universe layout that never changes and is derived purely from the generation seed.

**Examples**:
- System positions (2D coordinates)
- System names
- Jump network connections (which systems link to which)
- Star types and properties
- Planet/moon orbital data
- Asteroid field locations

**Client Access**: 
- ✅ Clients MAY generate this locally from seed for display purposes
- ✅ Used for galaxy map, navigation UI, system visualization
- ⚠️ Client-generated data has NO gameplay authority
- ⚠️ If server and client disagree, server wins

**Why Allow This**: 
- Bandwidth savings: Don't replicate 500 systems worth of static positions
- Instant galaxy map rendering without waiting for server data
- Deterministic generation guarantees client matches server

### 2. Static Content (Seed-Deterministic)
**Definition**: Initial universe content that is generated but doesn't change during gameplay.

**Examples**:
- Celestial body types (star, planet, gas giant, etc.)
- Location types (station, trade hub, mining colony, etc.)
- Initial resource richness values
- Starting population counts
- Production/consumption type assignments

**Client Access**:
- ✅ Clients MAY generate this locally from seed for reference
- ⚠️ Display only - never use for calculations
- ⚠️ Server may override with actual current values

**Why Partial Access**:
- Lets clients show "discovered" system info before visiting
- Supports lore/database entries ("Sol III was originally...")
- Reduces initial data transfer on first login

### 3. Dynamic State (Server-Only)
**Definition**: Live gameplay state that changes during play and affects game mechanics.

**Examples**:
- **Ownership**: Current faction control of systems/stations
- **Economy**: Market inventory, prices, shortages, surpluses
- **Population**: Current population (births/deaths/migration)
- **Ships**: Active ship positions, states, cargo
- **Missions**: Active missions, patrol routes, quest states
- **Conflicts**: Territory disputes, battle zones, faction relations
- **Player Actions**: Trade history, reputation, completed objectives

**Client Access**:
- ❌ Clients MUST NOT generate or calculate this
- ❌ Clients MUST NOT receive this for systems outside visibility
- ✅ Server sends filtered data based on fog-of-war rules
- ✅ Only visible/discovered dynamic state is replicated

**Why Strict Control**:
- Prevents cheating (can't see hidden enemy positions)
- Prevents spoilers (can't peek at undiscovered market prices)
- Enforces fog-of-war gameplay
- Reduces bandwidth (only replicate what's visible)

## Fog-of-War Rules

### Visibility Levels

#### Level 0: Unknown System
- **Client Knows**: Nothing (or just name/position if galaxy map revealed)
- **Client Can See**: System exists on map (grayed out)
- **Client Can Query**: Nothing

#### Level 1: Discovered System (Visited Before)
- **Client Knows**: Static structure (from seed or first visit)
- **Client Can See**: System name, position, jump links, planet count
- **Client Can Query**: Static content only (planet names, types)
- **Client CANNOT See**: Current ownership, markets, ships, conflicts

#### Level 2: Adjacent System (One Jump Away)
- **Client Knows**: Static structure + limited intel
- **Client Can See**: Current faction ownership (if changed)
- **Client Can Query**: Basic faction control info
- **Client CANNOT See**: Market prices, shortages, ship positions

#### Level 3: Current System (Player Present)
- **Client Knows**: Full dynamic state for this system only
- **Client Can See**: All stations, ships, markets, missions
- **Client Can Query**: Market prices, inventory, active patrols
- **Client CANNOT See**: Other systems (even if previously visited)

### Discovery Mechanics
- **Initial State**: Player knows their starting system (Level 3)
- **Jump Travel**: Visiting a system sets it to Level 1 permanently
- **Intel/Scanning**: May reveal Level 1 or Level 2 info without visiting
- **Real-Time Updates**: Only Level 3 (current system) gets live updates
- **On Exit**: Previous system drops to Level 1 (no more live data)

## Network Architecture Layers

### Layer 1: Server Authority (UniverseSubsystem)
**Purpose**: Store and simulate the authoritative universe state

**Responsibilities**:
- Generate universe from seed
- Run economy simulation (active/background)
- Advance game time
- Track all dynamic state
- Validate all player actions

**Access**:
- Server/Standalone only
- Never directly accessed by client code
- Single source of truth

### Layer 2: Replication Layer (GameState/Actors)
**Purpose**: Bridge between server authority and client visibility

**Components**:
- `AMyGameState`: Replicated current system data
- `ASystemActor`: Replicated per-system state (for visible systems)
- `AStationActor`: Replicated station/location state (in current system)
- `AShipActor`: Replicated ship positions/states (visible ships)

**Pattern**:
```cpp
UCLASS()
class AMyGameState : public AGameStateBase
{
	GENERATED_BODY()

	// Replicated to all clients
	UPROPERTY(Replicated)
	int32 CurrentSystemId;

	UPROPERTY(Replicated)
	FUniverseTime CurrentTime;

	UPROPERTY(Replicated)
	TArray<FSystemVisibilityInfo> VisibleSystems;

	// Server-only
	FFogOfWarState* GetPlayerFogOfWar(APlayerController* Player);
};
```

### Layer 3: Request/Response (RPCs)
**Purpose**: Client requests data, server validates and responds

**Pattern**:
```cpp
// Client requests market data
UFUNCTION(Server, Reliable)
void Server_RequestMarketData(int32 SystemId, int32 LocationId);

void AMyPlayerController::Server_RequestMarketData_Implementation(int32 SystemId, int32 LocationId)
{
	// Validate player can see this system
	if (!CanPlayerSeeSystem(SystemId))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player requested market data for invisible system!"));
		return;
	}

	// Get data from authority
	UUniverseSubsystem* Universe = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	FMarketState Market = Universe->GetMarketState(SystemId, LocationId);

	// Send back to client
	Client_ReceiveMarketData(SystemId, LocationId, Market);
}

UFUNCTION(Client, Reliable)
void Client_ReceiveMarketData(int32 SystemId, int32 LocationId, FMarketState Market);
```

### Layer 4: Client Cache (Display Only)
**Purpose**: Fast local data for UI/visualization without server round-trip

**Components**:
- Seed-generated static universe structure
- Cached market data (timestamp + expiry)
- Discovered system list
- Navigation path cache

**Rules**:
- ❌ Never use for gameplay decisions
- ✅ Use for UI rendering only
- ⚠️ Always verify with server before actions
- ⚠️ Assume cache may be stale/incorrect

## Implementation Phases

### Phase 1: Server Authority ✅ COMPLETE
- Authority checks (IsAuthority/IsClient)
- Server-only generation, time, economy
- Foundation for filtering

### Phase 2: Player Presence Tracking (NEXT)
- Track which players are in which systems
- Active system only if player present
- Login catch-up + live updates
- Logout sleep mode

### Phase 3: Fog-of-War State
- `FFogOfWarState` per player
- Track discovered systems (Level 1+)
- Track current system (Level 3)
- Visibility validation helpers

### Phase 4: Filtered Query APIs
- Replace `GetUniverseData()` with filtered queries
- Add `GetVisibleSystemById(PlayerController, SystemId)`
- Add `Server_RequestMarketData` RPC pattern
- Remove broad access functions

### Phase 5: Replication Layer
- Create `AMyGameState` with replicated time
- Create `ASystemActor` for visible systems
- Replicate only current system dynamic state
- Implement RPC request/response pattern

### Phase 6: Client Seed Cache
- Client-side universe generation from seed
- Display-only galaxy map
- Navigation UI without server queries
- Clear "not authoritative" warnings

### Phase 7: Bandwidth Optimization
- Delta compression for market updates
- Event-based replication (only on change)
- Batch RPC responses
- Prioritize current system over adjacent systems

## Anti-Patterns to Avoid

### ❌ DON'T: Expose Full Universe to Clients
```cpp
// BAD: Returns everything, no filtering
const FUniverseData& GetUniverseData() const;

// Client could read:
for (const FStarSystemData& System : UniverseData.Systems) {
	// See all 500 systems, all markets, all ownership!
}
```

### ✅ DO: Filter by Visibility
```cpp
// GOOD: Only returns what player can see
TArray<FSystemInfo> GetVisibleSystems(APlayerController* Player) const;

// Server validates visibility first
if (!FogOfWarManager->CanPlayerSeeSystem(Player, SystemId)) {
	return TArray<FSystemInfo>(); // Empty, no data
}
```

### ❌ DON'T: Trust Client Calculations
```cpp
// BAD: Client calculates price and tells server
float ClientPrice = CalculateMarketPrice(Supply, Demand);
Server_BuyGood(GoodType, ClientPrice); // Exploitable!
```

### ✅ DO: Server Calculates Everything
```cpp
// GOOD: Client requests, server validates and calculates
Server_RequestBuyGood(GoodType, Quantity);

void Server_RequestBuyGood_Implementation(EGoodType Good, int32 Qty) {
	// Server calculates price
	float Price = UniverseSubsystem->GetGoodPrice(SystemId, LocationId, Good);

	// Server validates player can afford
	if (PlayerCredits < Price * Qty) return;

	// Server performs transaction
	ProcessTrade(Good, Qty, Price);
}
```

### ❌ DON'T: Replicate Everything to Everyone
```cpp
// BAD: All clients get all system updates
UPROPERTY(Replicated)
TArray<FStarSystemData> AllSystems; // 500 systems * N properties * 60 Hz = 💥
```

### ✅ DO: Replicate Only Visible State
```cpp
// GOOD: Only current system + adjacent (if Level 2 visibility)
UPROPERTY(Replicated)
FSystemDynamicState CurrentSystemState; // 1 system worth of data

UPROPERTY(Replicated)
TArray<FSystemBasicInfo> AdjacentSystems; // Just faction ownership
```

## Security Considerations

### Cheating Vectors to Block
1. **Market Spoofing**: Client claims different prices than server
   - **Solution**: Server always calculates prices, client only displays
2. **Invisible Ship Detection**: Client receives hidden ship positions
   - **Solution**: Only replicate ships in current system
3. **Fog-of-War Bypass**: Client generates seed and reads hidden systems
   - **Solution**: Static structure OK, dynamic state must come from server
4. **Trade Exploitation**: Client requests trades for systems they're not in
   - **Solution**: Validate `PlayerSystemId == RequestedSystemId` on server

### Validation Pattern (Every Server RPC)
```cpp
void Server_SomeAction_Implementation(int32 SystemId, ...)
{
	// 1. Validate player controller
	if (!IsValid(this)) return;

	// 2. Validate visibility (fog-of-war)
	if (!CanPlayerSeeSystem(SystemId)) {
		UE_LOG(LogTemp, Warning, TEXT("FOW violation"));
		return;
	}

	// 3. Validate player location (if required)
	if (RequiresPresence && GetPlayerSystemId() != SystemId) {
		UE_LOG(LogTemp, Warning, TEXT("Location violation"));
		return;
	}

	// 4. Get authoritative data
	UUniverseSubsystem* Universe = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();

	// 5. Validate action is legal
	if (!Universe->CanPerformAction(...)) {
		return;
	}

	// 6. Perform action on server
	Universe->PerformAction(...);

	// 7. Send result to client
	Client_ActionResult(...);
}
```

## Performance Targets

### Bandwidth Budget (Per Client)
- **Current System**: ~10 KB/s (full dynamic state, 10Hz updates)
- **Adjacent Systems**: ~1 KB/s (basic info, 1Hz updates)
- **Time/Global State**: ~100 bytes/s (universal time, factions)
- **Events**: ~5 KB/s (ship movement, combat, trades)
- **Total Target**: <20 KB/s per client sustained

### Replication Strategies
- **Current System Dynamic State**: Every 100ms (10Hz)
- **Market Prices**: On change only (event-driven)
- **Ship Positions**: 10Hz (if moving), 1Hz (if stationary)
- **Time**: 1Hz (sufficient for game time display)
- **Faction Ownership**: On change only (rare)

### Scalability Goals
- Support 100+ concurrent players
- Each player sees 1 active system + up to 10 adjacent systems
- Server simulates 1-10 active systems (where players are)
- 490+ systems remain in background/catch-up mode
- Target: <10ms per active system per tick

## Summary

| Aspect | Server | Client |
|--------|--------|--------|
| **Universe Generation** | Authority | Display cache (optional) |
| **Static Structure** | Source of truth | May cache from seed |
| **Dynamic State** | Exclusive owner | Receives filtered subset |
| **Economy Simulation** | Runs sim | Displays results |
| **Time Advancement** | Advances time | Receives updates |
| **Fog-of-War** | Enforces rules | Trusts server |
| **Market Queries** | Calculates/validates | Requests via RPC |
| **Trade Actions** | Processes transactions | Sends requests |
| **Ship Positions** | Simulates movement | Renders replicated state |

**Bottom Line**: Server does everything. Clients display what server allows them to see.

---

**Status**: Design document - implementation across Phases 2-7  
**Current Phase**: Phase 1 complete, Phase 2 next (player presence)  
**Critical**: Refactor broad data access before multiplayer testing
