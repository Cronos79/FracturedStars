# Player Presence Model: RTS/Stellaris Camera Controller

## Critical Design Clarification

### NOT Traditional Character-Based
❌ **Wrong Model**: Player has a physical ship/character avatar in a specific system  
✅ **Correct Model**: Player has a **strategic camera view** that can observe any system they have access to (fog-of-war permitting)

### Stellaris-Style Player Model
The player is an **omniscient faction controller** with:
- **Strategic camera**: Can pan/zoom around the galaxy map
- **System view**: Can focus on any discovered/visible system
- **Simultaneous visibility**: Can see multiple systems at once (via UI panels, split views, etc.)
- **Crew/ships**: Are actors in the world, player commands them but is not "in" them

## Revised Player Presence Rules

### What "Player in System" Actually Means

#### Option A: Active Observation (Recommended)
**Definition**: System is actively being viewed/managed by the player's UI/camera

**Triggers for "presence"**:
- Player's main camera is focused on system (galaxy map selected, or system detail view open)
- Player has UI panel showing system detail (market overlay, production queue, etc.)
- Player is issuing commands to ships/stations in system

**Wake-up logic**:
```cpp
// Player selects system on galaxy map
OnPlayerFocusSystem(PlayerController, SystemId);
  -> Wakes up live simulation
  -> Starts real-time price updates
  -> Shows live ship movements

// Player switches to different system
OnPlayerUnfocusSystem(PlayerController, OldSystemId);
OnPlayerFocusSystem(PlayerController, NewSystemId);
  -> Old system sleeps
  -> New system wakes
```

**Benefits**:
- Only simulates what player is actively watching
- Natural integration with UI focus events
- Smooth for multiplayer (each player watches their own systems)

#### Option B: Asset Ownership (Alternative)
**Definition**: System contains player-owned assets (stations, ships, active missions)

**Triggers for "presence"**:
- Player owns at least one station in system
- Player has ships currently in system
- Player has active mission/contract in system

**Wake-up logic**:
```cpp
// Ship enters system
OnPlayerAssetEnterSystem(ShipActor, SystemId);
  -> Track asset count per system
  -> If first asset, wake up system

// Ship leaves system
OnPlayerAssetLeaveSystem(ShipActor, SystemId);
  -> Decrement asset count
  -> If last asset, sleep system
```

**Benefits**:
- More "realistic" (systems with your stuff stay active)
- Supports background missions (cargo ships traveling while you watch other systems)
- Better for AI/automation

#### Option C: Hybrid (Recommended for Production)
**Definition**: System is active if player is **either** observing **or** has assets

**Rules**:
1. **Observed system**: Always active (real-time UI updates)
2. **Asset-only system**: Active if player assets present (background operations)
3. **Neither**: Sleep mode (catch-up on next visit/focus)

