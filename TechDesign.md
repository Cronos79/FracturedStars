# TechDesign.md

## Purpose

This document describes the **systems and architecture required to make
the Vision possible**.

It is not a lore or gameplay fantasy document.

------------------------------------------------------------------------

## Universe Framework

### Universe Hierarchy

Universe - Factions - Star Systems - Global Events - Characters / Crew -
News / Information

Star System - Locations - Ships - Gates - Security - Faction Presence -
Local Events

Location - Planet - Station - Colony - Mining Site - Shipyard - Pirate
Base - Research Site

### Simulation Model

Layered simulation depth.

High Detail: - player presence - trusted crew presence - owned assets -
meaningful visibility

Abstract Background: - reduced-cost simulation - economy still
advances - factions still operate - logistics still matter

### Visibility / Information

Real universe state and player-known state are separate.

Visibility sources: - player presence - crew presence - owned ships -
stations / assets - news / intelligence

------------------------------------------------------------------------

## Economy Framework

### Market Ownership

Markets belong to **locations**, not systems.

Each location maintains:

-   real inventory quantities
-   production
-   consumption
-   imports
-   exports
-   pricing
-   storage

Goods can reach zero.

Shortages create: - demand spikes - mission generation - NPC logistics
response - black markets - political pressure

### Logistics

Critical goods move through actual transport.

NPC cargo ships: - haul goods - replenish markets - satisfy contracts -
respond to shortages

Distance and risk matter.

------------------------------------------------------------------------

## Faction Framework

Major faction regions: - separated by distance - buffer zones - lawless
corridors - dangerous transit routes

Interdependency is intentional.

Factions require: - diplomacy - trade - convoy protection - smuggling
tolerance - strategic logistics

------------------------------------------------------------------------

## Mission Framework

Mission generation should emerge from simulation.

Examples: - shortages - piracy - embargoes - disease outbreaks - convoy
needs - political instability

------------------------------------------------------------------------

## Persistence Framework

Save / Load foundation:

-   universe seed
-   generated systems
-   faction placement
-   markets
-   inventories
-   player assets
-   world state
-   versioned saves
