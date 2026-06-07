# WBP_MainHUD – Right Panel Enrichment Guide

This document describes every row to add to the right context panel and the exact
C++ function to call for each piece of data. All functions live on the widget's
`Self` reference (cast to `MainHUDWidget`) and are already compiled and exposed
to Blueprint.

> **Rule:** Do NOT widen the panel. Add rows/sections inside the existing width.

---

## Data Source Reference

All calls below use `SystemId = GetSelectedSystemId()` as input.

---

## Panel Layout (top → bottom)

### ── SYSTEM ──────────────────────────────

| Row | Label | Function | Notes |
|-----|-------|----------|-------|
| 1 | *(large text)* | `GetSelectedSystemId()` → look up name via `GetUniverseSubsystem → GetSystemById` | Display `SystemName` field |
| 2 | Faction | `GetFactionName(GetSystemFactionId(SystemId))` | Color text with `GetFactionColor(FactionId)` |
| 3 | Region | Already wired – keep existing `GetRegionTypeDisplayName` call | — |
| 4 | Lawfulness | Already wired – keep existing lawfulness bar | — |

---

### ── POPULATION ──────────────────────────

| Row | Label | Function | Notes |
|-----|-------|----------|-------|
| 5 | Population | `GetSystemTotalPopulation(SystemId)` | Format with `FormatNumber(value)` |

---

### ── BODIES ──────────────────────────────

Add a small horizontal row of icon+count pairs:

| Icon label | Function |
|------------|----------|
| 🪐 Planets | `GetSystemPlanetCount(SystemId)` |
| 🛰 Stations | `GetSystemStationCount(SystemId)` |
| ☄ Asteroids | `GetSystemAsteroidFieldCount(SystemId)` |

> Suggested widget: three `HorizontalBox` entries each with a small icon and a
> `TextBlock`. Hide an entry if its count == 0.

---

### ── ECONOMY ─────────────────────────────

#### Shortages
- Call `GetSystemShortages(SystemId)` → returns `TArray<FString>`
- Display as a `VerticalBox` of red `TextBlock` rows, one per entry
- Hide the section header if the array is empty

#### Surpluses
- Call `GetSystemSurpluses(SystemId)` → returns `TArray<FString>`
- Display as a `VerticalBox` of green `TextBlock` rows, one per entry
- Hide the section header if the array is empty

---

### ── TRADE ───────────────────────────────

#### Imports
- Call `GetSystemImports(SystemId)` → returns `TArray<FString>` (top 5 by volume)
- Label: **Imports**
- Display as compact list; hide section if empty

#### Exports
- Call `GetSystemExports(SystemId)` → returns `TArray<FString>` (top 5 by volume)
- Label: **Exports**
- Display as compact list; hide section if empty

#### Known Trade Routes
- Call `GetSystemTradeRouteCount(SystemId)` → returns `int32`
- Display as single line: `"Trade Routes: 12"`
- Hide if 0

---

### ── SHIPS ───────────────────────────────

| Row | Label | Function | Notes |
|-----|-------|----------|-------|
| Known Ships | `GetSystemShipCount(SystemId)` | Shows **player** ships only; hide if 0 |

---

## Blueprint Implementation Tips

### Binding pattern for arrays
For `TArray<FString>` results, use a **For Each Loop** in the
`OnSystemSelected` event to populate a `VerticalBox`:

```
OnSystemSelected(SystemId)
  → Clear VerticalBox children
  → GetSystemShortages(SystemId)
  → For Each Loop
	   → Create Widget (WBP_EconomyRow or simple TextBlock)
	   → Set text = loop element
	   → Add child to VerticalBox
```

### Hiding empty sections
Wrap each section in a `VerticalBox` with a header row.
Set `Visibility = Collapsed` when the data is empty:

```
if ArrayLength == 0 → Set Visibility (Collapsed)
else               → Set Visibility (Visible)
```

### Faction color on system name / faction row
```
TextBlock (Faction Name)
  → Set Text  = GetFactionName(GetSystemFactionId(SelectedSystemId))
  → Set Color = GetFactionColor(GetSystemFactionId(SelectedSystemId))
```

---

## Events to hook

All updates should fire inside `OnSystemSelected(SystemId)`:

1. Set System Name text
2. Set Faction text + color
3. Set Population text (FormatNumber)
4. Set Planets / Stations / Asteroids counts
5. Rebuild Shortages list
6. Rebuild Surpluses list
7. Rebuild Imports list
8. Rebuild Exports list
9. Set Trade Routes text
10. Set Ships text

And in `OnSelectionCleared()`:
- Collapse the entire right panel or show the player overview section.

---

## Quick Function Index

```
GetSelectedSystemId()                       → int32
GetSystemFactionId(SystemId)                → int32
GetFactionName(FactionId)                   → FString
GetFactionColor(FactionId)                  → LinearColor
GetSystemTotalPopulation(SystemId)          → int32
GetSystemPlanetCount(SystemId)              → int32
GetSystemStationCount(SystemId)             → int32
GetSystemAsteroidFieldCount(SystemId)       → int32
GetSystemShortages(SystemId)                → TArray<FString>
GetSystemSurpluses(SystemId)                → TArray<FString>
GetSystemImports(SystemId)                  → TArray<FString>
GetSystemExports(SystemId)                  → TArray<FString>
GetSystemTradeRouteCount(SystemId)          → int32
GetSystemShipCount(SystemId)                → int32
FormatNumber(int32)                         → FString
GetRegionTypeDisplayName(ERegionType)       → FString
```

All are `BlueprintPure` (no execution pin needed) except `FormatNumber`
and `GetRegionTypeDisplayName` which are `BlueprintCallable`.
