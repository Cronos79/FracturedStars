# Sprint05_5_ShipFrameworkAndComponentEconomy_Spec.md

# Sprint 5.5 — Ship Framework & Component Economy

## Purpose

This sprint creates the foundation for all ships in Fractured Stars.

The goal is to ensure ships are not predefined objects with fixed stats.

Instead:

Ship Frame
+
Installed Components
====================

Actual Ship

This system will be shared by:

* Player ships
* NPC ships
* Pirate ships
* Military ships
* Faction ships
* Trade ships
* Capital ships

One ship architecture must support all ship types.

---

# Core Philosophy

Ships are assemblies.

Ships are not magical finished products.

A ship frame provides structure.

Components provide capability.

Example:

Starter Freighter Frame

Contains:

* Engine Slot
* Power Plant Slot
* Shield Slot
* Utility Slots
* Weapon Slots

Installed components determine performance.

The frame does not determine most gameplay capability.

The components do.

---

# Design Goals

The ship system must support:

* progression
* upgrades
* specialization
* maintenance
* trade
* manufacturing
* shortages
* logistics

The economy and ship systems must be directly connected.

---

# Ship Construction Model

Ships are built from:

## Ship Frame

Provides:

* Hull
* Mass
* Base Crew Capacity
* Base Cargo Capacity
* Slot Layout
* Structural Integrity
* FTL Capability Rules

Frames define what can be installed.

Frames do not provide most operational functionality.

---

## Components

Provide actual functionality.

Examples:

* Engines
* Power Plants
* Shield Generators
* Weapons
* Utility Modules
* Sensors
* Cargo Modules
* Fuel Modules
* Mining Equipment

Without required components, ships may not function.

---

# Required Component Categories

## Engine

Required.

Provides:

* Movement
* Acceleration
* Travel capability

Without engine:

Ship cannot move.

---

## Power Plant

Required.

Provides:

* Power generation

All major systems consume power.

Without power plant:

Ship is non-operational.

---

## Shield Generator

Strongly recommended.

Provides:

* Shield capacity
* Shield recharge

Most ships should have at least one shield slot.

---

## Weapon Systems

Optional.

Examples:

* Laser
* Cannon
* Missile Launcher

Not all ships require weapons.

---

## Utility Modules

Highly important.

Examples:

* Mining Laser
* Cargo Expansion
* Fuel Expansion
* Sensor Array
* Tractor System
* Refinery Module
* Salvage Equipment

Utility modules create ship specialization.

---

# Slot Size System

Components use slot sizes.

Examples:

* Small
* Medium
* Large
* Capital

Example:

Starter Frame

* Small Engine Slot
* Small Power Plant Slot
* Small Shield Slot
* 2 Small Utility Slots
* 1 Small Weapon Slot

Example:

Medium Freighter

* Medium Engine Slot
* Medium Power Plant Slot
* Medium Shield Slots x2
* Utility Slots x4
* Weapon Slots x2

Example:

Capital Ship

* Capital Engine Slots
* Capital Power Plants
* Multiple Shield Arrays
* Multiple Utility Bays

---

# Frame Examples

## Starter Mining Frame

Slots:

* Small Engine
* Small Power Plant
* Small Shield
* Mining Utility
* Utility
* Small Weapon

---

## Starter Trade Frame

Slots:

* Small Engine
* Small Power Plant
* Small Shield
* Utility
* Utility

Higher cargo focus.

---

## Medium Freighter

Slots:

* Medium Engine
* Medium Power Plant
* Medium Shield
* Cargo Utilities
* Utility Slots

---

## Carrier

Slots:

* Capital Engines
* Capital Power Plants
* Hangar Facilities
* Support Systems

---

# Ship Stat Calculation

Final ship statistics are calculated.

Example:

Frame
+
Components
==========

Final Stats

Calculated values:

* Cargo Capacity
* Fuel Capacity
* Power Production
* Power Usage
* Shield Strength
* Speed
* Acceleration
* Crew Capacity
* Sensor Range
* Mining Capability

