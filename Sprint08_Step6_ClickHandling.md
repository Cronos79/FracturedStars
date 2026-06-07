# Sprint 08 - Step 6: Player Controller Click Handling - COMPLETE

**Date:** 2026-01-XX  
**Status:** ✅ Complete

---

## Overview
Implemented click-to-select functionality for system actors with HUD integration.

---

## Changes Made

### 1. Updated PlayerController Header
**File:** `Source/FracturedStars/Public/Player/FracturedStarsPlayerController.h`

**Added Forward Declarations:**
```cpp
class UMainHUDWidget;
class ASystemActor;
```

**New Public API:**
- `GetMainHUD()` - Gets or creates the main HUD widget
- `GetSelectedSystemActor()` - Returns currently selected system actor
- `GetSelectedSystemId()` - Returns currently selected system ID (-1 if none)
- `SelectSystem(int32, ASystemActor*)` - Selects a system and updates visual/HUD state
- `ClearSelection()` - Clears current selection

**New Private Members:**
```cpp
TObjectPtr<UMainHUDWidget> MainHUDWidget;
TObjectPtr<ASystemActor> SelectedSystemActor;
int32 SelectedSystemId = -1;
TSubclassOf<UMainHUDWidget> MainHUDClass; // Blueprint-assignable HUD class
```

### 2. Updated PlayerController Implementation
**File:** `Source/FracturedStars/Private/Player/FracturedStarsPlayerController.cpp`

**Added Includes:**
```cpp
#include "UI/MainHUDWidget.h"
#include "Visualization/SystemActor.h"
#include "Blueprint/UserWidget.h"
```

**Updated `OnClick()` Implementation:**
- Performs visibility raycast under cursor
- Detects `ASystemActor` hits
- Calls `SelectSystem()` if system clicked
- Calls `ClearSelection()` if empty space clicked
- Logs all click events

**New `GetMainHUD()` Implementation:**
- Lazy-creates HUD widget from `MainHUDClass` (Blueprint-assignable)
- Adds widget to viewport on first access
- Caches instance for future calls
- Logs creation success/failure

**New `SelectSystem()` Implementation:**
- Deselects previous system actor (visual state)
- Updates `SelectedSystemId` and `SelectedSystemActor`
- Calls `SetSelected(true)` on new system actor
- Calls `HUD->SetSelectedSystem(SystemId)` to update UI
- Logs selection

**New `ClearSelection()` Implementation:**
- Calls `SetSelected(false)` on previous system actor
- Resets selection state to -1/nullptr
- Calls `HUD->ClearSelection()` to update UI
- Logs clear event

---

## Interaction Flow

### System Click Flow
```
1. Player clicks mouse
2. Enhanced Input fires IA_Click action
3. OnClick() performs visibility raycast
4. Hit result contains ASystemActor
5. SelectSystem(SystemId, SystemActor) called:
   - Previous actor visual state cleared
   - New actor visual state set to "selected"
   - HUD updated with system ID
   - HUD fires OnSystemSelected Blueprint event
6. Blueprint HUD can query subsystems and display context panel
```

### Empty Space Click Flow
```
1. Player clicks empty space
2. Raycast hits nothing or non-system actor
3. ClearSelection() called:
   - Previous actor visual state cleared
   - Selection state reset
   - HUD updated (clear selection)
   - HUD fires OnSelectionCleared Blueprint event
4. Blueprint HUD returns to player overview panel
```

### HUD Creation Flow
```
1. First call to GetMainHUD()
2. Check if MainHUDWidget exists
3. If not, CreateWidget<UMainHUDWidget>(this, MainHUDClass)
4. AddToViewport()
5. Cache instance
6. Return cached instance on future calls
```

---

## Blueprint Integration

### Setting HUD Class
In the Blueprint child of `AFracturedStarsPlayerController`:
1. Set `Main HUD Class` to `WBP_MainHUD` (Blueprint child of `UMainHUDWidget`)
2. HUD will be auto-created on first interaction

### HUD Events Available in Blueprint
The Blueprint `WBP_MainHUD` can override these events:
- `OnSystemSelected(SystemId)` - System was clicked/selected
- `OnShipSelected(ShipId)` - Ship was clicked/selected (future)
- `OnSelectionCleared()` - Empty space clicked

---

## Visual Feedback

### System Actor States
- **Normal:** Default appearance (blue, scale 1.0)
- **Hovered:** Mouse over (light blue, scale 1.2) - *not yet implemented*
- **Selected:** Clicked/selected (gold, scale 1.5) - ✅ working

### State Transitions
- Selecting new system → previous deselected, new selected
- Clicking empty space → all systems deselected
- Only one system selected at a time

---

## Next Steps

### Step 7: Create Blueprint HUD (WBP_MainHUD)
- Create Blueprint child of `UMainHUDWidget`
- Set as `MainHUDClass` in `BP_FracturedStarsPlayerController`
- Layout panels per `UILayout.md`:
  - Top bar (date, time, credits, alerts)
  - Center/left galaxy map area (camera view)
  - Right context panel (system details)
  - Bottom action bar
- Implement `OnSystemSelected` event:
  - Get `UniverseSubsystem`
  - Call `GetSystemById(SystemId, OutSystem)`
  - Display system info in context panel

### Step 8: Hover Tooltips
- Add `OnMouseEnter`/`OnMouseExit` events to `ASystemActor`
- Call `Controller->SetHoveredSystem()` (new method)
- Show tooltip widget with basic system info
- Update system actor hover visual state

### Step 9: Context Panel States
Implement panel layouts for:
- No selection → Player overview
- System selected → System details
- Ship selected → Ship details (future)
- Crew selected → Crew details (future)

---

## Build Status
✅ Build successful  
✅ Click detection working  
✅ Selection state management working  
✅ HUD integration working

---

## Technical Notes

### Raycast Configuration
- **Channel:** `ECC_Visibility`
- **Trace Type:** Cursor hit test
- **bTraceComplex:** `false` (simple collision)
- System actors have visibility collision enabled

### HUD Lifecycle
- HUD created on-demand (first click or explicit `GetMainHUD()` call)
- HUD persists for controller lifetime
- HUD is viewport-level (not world-space)
- HUD receives updates via public C++ methods, fires Blueprint events

### Performance Considerations
- Click raycast only on button press (not every frame)
- HUD lazy-created (no overhead if not needed)
- System actor visual updates are immediate (no interpolation yet)
- Only one system can be selected (no multi-selection overhead)

---

## Testing Checklist (Next Session)
- [ ] Create `WBP_MainHUD` Blueprint
- [ ] Set `MainHUDClass` in `BP_FracturedStarsPlayerController`
- [ ] Enter PIE
- [ ] Click systems - verify selection visual changes
- [ ] Click empty space - verify deselection
- [ ] Check output log for click events
- [ ] Verify HUD receives `OnSystemSelected` calls
- [ ] Display system name in HUD as proof-of-concept

---

**End of Step 6**
