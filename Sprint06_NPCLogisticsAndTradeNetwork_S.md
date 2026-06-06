# Sprint06_NPCLogisticsAndTradeNetwork_Spec.md

# Sprint 6 — NPC Logistics & Trade Network

## Purpose

This sprint creates the first true living-universe behavior.

The economy already creates:

* production
* consumption
* shortages
* surpluses
* economic stress

However goods currently exist only inside local markets.

This sprint creates the system that physically moves goods throughout the universe.

Goods must never teleport.

Goods move because ships move them.

This is one of the core pillars of Fractured Stars.

---

# Core Philosophy

The economy drives the universe.

Locations require resources.

Resources must travel.

Trade exists because locations are not self-sufficient.

Every trade route should exist for a reason.

The simulation should naturally create:

* trade routes
* shortages
* exporters
* importers
* bottlenecks
* economic dependencies

The player should eventually be able to participate in the same logistics network used by NPCs.

---

# Major Design Rule

Goods do not teleport.

Incorrect:

Earth Food -= 1000

Mars Food += 1000

Correct:

Earth Market
↓
Cargo Ship
↓
Jump Network
↓
Mars Market

All economic movement should flow through logistics.

---

# Scope Of Sprint

Implement:

* trade requests
* export opportunities
* route generation
* NPC logistics companies
* cargo ship simulation
* cargo movement
* market delivery
* trade statistics

Do NOT implement:

* combat
* piracy
* escort gameplay
* player trading UI
* diplomacy
* news
* missions

Those systems come later.

---

# Trade Requests

Locations generate requests when shortages exist.

Examples:

Food Shortage

Medicine Shortage

Fuel Shortage

Machinery Shortage

Ship Component Shortage

Each request should contain:

* RequestId
* LocationId
* SystemId
* GoodType
* QuantityNeeded
* Priority
* RequestAge
* EconomicImportance

Example:

Mars

Food Needed:
5000

Priority:
Critical

---

# Export Opportunities

Locations generate exports when surpluses exist.

Example:

Earth

Food Surplus:
25000

Exportable:
15000

Priority:
Normal

Export data should include:

* GoodType
* QuantityAvailable
* MarketPrice
* ExportPriority

---

# Trade Matching

The logistics system should automatically match:

Exporter
+
Importer

Example:

Earth Food Surplus

↓

Mars Food Shortage

↓

Trade Opportunity Created

The system should identify:

* source location
* destination location
* cargo quantity
* estimated profit
* route distance

---

# Trade Route Generation

Routes use the universe jump network.

Route calculation should use:

* shortest jump count
* future support for route risk

Example:

Earth
↓
Alpha
↓
Frontier Outpost
↓
Mars

The route system should be reusable for future:

* player autopilot
* missions
* military movement
* piracy
* patrols

---

# Logistics Companies

Introduce NPC logistics organizations.

Examples:

Colonial Freight

Universal Cargo

Frontier Logistics

Independent Traders

These are simulation entities.

No visual representation required.

Minimum fields:

* CompanyId
* Name
* Credits
* ActiveShips
* CargoDelivered
* Reputation Placeholder

---

# NPC Cargo Ships

NPC ships must use the same ship architecture as players.

No special NPC ship system.

Examples:

Small Freighter

Medium Freighter

Heavy Freighter

Industrial Hauler

Each ship should reference:

* Frame
* Components
* Cargo Capacity
* Current Cargo
* Current Route
* Current System
* Current Destination

---

# Cargo Loading

When a ship accepts a trade job:

Export location inventory decreases.

Ship inventory increases.

Example:

Earth Food:
25000

Load:
5000

Earth Inventory:
20000

Ship Cargo:
5000

---

# Cargo Transit

Ships travel through jump routes.

The simulation may use abstract travel.

Visual travel is not required.

Track:

* Current System
* Route Progress
* Destination

Ships should physically exist within the simulation.

---

# Cargo Delivery

Upon arrival:

Ship Cargo:
decreases

Destination Inventory:
increases

Example:

Mars Food:
0

Arrival:
5000

Mars Food:
5000

Shortage improves.

Prices update naturally through existing economy systems.

---

# Ship Component Logistics

The logistics system must support component transport.

Examples:

Titan Engines

Nova Power Plants

Atlas Shields

Mining Lasers

Cargo Modules

These are treated exactly like other trade goods.

This is critical.

Ship production depends on component logistics.

---

# Industrial Supply Chains

The system must support multi-stage production.

Example:

Mining Colony
Produces Ore

↓

Refinery
Produces Refined Metals

↓

Industrial World
Produces Industrial Materials

↓

Engine Factory
Produces Titan Engines

↓

Shipyard
Builds Ships

Each stage creates demand for the previous stage.

This is a core design requirement.

---

# Economic Dependency

Locations should become dependent on other locations.

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

Trade naturally emerges.

No scripting required.

---

# Trade Profitability

Logistics companies should prefer profitable routes.

Basic calculation:

## Revenue

## Purchase Cost

Operating Cost

Future systems may add:

* fuel cost
* maintenance cost
* piracy risk
* faction tariffs

Architecture should support expansion.

---

# Economic Recovery

Shortages should eventually recover through logistics.

Example:

Mars Food:
0

NPC Trade Arrives

Mars Food:
5000

Food Prices Fall

Economic Stress Improves

The universe should be capable of solving its own problems.

---

# Statistics System

Track universe-wide metrics.

Examples:

Top Exporters

Top Importers

Most Traded Goods

Most Valuable Goods

Busiest Trade Routes

Largest Logistics Companies

Total Cargo Moved

Average Delivery Distance

These statistics become future news content.

---

# Future Integration Hooks

This sprint must prepare for:

* news generation
* mission generation
* piracy
* smuggling
* military logistics
* faction diplomacy
* embargoes
* wars

Do not implement these systems yet.

Only provide hooks.

---

# Debug Requirements

Required reports:

PrintTradeRequests

PrintExportOpportunities

PrintTradeRoutes

PrintCompanyStatistics

PrintCargoShipStatistics

PrintEconomicDependencies

PrintTopImports

PrintTopExports

PrintMostTradedGoods

---

# Validation Scenario

Create test universe:

Agricultural World
Food Surplus

Mining Colony
Food Shortage

Industrial World
Machinery Shortage

Run simulation.

Verify:

* trade requests generated
* exports generated
* routes created
* cargo loaded
* cargo delivered
* shortages improve
* prices react
* stress decreases

---

# Explicitly Out Of Scope

No combat.

No pirates.

No escorts.

No blockades.

No inspections.

No player trading UI.

No missions.

No diplomacy.

No news.

No wars.

No smuggling.

Only logistics and trade.

---

# Acceptance Criteria

✓ Trade requests generated from shortages.

✓ Export opportunities generated from surpluses.

✓ Routes generated using jump network.

✓ Logistics companies exist.

✓ NPC cargo ships exist.

✓ Cargo physically moves through simulation.

✓ Markets update on delivery.

✓ Component trade supported.

✓ Industrial supply chains supported.

✓ Economic dependencies emerge naturally.

✓ Shortages can recover through logistics.

✓ Statistics system functions.

✓ No goods teleport.

✓ Player and NPC ships use the same ship architecture.

---

# Most Important Rule

The universe must solve its own economic problems.

Trade should not be scripted.

Trade should emerge because locations genuinely need goods they cannot produce themselves.

Every convoy should exist because the economy created a reason for it.
