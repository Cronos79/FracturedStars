# Sprint05_EconomicSimulation_Spec.md

# Sprint 5 — Economic Simulation Specification

## Purpose

This sprint transforms the economy from static market data into a living simulation.

The economy is the heart of Fractured Stars.

Trade, missions, politics, piracy, shortages, diplomacy, news, and wars should emerge from economic pressure.

The economy is not a side system.

The economy drives the universe.

---

# Core Philosophy

Every location has needs.

Every location produces some things.

No location should be fully self-sufficient.

Interdependency is intentional.

Trade must become necessary.

Economic dependency creates:

* trade routes
* shortages
* profits
* smuggling
* piracy
* diplomacy
* conflict

The universe should naturally generate problems that require movement of goods.

---

# Economic Tick

Implement a recurring economy simulation tick.

Example:

Every game hour.

Every tick:

1. Produce goods.
2. Consume goods.
3. Update inventories.
4. Recalculate shortages.
5. Recalculate surpluses.
6. Update prices.
7. Update economic stress.

No NPC cargo movement yet.

This sprint only simulates local economies.

---

# Goods Philosophy

Goods should exist in categories.

The goal is to support future complexity without requiring hundreds of goods immediately.

---

## Population Goods

Required for civilian life.

Examples:

* Food
* Water
* Medicine
* Consumer Goods
* Clothing

Population requires these to function normally.

---

## Industrial Goods

Required for industry.

Examples:

* Ore
* Refined Metals
* Machinery
* Electronics
* Industrial Parts

Industry cannot operate efficiently without them.

---

## Energy Goods

Examples:

* Fuel
* Reactor Fuel
* Energy Cells

Energy shortages should impact production.

---

## Construction Goods

Examples:

* Building Materials
* Structural Components
* Concrete Equivalent
* Habitat Components

Used for future expansion systems.

---

## Ship Support Goods

Examples:

* Ship Parts
* Advanced Components
* Repair Materials
* Fuel

Ships require maintenance and operation support.

---

## Advanced Goods

Examples:

* Research Materials
* Medical Supplies
* High Technology Components

More specialized and valuable.

---

## Illegal Goods

Placeholder support only.

Examples:

* Contraband
* Illegal Weapons
* Restricted Technology

Future black market integration.

---

# Location Production Profiles

Every location should have a profile.

Examples:

Agricultural World
Produces:

* Food
* Water

Consumes:

* Machinery
* Fuel
* Consumer Goods

---

Mining Colony

Produces:

* Ore

Consumes:

* Food
* Water
* Fuel
* Machinery
* Medicine

---

Industrial World

Produces:

* Machinery
* Electronics
* Industrial Parts

Consumes:

* Ore
* Fuel
* Food

---

Shipyard

Produces:

* Ship Parts
* Advanced Components

Consumes:

* Refined Metals
* Machinery
* Electronics
* Fuel

---

Research Station

Produces:

* Research Materials
* Medical Supplies

Consumes:

* Electronics
* Food
* Medicine

---

Military Base

Produces:

* none or minimal goods

Consumes:

* Food
* Fuel
* Weapons
* Ship Parts
* Medicine

---

Pirate Outpost

Produces:

* minimal legal production

Consumes:

* Food
* Fuel
* Weapons

May later support black markets.

---

# Population Consumption

Population consumes resources.

Examples:

Food
Water
Medicine
Consumer Goods
Clothing

Population size should directly influence consumption.

Higher population:

* more consumption
* larger markets
* stronger economic influence

---

# Industrial Consumption

Industry consumes resources.

Examples:

Ore → Machinery

Machinery production requires:

* Ore
* Fuel
* Labor placeholder

Production should not be free.

---

# Ship Maintenance Demand

This sprint should begin creating demand for ships.

Ships do not need full maintenance systems yet.

However locations should consume:

* Ship Parts
* Repair Materials
* Fuel

This creates future demand.

Later NPC fleets will consume these goods.

---

# Currency Model

All locations use currency.

Trade should not require direct barter.

Example:

Earth sells food.

Player buys food using credits.

Player sells food elsewhere.

Credits are the common exchange mechanism.

Locations may have:

* available credits
* buying budgets
* market liquidity

Simple implementation is acceptable initially.

Architecture must support future expansion.

---

# Shortages

Shortages occur when inventory falls below desired levels.

Examples:

Food Shortage
Fuel Shortage
Medicine Shortage
Machinery Shortage

Shortages should be queryable.

Shortages should influence price.

---

# Surpluses

Surpluses occur when inventory exceeds desired levels.

Surpluses create export opportunities.

Locations with surpluses should become natural trade sources.

---

# Economic Stress

Locations should calculate stress levels.

Examples:

Food missing
Medicine missing
Fuel missing

Economic stress is a key future driver.

Future systems will use stress for:

* missions
* unrest
* politics
* news
* faction decisions
* migration
* conflict

Only calculate the value in Sprint 5.

No consequences yet.

---

# Production Efficiency

Production should depend on required inputs.

Example:

Machinery Factory

Needs:
Ore
Fuel

If Fuel is unavailable:

Production efficiency drops.

If Ore is unavailable:

Production stops.

This is critical.

Goods should not magically appear.

---

# Determinism Requirements

Economy simulation must remain deterministic.

Same seed.

Same tick count.

Same economy state.

Results should match.

---

# Debug Requirements

Required reports:

* total production per good
* total consumption per good
* total shortages
* total surpluses
* highest demand locations
* largest exporters
* largest importers
* economic stress rankings

---

# Test Scenario

Create validation scenario:

Agricultural World:
Food surplus

Mining Colony:
Food shortage

Industrial World:
Fuel shortage

Run simulation.

Verify:

* shortages occur
* prices adjust
* stress values update
* inventories change
* production efficiency changes

---

# Explicitly Out Of Scope

No NPC cargo ships.

No trade routes.

No missions.

No news.

No diplomacy.

No piracy.

No wars.

No black markets.

No player trading UI.

Only economic simulation.

---

# Acceptance Criteria

✓ Goods are produced.

✓ Goods are consumed.

✓ Inventories change over time.

✓ Prices respond to shortages and surpluses.

✓ Production requires inputs.

✓ Production can slow or stop due to shortages.

✓ Economic stress is calculated.

✓ Locations are not fully self-sufficient.

✓ Trade opportunities naturally emerge.

✓ Simulation remains deterministic.

✓ Debug reports validate economy behavior.

The most important rule:

The universe must create reasons for trade.

Trade should not exist because the designer says so.

Trade should exist because locations genuinely need things they cannot produce themselves.