Nothing should be hardcoded into the final ship.

---

# Power System

Every major component consumes power.

Examples:

Engine:
Consumes Power

Shield:
Consumes Power

Mining Laser:
Consumes Power

Sensors:
Consumes Power

Power Plant:
Produces Power

If power demand exceeds supply:

Systems degrade or shut down.

Full simulation can come later.

Architecture must support it now.

---

# Component Quality Tiers

Components should support generations.

Examples:

Titan I Small Engine

Titan II Small Engine

Titan III Small Engine

Titan IV Small Engine

Titan V Small Engine

Higher generations provide improved performance.

This creates progression.

---

# Component Condition

All components should support condition values.

Fields:

* MaxCondition
* CurrentCondition

Example:

Titan III Engine

Condition: 72%

Future systems may:

* Repair
* Replace
* Upgrade

Condition architecture should exist now.

---

# Maintenance Demand

Ships create ongoing demand.

Examples:

* Repair Materials
* Replacement Components
* Fuel
* Service Costs

This is critical.

Ships must consume resources over time.

Otherwise the economy eventually stagnates.

---

# Economic Integration

Components are economic goods.

Examples:

* Titan Engine
* Atlas Shield
* Nova Power Plant
* Mining Laser
* Cargo Expansion

These are not generic "ship parts."

They are specific products.

Markets should be able to buy and sell them.

---

# Industrial Production Chains

Not every location should produce finished components.

Production should be layered.

Example:

Mining Colony
Produces:
Ore

↓

Refinery World
Produces:
Refined Metals

↓

Industrial World
Produces:
Industrial Materials

↓

Engine Factory
Produces:
Titan Engines

↓

Shipyard
Builds:
Ships

This creates natural trade routes.

---

# Faction Dependency

Factions should not be self-sufficient.

Example:

Faction A

Produces:
Engines

Needs:
Medicine

Faction B

Produces:
Medicine

Needs:
Engines

Trade emerges naturally.

Future diplomacy and conflict emerge from dependency.

---

# Jump Travel Rules

## Small Ships

Must use jump gates.

Cannot create independent jumps.

---

## Medium Ships

Primarily use jump gates.

May support limited exceptions later.

---

## Large Ships

Can support independent FTL.

Architecture should allow this.

---

## Capital Ships

Can generate fleet jumps.

Escort ships may travel with them.

Example:

Carrier Group

Carrier generates jump.

Escorts accompany carrier.

This creates strategic fleet movement.

---

# NPC Integration

NPC ships use the same framework.

No separate NPC ship architecture.

Player and NPC ships must use identical systems.

---

# Save / Load Requirements

Ship state must be serializable.

Required:

* Frame
* Installed Components
* Condition
* Cargo
* Fuel
* Location
* Ownership

---

# Debug Requirements

Provide testing functions.

Examples:

CreateTestShip

InstallComponent

RemoveComponent

PrintShipStats

PrintPowerUsage

PrintInstalledComponents

PrintShipEconomyData

---

# Acceptance Criteria

✓ Ship Frames exist.

✓ Component definitions exist.

✓ Slot system exists.

✓ Components determine ship capability.

✓ Power generation and usage framework exists.

✓ Component condition exists.

✓ Components are economic goods.

✓ Industrial production chains support component manufacturing.

✓ Factions can depend on external component suppliers.

✓ Small ships require jump gates.

✓ Capital ships support future independent FTL.

✓ Player and NPC ships use the same architecture.

✓ Ship data is ready for persistence.

---

# Most Important Rule

Ships are assemblies.

Frames provide structure.

Components provide capability.

The economy produces components.

Components build ships.

Ships consume components.

That relationship must drive trade, logistics, industry, shortages, and long-term progression throughout the universe.

lets also do this •	⚠️ Currency/credits system - lets add a universal credit system .. all factions trade in this currancy 