# WBP_MainHUD Visual Layout Reference

**Resolution Reference:** 1920x1080 (can scale)  
**UI Style:** Dark theme, RTS/Strategy game aesthetic

---

## Full Screen Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│ TOP BAR (Height: 60px, Full Width, Black 50% opacity)              │
│ July 8, 2094  |  12:45:30  |  Credits: 10,000        Alerts: 3    │
└─────────────────────────────────────────────────────────────────────┘
│                                                    ┌─────────────────┤
│                                                    │ CONTEXT PANEL   │
│                                                    │ (Width: 400px)  │
│                                                    │ (Full Height)   │
│                                                    │ Dark Gray 80%   │
│                                                    │                 │
│                                                    │ ┌─────────────┐ │
│                   GALAXY MAP                       │ │   System    │ │
│                   (Camera View)                    │ │   Details   │ │
│                   (Main Gameplay Area)             │ │   or        │ │
│                                                    │ │   Player    │ │
│                                                    │ │   Overview  │ │
│                                                    │ └─────────────┘ │
│                                                    │                 │
│                                                    │ (Scrollable)    │
│                                                    │                 │
│                                                    │                 │
└────────────────────────────────────────────────────┴─────────────────┘
│ BOTTOM BAR (Height: 60px, Full Width, Black 50% opacity)           │
│ Alert: Market shortage in Proxima Centauri...                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Top Bar Detail (60px height)

```
┌─────────────────────────────────────────────────────────────────────┐
│  📅 July 8, 2094    🕐 12:45:30    💰 Credits: 10,000    ⚠️ Alerts: 3│
│  (20, 15)           (200, 15)      (400, 15)            (1800, 15)  │
│  Font: 18          Font: 18       Font: 18, Gold       Font: 18    │
└─────────────────────────────────────────────────────────────────────┘
```

### Implementation:
- **Background:** Canvas Panel, Black, 50% opacity
- **Anchors:** Top-Left (0, 0) to Top-Right (1, 0)
- **Size:** Fill width, Height 60
- **Text Blocks:** Size 18, White (except Credits = Gold, Alerts = Orange)

---

## Context Panel - System Selected (400px width)

```
┌────────────────────────┐
│  Proxima Centauri      │ ← Font 24, Bold, White
│  (SystemName)          │
├────────────────────────┤
│  Owner: Earth Gov      │ ← Font 16, Light Gray
│  Region: Faction Core  │
│  Lawfulness: 85%       │
│  Population: 1,245,000 │
│  Locations: 5          │
├────────────────────────┤
│  Known Locations:      │ ← Font 16, Bold
│  • Proxima Station     │ ← Font 14
│  • Mining Colony Alpha │
│  • Trade Hub Beta      │
│  ...                   │
├────────────────────────┤
│  Trade Summary:        │ ← Font 16, Bold
│  Imports: Food, Water  │ ← Font 14
│  Exports: Ore, Fuel    │
│  Shortages: Medicine   │ ← Red
│  Surpluses: Ore        │ ← Green
└────────────────────────┘
```

