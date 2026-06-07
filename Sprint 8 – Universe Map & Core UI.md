# Sprint 8 – Universe Map & Core UI

## Goal

Transform the existing debug universe visualization into the first playable game interface.

This sprint does NOT add new simulation systems.

This sprint exposes existing systems to the player.

Existing systems already implemented:

* Universe Generation
* Economy
* Markets
* Logistics
* Ships
* Components
* Fog Of War
* Player Framework
* Crew Framework
* Faction Framework

The UI must consume those systems.

Do NOT recreate them.

---

# Primary Design Goal

The galaxy map is the game.

The galaxy map is not a menu.

The galaxy map is not a secondary screen.

The galaxy map is the primary player experience.

The player should spend most of their time looking at the galaxy map.

---

# Screen Layout

Top Bar

* Current Date
* Current Time
* Credits
* Active Alerts Count

Center

* Galaxy Map

Bottom Left

* Quick Navigation Buttons

Bottom Center

* Alert Feed

Right Side

* Full Height Context Panel

---

# Context Panel Rules

The context panel occupies the full right side of the screen.

The panel itself never changes position.

Only its contents change.

No popup windows for common actions.

No multiple floating windows.

No permanent left-side information panels.

Keep the interface clean.

---

# Context Panel States

## Nothing Selected

Show:

Player Overview

Credits

Owned Ships

Crew Count

Recent Alerts

---

## System Selected

Show:

System Name

Owner

Region Type

Lawfulness

Population

Known Planets

Known Stations

Known Asteroid Fields

Known Shortages

Known Surpluses

Known Imports

Known Exports

Fog Of War restrictions apply.

---

## Location Selected

Show:

Location Name

Location Type

Owner

Population

Market Summary

Imports

Exports

Shortages

Surpluses

Fog Of War restrictions apply.

---

## Ship Selected

Show:

Ship Name

Ship Frame

Installed Components

Fuel

Cargo

Crew

Current Orders

Destination

Status

---

## Crew Selected

Show:

Crew Name

Skills

Assignment

Ship

Status

---

## Faction Selected

Show:

Faction Name

Controlled Systems

Economic Stress

Dependencies

Known Imports

Known Exports

---

## News Selected

Show:

Full News Article

Related Systems

Related Factions

Related Locations

---

# Galaxy Map Requirements

Systems must be clickable.

Selection must visibly highlight.

Hover tooltip must display:

System Name

Owner

Region Type

Population (if known)

---

# Fog Of War Integration

All displayed information must respect fog of war.

Unknown systems:

Display name only.

Known systems:

Display limited information.

Surveyed systems:

Display detailed information.

Active observation systems:

Display live information.

---

# Out Of Scope

No diplomacy screens.

No mission screens.

No combat screens.

No market trading UI.

No inventory management UI.

No ship fitting UI.

Those are future sprints.

This sprint is visibility and interaction only.

---

# Acceptance Criteria

✓ Systems can be clicked

✓ Systems can be selected

✓ Context panel updates correctly

✓ Fog Of War restrictions work

✓ Existing game data displays correctly

✓ No duplicate simulation systems created

✓ Galaxy map becomes primary gameplay screen
