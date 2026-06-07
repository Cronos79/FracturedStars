# Blueprint HUD Quick Reference Card

**For:** `WBP_MainHUD` Blueprint Implementation  
**Parent Class:** `MainHUDWidget` (C++)

---

## Available C++ Functions (BlueprintCallable)

### Subsystem Getters
All return valid subsystem reference (or nullptr if not available):

```
GetUniverseSubsystem() → UUniverseSubsystem*
GetEconomySubsystem() → UEconomySubsystem*
GetLogisticsSubsystem() → ULogisticsSubsystem*
GetFactionSubsystem() → UFactionSubsystem*
GetFogOfWarSubsystem() → UFogOfWarSubsystem*
GetPlayerSubsystem() → UPlayerSubsystem*
```

### Selection State Getters
```
GetSelectedSystemId() → int32 (returns -1 if none)
GetSelectedShipId() → int32 (returns -1 if none)
```

### Helper Functions
```
GetOwningFracturedStarsPlayerController() → AFracturedStarsPlayerController*
	- Gets the player controller that owns this HUD

FormatNumber(int32 Number) → FString
	- Example: 1000000 → "1,000,000"

FormatPercentage(float Value, int32 DecimalPlaces = 0) → FString
	- Example: 0.756 → "76%" (0 decimals)
	- Example: 0.756 → "75.6%" (1 decimal)

GetRegionTypeDisplayName(ERegionType RegionType) → FString
	- Converts enum to readable string
	- FactionCore → "Faction Core"
	- Lawless → "Lawless"
	- etc.
```

---

## Events to Override

### Event On System Selected (SystemId: int32)
**When:** Player clicks a system  
**Use:** Update context panel to show system details

**Example Blueprint Logic:**
```
1. Get Universe Subsystem
2. Call GetSystemById(SystemId, OutSystem)
3. Hide player overview widgets
4. Show system detail widgets
5. Set text values from OutSystem data
```

### Event On Selection Cleared
**When:** Player clicks empty space  
**Use:** Return context panel to player overview

**Example Blueprint Logic:**
```
1. Hide system detail widgets
2. Show player overview widgets
3. Get Player Subsystem
4. Query player data (credits, ships, crew)
5. Update text values
```

### Event On Ship Selected (ShipId: int32)
**When:** Player clicks a ship (future implementation)  
**Use:** Show ship details

---

## Common Blueprint Node Patterns

### Pattern 1: Get System Data
```
Get Universe Subsystem (from parent)
	↓
Branch (IsValid?)
	├─ TRUE:
	│   ↓
	│   Call GetSystemById(SystemId, OutSystem)
	│   ↓
	│   Use OutSystem.SystemName, OutSystem.Lawfulness, etc.
	│
	└─ FALSE:
		Print "Universe subsystem not found"
```

### Pattern 2: Get Faction Name from System
```
Get Faction Subsystem
	↓
Get System.ControllingFactionId
	↓
Call GetFactionData(FactionId)
	↓
Branch (Valid?)
	├─ TRUE: Use FactionData.FactionName
	└─ FALSE: Use "Independent" or "Unknown"
```

### Pattern 3: Format Population
```
Get System.Locations (array)
	↓
ForEach Loop
	↓
	Add Location.Population to Total
	↓
Call FormatNumber(Total)
	↓
Format Text: "Population: {Formatted}"
```

### Pattern 4: Format Lawfulness
```
Get System.Lawfulness (float 0.0-1.0)
	↓
Call FormatPercentage(Lawfulness, 0)
	↓
Format Text: "Lawfulness: {Result}"
	(Example output: "Lawfulness: 75%")
```

### Pattern 5: Get Region Type String
```
Get System.RegionType (enum)
	↓
Call GetRegionTypeDisplayName(RegionType)
	↓
Format Text: "Region: {Name}"
	(Example output: "Region: Faction Core")
```

---

## Widget Naming Convention

Use these exact names in your Designer for easy reference:

### Top Bar
- `TXT_Date` - Current date display
- `TXT_Time` - Current time display
- `TXT_Credits` - Player credits
- `TXT_AlertCount` - Number of active alerts