**Example**:
- Player watches System A (active - live updates)
- Player has cargo ship traveling through System B (active - path simulation)
- Player has explored but empty System C (sleep - no updates)
- System D is undiscovered (sleep - doesn't exist for player yet)

## Crew System Integration

### Crew Are Actors, Not Player Avatars
```
Player (Strategic Controller)
  ├─ Camera (observes galaxy)
  ├─ Faction State (ownership, resources, tech)
  └─ Commands → Crew/Ships (actors in the world)
		├─ Ship A at System 42
		├─ Ship B traveling to System 117
		└─ Station crew at System 8
```

### Crew Do Not Affect Player "Presence"
**Key Rule**: Crew are **game entities**, player is **strategic observer**.

**Example Flow**:
1. Player opens System 42 detail view → System wakes up (observation)
2. Player orders Ship A to travel to System 117 → Ship begins journey
3. Player closes System 42 view → System sleeps (no more observation)
4. **Ship A continues traveling** (background simulation, no live updates needed)
5. Player opens System 117 view → System wakes up, shows Ship A arriving

**Crew-Driven Wake**:
- If using **Option B or C**, crew ships entering a system can wake it up
- If using **Option A**, crew ships travel in background, system only wakes when player looks

### Fog-of-War + Crew

#### What Crew Reveal
**Static Discovery**:
- Ship visits System X → System X marked as "discovered"
- Player can now see system on galaxy map (name, position, connections)
- Player can **choose** to view System X detail (wakes it up)

**Dynamic Intelligence**:
- Ship in System X → Player can query "what's there right now?" (requires live sim)
- Ship reports back: "3 enemy ships, 1 trade station, prices are X/Y/Z"
- **Without ship present**: Player sees old/cached data (last visit timestamp)

**Intel Decay**:
```
System State:
  Last Visited: 2094-08-15
  Current Time: 2094-09-20
  Intel Age: 36 days (stale)

Player Views System:
  "Market data from 36 days ago. Send a ship to update intel."
```

#### Crew + Multiplayer
**Different players see different systems**:
- Player A has ships in System 42 → Can see live data
- Player B has no assets in System 42 → Sees cached/old data (or nothing if undiscovered)
- Player B has ships in System 117 → Can see live data there
- Server only simulates systems with **any** player observation or assets

## Implementation Adjustments for RTS Model

### Current Phase 2 Code (Character-Based)
```cpp
// Assumes player "enters" and "leaves" systems physically
OnPlayerEnterSystem(PlayerController, SystemId);
OnPlayerLeaveSystem(PlayerController, SystemId);
```

### Revised for RTS/Camera Model

#### Option A: Camera Focus (Simplest)
```cpp
// Player focuses camera on system (UI event)
UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
void OnPlayerFocusSystem(APlayerController* Player, int32 SystemId);

// Player switches away from system (UI event)
UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
void OnPlayerUnfocusSystem(APlayerController* Player, int32 SystemId);
```

**Use Case**: Player opens system detail panel or selects system on galaxy map.

#### Option B: Asset Tracking (More Complex)
```cpp
// Track player-owned assets per system
TMap<int32, TSet<AActor*>> PlayerAssetsInSystem;

// Ship/station enters system
void OnPlayerAssetEnterSystem(AActor* Asset, int32 SystemId);

// Ship/station leaves system
void OnPlayerAssetLeaveSystem(AActor* Asset, int32 SystemId);

// Check if player has any assets in system
bool HasPlayerAssetsInSystem(APlayerController* Player, int32 SystemId) const;
```

**Use Case**: Cargo ships, patrol routes, automated traders.

#### Option C: Hybrid (Recommended)
```cpp
// Track both observation and assets
TMap<int32, FSystemPresence> SystemPresence;

struct FSystemPresence
{
	TArray<APlayerController*> ObservingPlayers;  // Watching via UI
	TSet<AActor*> PlayerAssets;                    // Ships/stations

	bool IsActive() const 
	{ 
		return ObservingPlayers.Num() > 0 || PlayerAssets.Num() > 0; 
	}
};
```

**APIs**:
```cpp
void OnPlayerFocusSystem(APlayerController*, int32 SystemId);
void OnPlayerUnfocusSystem(APlayerController*, int32 SystemId);
void OnAssetEnterSystem(AActor*, int32 SystemId);
void OnAssetLeaveSystem(AActor*, int32 SystemId);
```

## Recommended Implementation Path

### Phase 2.5: Refactor for RTS Model

**Step 1**: Keep existing `OnPlayerEnterSystem/LeaveSystem` for now (backward compatible)

**Step 2**: Add new RTS-appropriate APIs:
```cpp
// Camera/UI focus
void OnPlayerFocusSystem(APlayerController* Player, int32 SystemId);
void OnPlayerUnfocusSystem(APlayerController* Player, int32 SystemId);

// Mark old functions as deprecated or rename them
void OnPlayerEnterSystem(...)  // Rename to OnPlayerFocusSystem
void OnPlayerLeaveSystem(...)  // Rename to OnPlayerUnfocusSystem
```

**Step 3**: Decide on asset tracking:
- **Start Simple**: Only track UI focus (Option A)
- **Add Later**: Asset-based presence when crew/ships are implemented

**Step 4**: Update documentation to reflect camera-controller model

## Multiplayer Scenarios (RTS Model)

### Scenario 1: Co-op (Same Faction)
```
Player A focuses System 42 → Active (live updates for both)
Player B focuses System 117 → Active (live updates for both)
Both players see same faction assets across both systems
Server runs 2 active systems simultaneously
```

### Scenario 2: Competitive (Different Factions)
```
Player A (Faction Red) focuses System 42 → Active (Red's view)
Player B (Faction Blue) focuses System 42 → Active (Blue's view)
Server runs 1 active system with 2 different visibility filters
Each player sees only their own assets + public/detected enemy ships
```

### Scenario 3: Background Operations
```
Player A focuses System 42 (active)
Player A has cargo ship in System 117 (background asset)
  Option A: System 117 stays asleep, ship travels in background
  Option B: System 117 wakes up, runs live sim for ship
  Option C: System 117 wakes up only if ship encounters events
```

## UI Integration Examples

### Galaxy Map Click
```cpp
// UGalaxyMapWidget.cpp
void UGalaxyMapWidget::OnSystemClicked(int32 SystemId)
{
	UUniverseSubsystem* Universe = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();

	// Unfocus previous system
	if (CurrentFocusedSystem >= 0)
	{
		Universe->OnPlayerUnfocusSystem(GetOwningPlayer(), CurrentFocusedSystem);
	}

	// Focus new system
	Universe->OnPlayerFocusSystem(GetOwningPlayer(), SystemId);
	CurrentFocusedSystem = SystemId;

	// Open system detail panel
	ShowSystemDetailPanel(SystemId);
}
```

### System Detail Panel
```cpp
// USystemDetailWidget.cpp
void USystemDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Wake up system when panel opens
	UUniverseSubsystem* Universe = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	Universe->OnPlayerFocusSystem(GetOwningPlayer(), DisplayedSystemId);
}

void USystemDetailWidget::NativeDestruct()
{
	// Sleep system when panel closes
	UUniverseSubsystem* Universe = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	Universe->OnPlayerUnfocusSystem(GetOwningPlayer(), DisplayedSystemId);

	Super::NativeDestruct();
}
```

### Ship Travel Command
```cpp
// ACargoShip.cpp
void ACargoShip::TravelToSystem(int32 TargetSystemId)
{
	// No player "enters" the system - ship travels in background
	// System only wakes if player focuses it OR uses asset-based presence

	int32 CurrentSystem = GetCurrentSystemId();

	// Optional: Asset tracking (if using Option B/C)
	UUniverseSubsystem* Universe = GetWorld()->GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
	Universe->OnAssetLeaveSystem(this, CurrentSystem);
	Universe->OnAssetEnterSystem(this, TargetSystemId);

	// Actual travel logic...
}
```

## Performance Implications

### Option A (Focus Only)
**Active Systems**: 1-3 per player (what they're actively viewing)
- 100 players → ~100-300 active systems max
- Very efficient, only simulates observed systems
- Best for pure strategy game (Stellaris-like)

### Option B (Assets Only)
**Active Systems**: 5-20 per player (where they have ships/stations)
- 100 players → ~500-2000 active systems
- Higher cost, but more "realistic" background operations
- Better for trade/logistics simulation

### Option C (Hybrid)
**Active Systems**: ~10-30 per player (observed + assets)
- 100 players → ~1000-3000 active systems
- Balanced approach
- Recommended for full-featured game

## Fog-of-War Answer

### Does Crew Affect Fog-of-War Design?
**Yes, but not in the way you might think:**

**Player Presence** (Phase 2) = Which systems are **actively simulated**  
**Fog-of-War** (Phase 3) = Which systems player **can see data for**

**Crew Role**:
1. **Exploration**: Crew ships discover new systems (add to fog-of-war map)
2. **Intel Gathering**: Crew in-system provides live data (requires active sim)
3. **Presence Trigger**: Crew can wake up systems (if using asset-based presence)

**Example**:
```
Player has explored 200 systems (fog-of-war: discovered)
Player is viewing 2 systems (active simulation)
Player has ships in 10 systems (asset presence)

Server simulates: 2-12 systems (depending on Option A/B/C)
Player can query data for: 200 systems (discovered via fog-of-war)
  - 12 systems return live data (active)
  - 188 systems return cached/stale data (last visit timestamp)
```

## Recommendation

**For Fractured Stars (Stellaris-style RTS)**:

1. **Start with Option A** (Camera Focus):
   - Simplest to implement
   - Works immediately with UI
   - Efficient performance
   - Rename current functions to `OnPlayerFocusSystem` / `OnPlayerUnfocusSystem`

2. **Add Option C** (Hybrid) when crew/ships are implemented:
   - Track player assets in systems
   - Wake systems with assets for background operations
   - Keep focus-based wake for UI interaction

3. **Fog-of-War Integration**:
   - Crew ships mark systems as "discovered"
   - Player can view galaxy map of all discovered systems
   - Live data only available for active systems
   - Cached/stale data shown for inactive discovered systems

**Next Step**: Should we refactor the Phase 2 code to use "Focus/Unfocus" terminology instead of "Enter/Leave"? This would make the RTS model explicit in the API.

---

**Key Insight**: You're absolutely right - the game is a strategic camera controller, not a character avatar. The "player presence" model needs to reflect **observation and command** rather than **physical location**.

