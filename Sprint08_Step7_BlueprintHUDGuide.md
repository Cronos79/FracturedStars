# Sprint 08 - Step 7: Blueprint HUD Creation Guide

**Date:** 2026-01-XX  
**Status:** 🔄 In Progress

---

## Overview
This guide walks through creating `WBP_MainHUD` in the Unreal Editor and implementing the context panel system.

---

## Part A: Create the Blueprint HUD Widget

### 1. Create the Widget Blueprint

1. **Open Unreal Editor**
2. **Navigate to Content Browser**
   - Go to `Content/UI/` (create folder if it doesn't exist)
3. **Create Widget Blueprint:**
   - Right-click → User Interface → Widget Blueprint
   - Name it: `WBP_MainHUD`
4. **Reparent to C++ Class:**
   - Open `WBP_MainHUD`
   - Click "Graph" tab
   - In the top-right, click "Class Settings"
   - Under "Class Options" → "Parent Class"
   - Search for and select `MainHUDWidget` (your C++ class)
   - Click "Compile"

### 2. Create the Layout Structure

Switch to "Designer" tab:

#### Root Canvas Panel (CRITICAL FIRST STEP!)
**If the Designer is empty, you need to add the root Canvas Panel:**

1. **From the Palette** (left side), find **Canvas Panel** under the "PANEL" category
2. **Drag Canvas Panel** onto the empty designer canvas
3. **This becomes your root widget** - everything else goes inside it
4. **In the Hierarchy panel** (bottom left), rename it to `RootCanvas`
5. **Set Anchors:** Click the anchor button, select **Fill Screen** (bottom-right option)
6. **Set Offsets:** All should be 0 (Left=0, Top=0, Right=0, Bottom=0)

**Your Hierarchy should now show:**
```
[WBP_MainHUD]
  └─ RootCanvas (Canvas Panel)
```

#### Top Bar Panel
1. Add **Canvas Panel** to RootCanvas
2. Name: `TopBarPanel`
3. Position: Anchored top, full width
   - Anchors: (0, 0) to (1, 0)
   - Position: X=0, Y=0
   - Size: Width=1920, Height=60
4. Background color: Black with 50% opacity

**Add to TopBarPanel:**
- **Text Block** - Name: `TXT_Date`
  - Position: (20, 15)
  - Text: "July 8, 2094"
  - Font Size: 18

- **Text Block** - Name: `TXT_Time`
  - Position: (200, 15)
  - Text: "00:00:00"
  - Font Size: 18

- **Text Block** - Name: `TXT_Credits`
  - Position: (400, 15)
  - Text: "Credits: 10,000"
  - Font Size: 18
  - Color: Gold

- **Text Block** - Name: `TXT_AlertCount`
  - Position: (1800, 15) - Right side
  - Text: "Alerts: 0"
  - Font Size: 18
  - Color: Orange

#### Context Panel (Right Side)
1. Add **Canvas Panel** to RootCanvas
2. Name: `ContextPanel`
3. Position: Anchored right, full height
   - Anchors: (1, 0) to (1, 1)
   - Offset: X=-400, Y=60 (below top bar)
   - Size: Width=400, Height=fill
4. Background color: Dark gray with 80% opacity

**Add to ContextPanel:**
- **Scroll Box** - Name: `ContextScrollBox`
  - Anchors: Fill parent
  - Position: (0, 0)
  - Size: Fill

**Add to ContextScrollBox (Vertical Box inside):**
- **Vertical Box** - Name: `ContextContent`

**System Selected Content (add to ContextContent):**
- **Text Block** - Name: `TXT_SystemName`
  - Text: "System Name"
  - Font Size: 24
  - Font: Bold
  - Padding: (10, 20, 10, 10)

- **Text Block** - Name: `TXT_SystemOwner`
  - Text: "Owner: Unknown"
  - Font Size: 16
  - Padding: (10, 5, 10, 5)

- **Text Block** - Name: `TXT_RegionType`
  - Text: "Region: Unknown"
  - Font Size: 16
  - Padding: (10, 5, 10, 5)

- **Text Block** - Name: `TXT_Lawfulness`
  - Text: "Lawfulness: 0%"
  - Font Size: 16
  - Padding: (10, 5, 10, 5)

- **Text Block** - Name: `TXT_Population`
  - Text: "Population: Unknown"
  - Font Size: 16
  - Padding: (10, 5, 10, 5)

- **Text Block** - Name: `TXT_LocationCount`
  - Text: "Locations: 0"
  - Font Size: 16
  - Padding: (10, 5, 10, 5)

**Player Overview Content (add to ContextContent):**
- **Text Block** - Name: `TXT_PlayerOverview`
  - Text: "Player Overview"
  - Font Size: 24
  - Font: Bold
  - Padding: (10, 20, 10, 10)

- **Text Block** - Name: `TXT_PlayerCredits`
  - Text: "Credits: 0"
  - Font Size: 18
  - Padding: (10, 10, 10, 10)

- **Text Block** - Name: `TXT_PlayerShips`
  - Text: "Ships: 0"
  - Font Size: 16
  - Padding: (10, 5, 10, 5)

- **Text Block** - Name: `TXT_PlayerCrew`
  - Text: "Crew: 0"
  - Font Size: 16
  - Padding: (10, 5, 10, 5)

#### Bottom Bar Panel
1. Add **Canvas Panel** to RootCanvas
2. Name: `BottomBarPanel`
3. Position: Anchored bottom, full width
   - Anchors: (0, 1) to (1, 1)
   - Offset: Y=-60
   - Size: Width=1920, Height=60
4. Background color: Black with 50% opacity

**Add to BottomBarPanel:**
- **Text Block** - Name: `TXT_AlertFeed`
  - Position: (20, 15)
  - Text: "Alert: System ready"
  - Font Size: 16
  - Color: Yellow

---

## Part B: Implement Blueprint Graph Logic

### 1. Event Construct
1. Switch to "Graph" tab
2. Find or add "Event Construct"
3. Add nodes:

```
Event Construct
	↓
Call "InitializeContextPanel"
```

### 2. Create Custom Event: InitializeContextPanel

1. Right-click → Add Custom Event
2. Name: `InitializeContextPanel`
3. Add logic:

```
InitializeContextPanel
	↓
SetVisibility(TXT_SystemName, Collapsed)
SetVisibility(TXT_SystemOwner, Collapsed)
SetVisibility(TXT_RegionType, Collapsed)
SetVisibility(TXT_Lawfulness, Collapsed)
SetVisibility(TXT_Population, Collapsed)
SetVisibility(TXT_LocationCount, Collapsed)
	↓
SetVisibility(TXT_PlayerOverview, Visible)
SetVisibility(TXT_PlayerCredits, Visible)
SetVisibility(TXT_PlayerShips, Visible)
SetVisibility(TXT_PlayerCrew, Visible)
```

### 3. Override Event: OnSystemSelected

1. Right-click → Add Event → Event On System Selected (from parent class)
2. **Input:** `System Id` (int32)
3. Add logic:

```
Event OnSystemSelected (SystemId)
	↓
Get Universe Subsystem (call parent function GetUniverseSubsystem)
	↓
Branch (IsValid?)
	├─ TRUE:
	│   ↓
	│   Call GetSystemById(SystemId, OutSystem)
	│   ↓
	│   Branch (system found?)
	│   ├─ TRUE:
	│   │   ↓
	│   │   // Hide player overview
	│   │   SetVisibility(TXT_PlayerOverview, Collapsed)
	│   │   SetVisibility(TXT_PlayerCredits, Collapsed)
	│   │   SetVisibility(TXT_PlayerShips, Collapsed)
	│   │   SetVisibility(TXT_PlayerCrew, Collapsed)
	│   │   ↓
	│   │   // Show system details
	│   │   SetVisibility(TXT_SystemName, Visible)
	│   │   SetVisibility(TXT_SystemOwner, Visible)
	│   │   SetVisibility(TXT_RegionType, Visible)
	│   │   SetVisibility(TXT_Lawfulness, Visible)
	│   │   SetVisibility(TXT_Population, Visible)
	│   │   SetVisibility(TXT_LocationCount, Visible)
	│   │   ↓
	│   │   // Set text values
	│   │   SetText(TXT_SystemName, OutSystem.SystemName)
	│   │   ↓
	│   │   // Format owner text
	│   │   Get Faction Subsystem
	│   │   Get Faction By Id (OutSystem.ControllingFactionId)
	│   │   Format Text: "Owner: {FactionName}"
	│   │   SetText(TXT_SystemOwner, formatted text)
	│   │   ↓
	│   │   // Format region type
	│   │   Convert ERegionType to String
	│   │   Format Text: "Region: {RegionType}"
	│   │   SetText(TXT_RegionType, formatted text)
	│   │   ↓
	│   │   // Format lawfulness
	│   │   Multiply OutSystem.Lawfulness by 100
	│   │   Format Text: "Lawfulness: {Value}%"
	│   │   SetText(TXT_Lawfulness, formatted text)
	│   │   ↓
	│   │   // Format population
	│   │   Calculate total population from locations
	│   │   Format Text: "Population: {Count}"
	│   │   SetText(TXT_Population, formatted text)
	│   │   ↓
	│   │   // Format location count
	│   │   Get OutSystem.Locations array length
	│   │   Format Text: "Locations: {Count}"
	│   │   SetText(TXT_LocationCount, formatted text)
	│   │
	│   └─ FALSE:
	│       Print "System not found"
	│
	└─ FALSE:
		Print "Universe subsystem not found"
```

### 4. Override Event: OnSelectionCleared

1. Right-click → Add Event → Event On Selection Cleared (from parent class)
2. Add logic:

```
Event OnSelectionCleared
	↓
// Hide system details
SetVisibility(TXT_SystemName, Collapsed)
SetVisibility(TXT_SystemOwner, Collapsed)
SetVisibility(TXT_RegionType, Collapsed)
SetVisibility(TXT_Lawfulness, Collapsed)
SetVisibility(TXT_Population, Collapsed)
SetVisibility(TXT_LocationCount, Collapsed)
	↓
// Show player overview
SetVisibility(TXT_PlayerOverview, Visible)
SetVisibility(TXT_PlayerCredits, Visible)
SetVisibility(TXT_PlayerShips, Visible)
SetVisibility(TXT_PlayerCrew, Visible)
	↓
// Update player data
Get Player Subsystem
Get Player Profile (GetPlayerId from controller)
	↓
Format Text: "
"
SetText(TXT_PlayerCredits, formatted text)
	↓
Get owned ships count
Format Text: "Ships: {Count}"
SetText(TXT_PlayerShips, formatted text)
	↓
Get owned crew count
Format Text: "Crew: {Count}"
SetText(TXT_PlayerCrew, formatted text)
```

### 5. Optional: Event Tick (for time display)

```
Event Tick
	↓
Get Universe Subsystem
Get Universe Data
Get Current Time
	↓
Format Date: "{Month}/{Day}/{Year}"
SetText(TXT_Date, formatted text)
	↓
Format Time: "{Hour:02d}:{Minute:02d}:{Second:02d}"
SetText(TXT_Time, formatted text)
```

---

## Part C: Configure Player Controller

### 1. Create or Open BP_FracturedStarsPlayerController

1. **If it doesn't exist:**
   - Content Browser → Right-click in `Content/Blueprints/`
   - Blueprint Class → Search "FracturedStarsPlayerController"
   - Select `FracturedStarsPlayerController`
   - Name: `BP_FracturedStarsPlayerController`

2. **Open the Blueprint**

3. **Set Main HUD Class:**
   - Select the root component (Class Defaults)
   - Find "Main HUD Class" property (under UI category)
   - Select `WBP_MainHUD`
   - Compile and Save

### 2. Update Game Mode to Use BP Controller

1. Open `Content/Blueprints/BP_FracturedStarsGameMode` (or create it)
2. If creating:
   - Blueprint Class → FracturedStarsGameMode
   - Name: `BP_FracturedStarsGameMode`
3. Set "Player Controller Class" to `BP_FracturedStarsPlayerController`
4. Compile and Save

### 3. Update Project Settings (if needed)

1. Edit → Project Settings
2. Maps & Modes
3. Default Modes → Default GameMode → `BP_FracturedStarsGameMode`
4. Save

---

## Part D: Testing

### 1. Open MainGameMap
- Content Browser → Content/Maps/MainGameMap.umap

### 2. Play In Editor (PIE)
- Click the green "Play" button

### 3. Test Checklist

**HUD Initialization:**
- [ ] HUD appears on screen
- [ ] Top bar shows placeholder date/time/credits
- [ ] Context panel shows "Player Overview"
- [ ] Bottom bar shows alert feed

**System Selection:**
- [ ] Click a system in the galaxy
- [ ] System highlights gold
- [ ] Context panel switches to system details
- [ ] System name displays
- [ ] Owner/Region/Lawfulness display
- [ ] Population and location count display

**Deselection:**
- [ ] Click empty space
- [ ] System deselects (returns to blue)
- [ ] Context panel switches back to player overview
- [ ] Player credits/ships/crew display

**Console Logs:**
- [ ] Check Output Log for "[PlayerController] Click" events
- [ ] Verify "Selected system" logs
- [ ] Check for any errors

### 4. Common Issues

**HUD not appearing:**
- Check that `MainHUDClass` is set in `BP_FracturedStarsPlayerController`
- Verify controller is set in game mode
- Check viewport layer order (HUD should be on top)

**Subsystem functions not found:**
- Rebuild C++ project
- Refresh Blueprint nodes (right-click → Refresh Nodes)
- Check that `MainHUDWidget.h` has correct UFUNCTION macros

**Text not updating:**
- Verify `OnSystemSelected` event is overridden
- Check that text blocks are named correctly
- Add Print String nodes to debug logic flow

**Systems not spawning:**
- Check Output Log for spawn count
- Verify universe was generated in game instance
- Check that game mode's `BeginPlay` is being called

---

## Part E: Polish (Optional)

### Visual Improvements
- Add border widgets to panels
- Use proper color schemes (dark UI theme)
- Add icons for credits, alerts, etc.
- Implement smooth fade transitions between panel states
- Add scrolling for large system lists

### Functionality Improvements
- Click on locations in the system to show location details
- Add "Focus Camera" button to center on selected system
- Display faction logo/color for owner
- Show trade routes on the map
- Highlight connected systems

### Performance
- Update time display only every second (not every tick)
- Cache subsystem references
- Use visibility binding instead of manual SetVisibility

---

## Expected Result

After completing this guide, you should have:
- ✅ Fully functional HUD with context panel
- ✅ System selection displays detailed information
- ✅ Player overview when nothing is selected
- ✅ Top bar with game state (date/time/credits/alerts)
- ✅ Bottom bar with alert feed
- ✅ Clean, readable interface layout

---

## Troubleshooting

### Blueprint Compilation Errors
```
Error: Could not find function 'GetUniverseSubsystem'
Solution: MainHUDWidget.h must have UFUNCTION(BlueprintCallable) on getter
		  Rebuild C++ project and refresh Blueprint nodes
```

### System Data Not Displaying
```
Problem: Text shows "Unknown" or empty
Solution: Check GetSystemById is being called with correct SystemId
		  Verify OutSystem has valid data
		  Add Print String to debug data values
```

### HUD Not Receiving Events
```
Problem: OnSystemSelected not firing
Solution: Verify WBP_MainHUD is properly reparented to MainHUDWidget
		  Check that controller's SetSelectedSystem is being called
		  Add logs to C++ SetSelectedSystem to confirm execution
```

---

**End of Blueprint HUD Creation Guide**