### Context Panel - System Details
- `TXT_SystemName` - System name (large, bold)
- `TXT_SystemOwner` - Faction owner
- `TXT_RegionType` - Region classification
- `TXT_Lawfulness` - Security rating
- `TXT_Population` - Total population
- `TXT_LocationCount` - Number of locations

### Context Panel - Player Overview
- `TXT_PlayerOverview` - "Player Overview" title
- `TXT_PlayerCredits` - Player credits
- `TXT_PlayerShips` - Number of owned ships
- `TXT_PlayerCrew` - Number of crew members

### Bottom Bar
- `TXT_AlertFeed` - Scrolling alert messages

---

## Visibility Management Pattern

### Show System Details, Hide Player Overview:
```
// Hide player widgets
Set Visibility (TXT_PlayerOverview, Collapsed)
Set Visibility (TXT_PlayerCredits, Collapsed)
Set Visibility (TXT_PlayerShips, Collapsed)
Set Visibility (TXT_PlayerCrew, Collapsed)

// Show system widgets
Set Visibility (TXT_SystemName, Visible)
Set Visibility (TXT_SystemOwner, Visible)
Set Visibility (TXT_RegionType, Visible)
Set Visibility (TXT_Lawfulness, Visible)
Set Visibility (TXT_Population, Visible)
Set Visibility (TXT_LocationCount, Visible)
```

### Show Player Overview, Hide System Details:
```
// Reverse of above
```

**Tip:** Create a custom event called "ShowSystemPanel" and "ShowPlayerPanel"  
to avoid duplicating this logic.

---

## Debug Tips

### Print Values in Blueprint
```
Print String (Value, Color=Yellow, Duration=5.0)
```

### Log Subsystem Availability
```
Get Universe Subsystem
	↓
Branch (IsValid?)
	├─ TRUE: Print "Universe subsystem OK"
	└─ FALSE: Print "Universe subsystem MISSING"
```

### Verify System Data
```
After GetSystemById:
	↓
Print String (OutSystem.SystemName)
Print String (String from Int: OutSystem.SystemId)
Print String (Format: "Lawfulness: {0}", OutSystem.Lawfulness)
```

---

## Common Issues & Solutions

### Issue: "GetUniverseSubsystem" not found in Blueprint
**Solution:** Rebuild C++ project, then right-click Blueprint node area → "Refresh Nodes"

### Issue: Text not updating
**Solution:** 
1. Check that `OnSystemSelected` event is overridden (not just created)
2. Verify text block variable names match exactly
3. Add Print String nodes to verify event is firing

### Issue: Subsystem returns nullptr
**Solution:**
1. Check that universe was generated (game instance should call GenerateUniverse)
2. Verify game mode has subsystems initialized
3. Check Output Log for subsystem initialization messages

### Issue: System data shows default values
**Solution:**
1. Verify SystemId is valid (not -1)
2. Check that GetSystemById is called with correct ID
3. Add Print String to show SystemId value

---

## Performance Tips

1. **Cache Subsystem References:** Get them once in Event Construct, store in variables
2. **Avoid Tick for Static Data:** Use events instead of polling every frame
3. **Update Time Display Efficiently:** Use a timer (1 second interval) instead of Event Tick
4. **Visibility Over Removal:** Use `Set Visibility (Collapsed)` instead of removing/re-adding widgets

---

## Next Steps After Basic Implementation

1. ✅ Basic layout and text display
2. ⏳ Add faction colors to owner display
3. ⏳ Show connected systems list
4. ⏳ Display location list with click-to-select
5. ⏳ Add market data summary
6. ⏳ Show trade routes on map
7. ⏳ Implement fog-of-war filtering
8. ⏳ Add smooth transitions between panel states

---

**Quick Start Checklist:**

- [ ] Create `WBP_MainHUD` Blueprint
- [ ] Reparent to `MainHUDWidget`
- [ ] Add Canvas Panels (Top, Context, Bottom)
- [ ] Add Text Blocks with correct names
- [ ] Override `Event On System Selected`
- [ ] Override `Event On Selection Cleared`
- [ ] Implement visibility switching logic
- [ ] Query subsystems for data
- [ ] Format and display data
- [ ] Set `MainHUDClass` in controller Blueprint
- [ ] Test in PIE

---

**End of Quick Reference Card**
