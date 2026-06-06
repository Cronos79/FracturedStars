# Sprint07_FactionSimulationFoundation_Spec.md

# Sprint 7 – Faction Simulation Foundation

## Purpose

This sprint introduces factions as active participants in the living universe.

Factions are not simply names attached to territory.

Factions are economic, political, and strategic entities that react to conditions within the universe.

This sprint creates the foundation that future systems will build upon:

* News
* Missions
* Diplomacy
* Trade Agreements
* Embargoes
* Piracy
* Wars
* Rebellions
* Political Movements

This sprint does NOT implement those systems.

It creates the faction simulation layer that will eventually drive them.

---

# Critical Architecture Rule

The faction system must use existing systems.

DO NOT create:

* a second economy system
* a second trade system
* a second logistics system
* a second ship system
* a second market system

The following systems already exist:

* Universe Generation
* Economy
* Markets
* Production
* Consumption
* Ship Framework
* Components
* Logistics
* Trade Routes
* Fog Of War

Factions must consume data from those systems.

Factions react to the universe.

They do not replace it.

---

# Example Of Correct Design

Correct:

Faction detects:

Food Shortage

↓

Economy reports shortage

↓

Faction increases food priority

↓

Faction requests more imports

↓

Existing logistics system moves food

---

Incorrect:

FactionFood += 1000

FactionFood -= 500

Custom faction trade route

Custom faction inventory

Custom faction market

This duplicates systems that already exist.

---

# Core Philosophy

The economy drives the universe.

Factions react to the economy.

The economy does NOT react to faction scripts.

Emergent behavior is preferred.

Example:

Faction A loses access to engines.

↓

Ship production declines.

↓

Mining fleet growth declines.

↓

Ore production declines.

↓

Industrial output declines.

↓

Economic stress rises.

↓

Political consequences happen later.

No scripted event required.

---

# Universe Layout Rules

The universe must continue following the established generation rules.

Faction home regions should remain widely separated.

The goal is to force trade, logistics, dependency, and long-range interaction.

---

# Exception: Sol System

One intentional exception exists.

Earth and Mars both exist inside Sol.

This is a deliberate lore exception.

Earth and Mars are not separate distant civilizations.

They are human factions sharing the same home system.

---

# Human Powers

## Earth Government

Primary human power.

Largest human civilization.

Most powerful economy.

Largest population.

Largest industrial base.

Controls Earth.

Maintains official authority.

---

## Mars Independence Movement

Not a full sovereign power.

Not openly at war.

Not a formal empire.

Exists inside Sol.

Represents independence movements, rebels, political dissidents, and anti-Earth interests.

Can receive support from outside factions.

Can create future political tension.

---

# Other Major Factions

Non-human civilizations.

Names may change later.

Current design assumptions:

Faction A
Faction B
Faction C
Faction D

Plus:

The Dissonance

The hidden manipulator faction.

Influences events indirectly.

Rarely acts openly.

Not intended to behave like normal factions.

---

# Faction Data Requirements

Each faction should have:

FactionId

FactionName

FactionType

HomeSystemId

CoreSystemIds

ControlledSystems

Population

EconomicStrength

IndustrialStrength

MilitaryStrength Placeholder

Credits

CurrentEconomicStress

CurrentImports

CurrentExports

StrategicPriorities

Relations Placeholder

GovernmentType Placeholder

---

# Strategic Priorities

Factions should evaluate needs.

Examples:

Food

Medicine

Fuel

Industrial Materials

Ship Components

Population Stability

Trade Security

A faction should understand what it lacks.

This information will drive future behavior.

---

# Economic Awareness

Factions should gather information from existing markets.

Examples:

Food shortage in core worlds.

Fuel shortage in frontier systems.

Engine shortage affecting ship production.

Medicine shortage affecting population stability.

No fake faction inventories.

Use real location data.

---

# Economic Dependency Tracking

Factions should understand dependencies.

Example:

Earth

Produces:
Food

Needs:
Titan Engines

Mars

Produces:
Titan Engines

Needs:
Food

Dependency exists.

Dependency should be measurable.

Future diplomacy will use this information.

---

# Controlled Territory

Each faction should know:

Core Worlds

Frontier Worlds

Disputed Worlds

Lawless Border Regions

This information should come from universe generation data.

Do not create a second territory system.

---

# Economic Stress

Calculate faction-wide stress.

Inputs:

Food shortages

Fuel shortages

Medicine shortages

Component shortages

Industrial disruption

Trade disruption

Stress value becomes important later.

No direct consequences yet.

---

# Ship Production Awareness

Factions should evaluate shipbuilding capability.

Example:

Required:

Engine
Power Plant
Shield

Missing:

Engine

Result:

Ship production reduced.

This uses the existing component economy.

Do not create a separate faction ship economy.

---

# Logistics Integration

Factions use the logistics system.

They do not own a separate logistics simulation.

Example:

Faction needs food.

↓

Existing logistics network identifies route.

↓

Existing cargo ships move goods.

Faction is a consumer of logistics.

Not a replacement for logistics.

---

# News Integration Hooks

Generate data only.

Examples:

Major Food Shortage

Industrial Collapse

Medicine Crisis

Trade Boom

Component Shortage

Store events for future news generation.

Do not create news articles yet.

---

# Mission Integration Hooks

Generate opportunities only.

Examples:

Food Needed

Medicine Needed

Engine Shipment Needed

Fuel Delivery Needed

Store data for future mission generation.

Do not create missions yet.

---

# Diplomacy Hooks

Store future relationship values.

Friendly

Neutral

Hostile

Allied

Rival

No diplomacy simulation yet.

Only architecture.

---

# Dissonance Rules

The Dissonance is special.

Not a normal empire.

Does not necessarily follow normal economic behavior.

Should support future hidden influence mechanics.

No gameplay implementation required now.

Only data structures.

---

# Debug Requirements

PrintFactionSummary

PrintFactionEconomy

PrintFactionDependencies

PrintFactionStress

PrintFactionImports

PrintFactionExports

PrintFactionControlledSystems

PrintFactionStrategicPriorities

PrintUniverseFactionReport

---

# Validation Scenario

Generate universe.

Verify:

Earth exists in Sol.

Mars exists in Sol.

Other faction cores remain widely separated.

Factions detect shortages.

Factions detect dependencies.

Economic stress values calculate correctly.

No duplicate economy systems created.

No duplicate logistics systems created.

No duplicate ship systems created.

All faction information comes from existing systems.

---

# Explicitly Out Of Scope

No wars.

No diplomacy.

No alliances.

No embargoes.

No piracy.

No missions.

No news articles.

No military AI.

No rebellion gameplay.

No player reputation.

No political simulation.

Foundation only.

---

# Acceptance Criteria

✓ Earth exists in Sol.

✓ Mars exists in Sol.

✓ Other faction cores remain widely separated.

✓ Factions have persistent data structures.

✓ Factions understand imports and exports.

✓ Factions understand dependencies.

✓ Factions calculate economic stress.

✓ Factions evaluate shortages.

✓ Factions use existing economy data.

✓ Factions use existing logistics data.

✓ No duplicate systems created.

✓ Future hooks exist for news, missions, diplomacy, and war.

---

# Most Important Rule

Factions are consumers of existing systems.

They must never recreate economy, logistics, ships, markets, production, or trade.

The universe simulation already exists.

Factions observe it, analyze it, and react to it.
 
 Factions can be non human, each race has its own advantages and set badks