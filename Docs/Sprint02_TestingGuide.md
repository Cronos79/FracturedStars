# Sprint 2 Testing Guide

## Overview
Sprint 2 adds universe content generation: celestial bodies, locations, ownership, and economy hooks.

## Quick Test Steps

### 1. Verify Content Generation
1. Open the project in Unreal Editor
2. Open the test level (Content/Maps/Dev/L_DevSandbox)
3. Find the UniverseDebugVisualizer actor in the outliner
4. Press Play (PIE)
5. Check the Output Log for:
   - "Generating system content..."
   - "System content generation complete"
   - "Validating system content..."
   - Content statistics (celestial bodies, locations, population)

### 2. Test Debug Visualizer
The visualizer now has three new display modes:

#### A. Location Count Overlay
- Select UniverseDebugVisualizer in outliner
- In Details panel, enable: **Universe Debug → Content → Show Location Count**
- Numbers appear above each system showing location count

#### B. Population Map
- Enable: **Universe Debug → Content → Show Population**
- Systems color by population:
  - Dark blue = low population
  - Bright white/blue = high population (1M+)

#### C. Ownership Map
- Enable: **Universe Debug → Content → Show Ownership**
- Systems color by controlling faction:
  - Each faction gets a distinct hue
  - Independent systems use default region color

### 3. Test Content Queries (Blueprint/Console)
Open the console (`) and test these commands:

```cpp
// Print content for a specific system
UniverseSubsystem->PrintSystemContent(0)

// This will show:
// - All celestial bodies (stars, planets, moons, asteroids)
// - All locations (stations, colonies, outposts)
// - Population, ownership, security
// - Production/consumption profiles
```

### 4. Validation Checks
The generation logs should show:

✅ **Every system has at least one location**
- "Systems without locations: 0"

✅ **Content distribution is reasonable**
- Average 5-10 celestial bodies per system
- Average 1-4 locations per system
- Total population in billions

✅ **Ownership follows region rules**
- Core systems: mostly faction-owned
- Lawless systems: independent/pirate
- Disputed: contested or no clear owner

✅ **No invalid references**
- "Invalid ownership references: 0"

### 5. Determinism Test
1. Note the current seed (default: 12345)
2. Generate universe (Play)
3. Copy the content statistics from the log
4. Restart PIE (Stop and Play again)
5. Verify statistics are IDENTICAL

## Expected Content Distribution

### Celestial Bodies
- **Stars**: 1-3 per system (mostly 1)
- **Planets**: 2-12 per system
- **Moons**: 0-3 per planet (gas giants more likely)
- **Asteroid Fields**: ~30% of systems
- **Anomalies**: ~5% of systems (rare)

### Locations by Region Type

#### Faction Core (3-6 locations)
- Trade Hubs
- Military Bases
- Shipyards
- Research Facilities
- High population (50K-500K per location)

#### Faction Frontier (2-4 locations)
- Military Bases
- Refueling Depots
- Mining Colonies
- Colonies
- Moderate population (10K-100K per location)

#### Neutral (1-3 locations)
- Trade Hubs
- Mining Colonies
- Colonies
- Stations
- Independent ownership

#### Lawless (1-2 locations)
- Pirate Outposts
- Abandoned Facilities
- Refueling Depots
- Low population or zero

#### Disputed (1-3 locations)
- Military Bases
- Abandoned Facilities
- Stations
- Contested ownership

## Economy Hooks

Each location has placeholder economy data:

### Production Examples
- **Mining Colony**: Ore, Rare Metals
- **Shipyard**: Industrial Parts
- **Research Facility**: Electronics, Chemical Compounds
- **Colony**: Food, Water
- **Military Base**: Weapons

### Consumption
All locations consume:
- Food (scales with population)
- Water (scales with population)
- Fuel (scales with population)

Plus type-specific needs:
- **Shipyard**: Ore, Rare Metals, Electronics
- **Research Facility**: Chemical Compounds, Electronics
- **Military Base**: Weapons, Machinery

## Performance Notes

With 500 systems:
- ~3,000-5,000 celestial bodies
- ~1,000-2,000 locations
- Generation time: < 1 second
- Visualization: 60 FPS (debug draw is lightweight)

## Common Issues

### "Systems without locations: X"
**Problem**: Validation error - some systems have no locations
**Fix**: Check GenerateLocations ensures LocationCount >= 1

### Visualizer not showing content overlays
**Problem**: Display modes not working
**Fix**: Ensure universe is generated (IsUniverseGenerated returns true)

### Population showing as zero everywhere
**Problem**: GeneratePopulation not being called
**Fix**: Check that GenerateLocations calls GeneratePopulation for each location

### Crash on content query
**Problem**: Invalid system ID
**Fix**: Ensure SystemId is in range [0, SystemCount)
