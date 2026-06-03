# Phase 2.5: Refactor to RTS Camera Model (Optional)

## Current State (Phase 2)
```cpp
// Character-based terminology (misleading for RTS)
OnPlayerEnterSystem(PlayerController, SystemId);
OnPlayerLeaveSystem(PlayerController, SystemId);
```

## Proposed Refactoring

### Option 1: Rename Functions (Breaking Change)
```cpp
// Old names deprecated/removed
void OnPlayerEnterSystem(...)   → DEPRECATED

// New RTS-appropriate names
void OnPlayerFocusSystem(APlayerController* Player, int32 SystemId);
void OnPlayerUnfocusSystem(APlayerController* Player, int32 SystemId);
```

**Pros**: Clear RTS semantics, no confusion  
**Cons**: Breaking change, requires updating any existing calls  

### Option 2: Add Aliases (Non-Breaking)
```cpp
// Keep old names for backward compatibility
void OnPlayerEnterSystem(APlayerController* Player, int32 SystemId);
void OnPlayerLeaveSystem(APlayerController* Player, int32 SystemId);

// Add new aliases that call the same implementation
UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
void OnPlayerFocusSystem(APlayerController* Player, int32 SystemId)
{
	OnPlayerEnterSystem(Player, SystemId); // Delegate to existing
}

UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
void OnPlayerUnfocusSystem(APlayerController* Player, int32 SystemId)
{
	OnPlayerLeaveSystem(Player, SystemId); // Delegate to existing
}
```

**Pros**: No breaking changes, both APIs available  
**Cons**: Duplicate API surface, potential confusion  

### Option 3: Keep As-Is, Document Intent (Recommended for Now)
```cpp
// Keep existing names, update documentation
/**
 * Register player observing a system (RTS camera focus)
 * In Stellaris-style gameplay, this means player's UI is viewing this system
 * NOT physical player location - this is a strategic camera controller
 */
UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
void OnPlayerEnterSystem(APlayerController* Player, int32 SystemId);
```

**Pros**: Zero code changes, just documentation updates  
**Cons**: API names don't reflect RTS model clearly  

## Recommendation

**For Now**: Use **Option 3** (document intent)
- Phase 2 is already committed and working
- API names are generic enough (Enter/Leave can mean "enter view")
- Avoid breaking changes until we test the system

**Future**: Consider **Option 1** (rename) when adding crew/ships
- Breaking change is acceptable before multiplayer release
- Combine with asset-tracking additions (Phase 2.5 or 3)
- Better to rename once than incrementally

## When to Refactor

**Trigger Points**:
1. When implementing crew/ship systems (need asset tracking)
2. When starting multiplayer testing (clarify API semantics)
3. When fog-of-war phase begins (visibility + presence separation)

**Not Urgent**: Current code works correctly for RTS model, just naming semantics

## Documentation Updates (Immediate)

**Files to Update**:
- ✅ Created `Docs/PlayerPresenceModel_RTS.md` (explains RTS model)
- ⚠️ Update `Docs/NetworkAuthority_Phase2_Complete.md` (add RTS clarification)
- ⚠️ Update code comments in `UniverseSubsystem.h` (clarify "enter" means "focus")

## Future Asset Tracking (Phase 2.5)

When crew/ships are implemented, add:

```cpp
// Asset presence tracking
TMap<int32, TSet<AActor*>> PlayerAssetsInSystem;

UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
void OnPlayerAssetEnterSystem(AActor* Asset, int32 SystemId);

UFUNCTION(BlueprintCallable, Category = "Universe|Multiplayer")
void OnPlayerAssetLeaveSystem(AActor* Asset, int32 SystemId);

UFUNCTION(BlueprintPure, Category = "Universe|Multiplayer")
bool HasPlayerAssetsInSystem(APlayerController* Player, int32 SystemId) const;
```

**System Active Logic**:
```cpp
bool IsSystemActive(int32 SystemId) const
{
	// Active if player is observing OR has assets
	return HasPlayersInSystem(SystemId) || HasAnyAssetsInSystem(SystemId);
}
```

---

**Decision Needed**: Should we refactor now or document-and-defer?

**My Recommendation**: 
- Document now (update comments to clarify RTS model)
- Refactor later (when adding crew/ships, combine with asset tracking)
- Current code works correctly, just naming semantics issue