### Implementation:
- **Background:** Canvas Panel, Dark Gray (#1A1A1A), 80% opacity
- **Anchors:** Top-Right (1, 0) to Bottom-Right (1, 1)
- **Position:** X = -400 (offset from right edge), Y = 60 (below top bar)
- **Size:** Width 400, Height = Fill to bottom
- **Content:** Vertical Box inside Scroll Box
- **Padding:** 10px on all sides between elements

---

## Context Panel - Player Overview (400px width)

```
┌────────────────────────┐
│  Player Overview       │ ← Font 24, Bold, White
├────────────────────────┤
│  💰 Credits: 10,000    │ ← Font 18, Gold
├────────────────────────┤
│  🚀 Ships: 3           │ ← Font 16, White
│  • SS Enterprise       │ ← Font 14, clickable
│  • Mining Vessel Alpha │
│  • Scout Ship Beta     │
├────────────────────────┤
│  👥 Crew: 12           │ ← Font 16, White
│  • John Smith (Pilot)  │ ← Font 14, clickable
│  • Jane Doe (Engineer) │
│  • ...                 │
├────────────────────────┤
│  📋 Recent Alerts:     │ ← Font 16, Bold
│  • Shortage in Proxima │ ← Font 14, Yellow
│  • Ship arrived at Sol │ ← Font 14, Green
│  • ...                 │
└────────────────────────┘
```

### Implementation:
Same container as System Selected, but different content visibility

---

## Bottom Bar Detail (60px height)

```
┌─────────────────────────────────────────────────────────────────────┐
│  ⚠️ Alert: Critical shortage detected in Proxima Centauri          │
│  (20, 15)   Font: 16, Yellow                                       │
└─────────────────────────────────────────────────────────────────────┘
```

### Implementation:
- **Background:** Canvas Panel, Black, 50% opacity
- **Anchors:** Bottom-Left (0, 1) to Bottom-Right (1, 1)
- **Position:** Y = -60 (offset from bottom)
- **Size:** Fill width, Height 60
- **Text Block:** Size 16, Yellow, Left-aligned
- **Future:** Horizontal scrolling marquee for multiple alerts

---

## Color Palette

### Background Colors
- **Top Bar:** Black (#000000), 50% opacity
- **Bottom Bar:** Black (#000000), 50% opacity
- **Context Panel:** Dark Gray (#1A1A1A), 80% opacity
- **Galaxy Background:** Black (#000000), 100% or skybox

### Text Colors
- **Primary Text:** White (#FFFFFF)
- **Secondary Text:** Light Gray (#CCCCCC)
- **Credits:** Gold (#FFD700)
- **Alerts:** Orange (#FFA500)
- **Success/Good:** Green (#00FF00)
- **Warning:** Yellow (#FFFF00)
- **Error/Critical:** Red (#FF0000)
- **Faction Colors:** Varies by faction

### System Colors
- **Normal:** Blue (#3380FF)
- **Hovered:** Light Blue (#80B3FF)
- **Selected:** Gold (#FFD700)

---

## Font Sizes

- **Title/Header:** 24px, Bold
- **Section Header:** 16px, Bold
- **Primary Text:** 16px, Regular
- **Secondary Text:** 14px, Regular
- **Top Bar:** 18px, Regular

---

## Spacing & Padding

- **Top Bar Padding:** 20px left, 15px top
- **Context Panel Padding:** 10px all sides
- **Element Spacing:** 5px vertical between items
- **Section Spacing:** 10-20px vertical between sections

---

## Responsive Design Notes

### For Different Resolutions:
- Use **Anchors** instead of fixed positions where possible
- **Top Bar:** Anchor to top, fill width
- **Context Panel:** Anchor to right, fill height
- **Bottom Bar:** Anchor to bottom, fill width
- **Galaxy View:** Fill remaining space (center)

### Scale Settings:
- Set Canvas Panel Scale Mode to "Scale to Fit"
- Use DPI scaling for text if needed
- Test at 1920x1080, 2560x1440, and 3840x2160

---

## Widget Hierarchy

```
WBP_MainHUD (UserWidget)
└─ RootCanvas (Canvas Panel)
   ├─ TopBarPanel (Canvas Panel)
   │  ├─ TXT_Date (Text Block)
   │  ├─ TXT_Time (Text Block)
   │  ├─ TXT_Credits (Text Block)
   │  └─ TXT_AlertCount (Text Block)
   │
   ├─ ContextPanel (Canvas Panel)
   │  └─ ContextScrollBox (Scroll Box)
   │     └─ ContextContent (Vertical Box)
   │        ├─ TXT_SystemName (Text Block)
   │        ├─ TXT_SystemOwner (Text Block)
   │        ├─ TXT_RegionType (Text Block)
   │        ├─ TXT_Lawfulness (Text Block)
   │        ├─ TXT_Population (Text Block)
   │        ├─ TXT_LocationCount (Text Block)
   │        ├─ [Spacer]
   │        ├─ TXT_PlayerOverview (Text Block)
   │        ├─ TXT_PlayerCredits (Text Block)
   │        ├─ TXT_PlayerShips (Text Block)
   │        └─ TXT_PlayerCrew (Text Block)
   │
   └─ BottomBarPanel (Canvas Panel)
	  └─ TXT_AlertFeed (Text Block)
```

---

## Animation & Polish (Future)

### Panel Transitions:
- **System → Player:** Fade out system widgets, fade in player widgets (0.2s)
- **Player → System:** Fade out player widgets, fade in system widgets (0.2s)

### Hover Effects:
- **Text:** Slight color change on hover (if clickable)
- **Panels:** Subtle glow or border highlight

### Alerts:
- **New Alert:** Brief flash/pulse effect
- **Scrolling:** Horizontal scroll for long messages

---

## Testing Resolutions

Test the HUD at these resolutions to ensure proper scaling:

- ✅ 1920x1080 (Full HD) - Primary target
- ✅ 2560x1440 (QHD) - Secondary target
- ✅ 3840x2160 (4K) - High-end target
- ✅ 1280x720 (HD) - Minimum supported

Verify:
- Text is readable at all resolutions
- Panels don't overlap
- Context panel doesn't cover too much of the galaxy view
- Top/bottom bars scale correctly

---

## Accessibility Considerations

- **Font Size:** Minimum 14px for readability
- **Contrast:** High contrast between text and background
- **Color Blindness:** Don't rely solely on color (use icons/text too)
- **Scaling:** Support UI scale slider in settings (future)

---

**This layout matches the design from `UILayout.md` and supports the requirements from `Sprint 8 – Universe Map & Core UI.md`**

---

**End of Visual Layout Reference**
