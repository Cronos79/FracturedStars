# Sprint03_EconomyCore_Spec.md

# Sprint 3 — Economy Core Specification

## Purpose

This document defines the detailed requirements for Sprint 3.

The goal is to create the first real economy foundation for Fractured Stars.

This sprint must support the project vision where planets, stations, colonies, and other locations have real supplies, real shortages, real demand, and real economic pressure.

The economy must not be decorative.

The economy must become the foundation for future trade, faction decisions, missions, piracy, smuggling, shortages, diplomacy, war pressure, and player opportunity.

---

## Sprint Goal

Implement the core location-based economy model.

By the end of Sprint 3, generated locations should have:

- goods definitions
- real inventory quantities
- production rules
- consumption rules
- shortage detection
- surplus detection
- basic price calculation
- market buy/sell data
- economy validation logs

This sprint does not need full NPC logistics yet.

However, the data model must be designed so NPC logistics, missions, black markets, diplomacy, and news can use it later.

---

## Core Design Philosophy

Markets belong to locations.

Not systems.

Not factions globally.

A star system may contain multiple economic locations.

Examples:

- Earth
- Mars
- orbital stations
- mining colonies
- shipyards
- research facilities
- pirate outposts
- frontier colonies
- military bases

Each economic location has its own inventory, production, consumption, shortages, and prices.

A good can reach zero.

A location with zero food should actually have no food for sale.

If Mars has no food, the market should reflect that.

If Earth produces excess food, Earth should sell food cheaply.

That gap creates trade.

Trade creates risk.

Risk creates missions.

Missions create player opportunity.

---

## Required Economy Concepts

### Goods

Goods are the items/resources traded and consumed by the universe.

Sprint 3 should define a basic goods catalog.

Initial goods should be simple and expandable.

Recommended starting categories:

#### Survival Goods

- Food
- Water
- Medicine
- Consumer Goods

#### Industrial Goods

- Ore
- Gas
- Refined Metals
- Machinery
- Electronics
- Fuel

#### Advanced Goods

- Advanced Components
- Medical Supplies
- Ship Parts
- Research Materials

#### Optional Illegal / Restricted Placeholder Goods

- Contraband
- Weapons
- Restricted Tech

The exact list can change later, but the architecture must support adding new goods cleanly.

---

## Goods Data Requirements

Each good should have at minimum:

- GoodId
- DisplayName
- Category
- BasePrice
- StorageUnitSize or CargoUnitSize
- IsLegalByDefault
- IsEssential
- Spoilage or decay flag if future support is desired
- Description placeholder

Example:

Food
- Category: Survival
- BasePrice: low
- Essential: true
- Legal: true

Medicine
- Category: Survival / Medical
- BasePrice: medium-high
- Essential: true
- Legal: true

Contraband
- Category: Illegal
- BasePrice: variable
- Essential: false
- Legal: false by default

---

## Location Market Requirements

Each economic location must have a market record.

A market stores real quantities.

Each market should track per-good:

- CurrentQuantity
- DesiredQuantity
- MinimumReserve
- MaximumStorage
- ProductionRate
- ConsumptionRate
- BuyPrice
- SellPrice
- DemandState
- SupplyState

The market must be able to represent:

- shortage
- severe shortage
- balanced supply
- surplus
- no stock
- full storage

---

## Inventory Rules

Inventory is real.

A market cannot sell more of a good than it has.

If CurrentQuantity is 0, available sell quantity is 0.

If a location needs goods, it may still buy them.

Example:

Mars Food:
- CurrentQuantity: 0
- DesiredQuantity: 10000
- MinimumReserve: 3000
- DemandState: Severe Shortage
- BuyPrice: high
- SellQuantity: 0

Earth Food:
- CurrentQuantity: 50000
- DesiredQuantity: 20000
- MinimumReserve: 10000
- DemandState: Surplus
- SellPrice: low
- BuyPrice: low

---

## Production Requirements

Locations may produce goods.

Production should be data-driven by location type, region, faction, and generated traits.

Examples:

Agricultural World:
- Produces Food
- Consumes Machinery, Fuel, Consumer Goods

Mining Colony:
- Produces Ore
- Consumes Food, Water, Machinery, Fuel

Industrial Station:
- Produces Machinery, Electronics
- Consumes Ore, Refined Metals, Fuel

Medical Research Colony:
- Produces Medicine / Medical Supplies
- Consumes Research Materials, Electronics, Food

Military Base:
- Produces little
- Consumes Food, Fuel, Weapons, Ship Parts

Not every location must produce something important.

Some locations mainly consume.

This is allowed.

---

## Consumption Requirements

Locations consume goods over time.

Consumption should be influenced by:

- population
- location type
- faction ownership
- facilities
- security status
- generated traits

Examples:

High population planets consume large amounts of food, water, medicine, and consumer goods.

Mining colonies consume machinery and fuel.

Military bases consume fuel, ship parts, weapons, and food.

Research sites consume electronics, research materials, medicine, and supplies.

---

## Economy Tick Requirements

Sprint 3 should implement a basic economy tick.

The tick does not need real-time perfection yet.

It should be able to run in controlled intervals.

Each tick should:

1. Apply production.
2. Apply consumption.
3. Clamp inventory between 0 and max storage.
4. Recalculate shortage/surplus state.
5. Recalculate buy/sell prices.
6. Log key economy changes for debugging.

The tick should be deterministic.

Same seed and same elapsed simulation steps should produce same economy results.

---

## Price Calculation Requirements

Prices must respond to actual supply and demand.

Do not use random prices detached from inventory.

Minimum logic:

- shortage raises buy price
- surplus lowers sell price
- zero quantity prevents selling
- essential goods react strongly to shortages
- luxury/nonessential goods react less severely

Recommended early formula behavior:

Severe shortage:
- BuyPrice much higher than BasePrice
- SellQuantity zero or very low

Shortage:
- BuyPrice higher than BasePrice

Balanced:
- BuyPrice and SellPrice near BasePrice

Surplus:
- SellPrice lower than BasePrice

Oversupply:
- BuyPrice low
- SellPrice very low

Exact numbers can be tuned later.

Architecture matters more than balance in this sprint.

---

## Market Buy/Sell Behavior

Sprint 3 should support basic market query behavior.

The game should be able to ask:

- What goods are available for sale here?
- What goods does this location want to buy?
- What is the buy price?
- What is the sell price?
- How many units can be bought?
- How many units can be sold to this market?
- Is this good illegal or restricted here?

Actual player trading UI may be out of scope.

But the backend market data must support future UI.

---

## Shortage Detection Requirements

A shortage exists when current inventory falls below desired levels.

Recommended states:

- None
- Low Supply
- Shortage
- Severe Shortage
- Critical / Empty

Critical / Empty should occur when CurrentQuantity is zero or near zero for an essential good.

Shortages should be logged and queryable.

Future systems will use shortages for:

- missions
- news
- faction decisions
- NPC logistics
- black markets
- unrest
- war pressure

---

## Surplus Detection Requirements

A surplus exists when current inventory is meaningfully above desired levels.

Surpluses should create export opportunities.

A location with surplus goods should be a natural supply source.

Future systems will use surplus data for:

- NPC trade routes
- player trade
- faction logistics
- contracts
- economic reporting

---

## Essential Goods

Some goods are essential.

Initial essential goods should include:

- Food
- Water
- Medicine
- Fuel

If an essential good is in severe shortage, the location should be flagged as economically stressed.

Economic stress should not need full consequences in Sprint 3.

But the state must exist.

Future systems will use this to drive:

- population loss
- unrest
- emergency missions
- diplomacy
- smuggling
- faction pressure
- news reports

---

## Example Scenario Requirement

Sprint 3 should be able to represent this situation in data:

Mars-like location:
- Food quantity: 0
- Food demand: high
- Food buy price: high
- Food sell quantity: 0
- Food shortage state: Critical

Earth-like location:
- Food quantity: high
- Food demand: satisfied
- Food sell price: low
- Food surplus state: Surplus

This creates a future trade route:

Buy Food from Earth-like location.
Transport Food to Mars-like location.
Sell Food for profit or mission reward.

NPC logistics and missions do not need to be implemented yet.

But the economy state must make this possible.

---

## Location Type Economy Profiles

Sprint 3 should create default economy profiles by location type.

Example profiles:

### Agricultural Planet

Produces:
- Food
- Water

Consumes:
- Machinery
- Fuel
- Consumer Goods
- Medicine

### Mining Colony

Produces:
- Ore

Consumes:
- Food
- Water
- Machinery
- Fuel
- Medicine

### Industrial Station

Produces:
- Refined Metals
- Machinery
- Electronics

Consumes:
- Ore
- Fuel
- Food
- Water

### Shipyard

Produces:
- Ship Parts
- Advanced Components

Consumes:
- Refined Metals
- Machinery
- Electronics
- Fuel

### Research Station

Produces:
- Research Materials
- Medical Supplies

Consumes:
- Electronics
- Food
- Medicine
- Advanced Components

### Military Base

Produces:
- Security / military readiness placeholder if needed

Consumes:
- Food
- Fuel
- Weapons
- Ship Parts
- Medicine

### Pirate Outpost

Produces:
- Contraband placeholder
- Stolen Goods placeholder

Consumes:
- Food
- Fuel
- Weapons
- Medicine

May buy goods illegally or at unusual prices.

---

## Region Influence On Economy

Region type should affect economy behavior.

### Faction Core

- stable supply
- higher population
- better storage
- stronger production
- less severe random shortage by default

### Frontier

- smaller markets
- less stable supply
- more dependency on imports

### Neutral

- varied markets
- independent stations
- trade hubs possible
- black markets possible later

### Lawless

- weak infrastructure
- pirate bases
- unstable markets
- smuggling importance
- scarce essential goods

### Disputed

- unstable ownership
- disrupted supply
- military demand
- higher fuel / medicine / weapons demand
- future conflict hooks

---

## Faction Influence On Economy

Faction ownership should influence economy profile.

Not all factions need detailed unique economies yet.

But architecture should support:

- faction preferred goods
- faction controlled goods
- faction restricted goods
- faction shortages
- faction strategic dependencies
- faction production bonuses

This matters for future politics and diplomacy.

Example:

One faction may produce most medicine.

Another faction may need that medicine during a plague.

This dependency can create trade negotiations, smuggling, war pressure, or humanitarian missions.

---

## Black Market Hooks

Full black market gameplay is out of scope.

However, market data should support it later.

Recommended placeholder fields:

- AllowsBlackMarket
- IllegalGoodsAccepted
- SmugglingRisk
- EnforcementLevel
- BlackMarketPriceModifier

Lawless, neutral, frontier, and disputed regions are likely candidates.

---

## Legal / Restricted Goods Hooks

Full legal system is out of scope.

But goods and markets should support legality later.

Data should allow:

- good legal by default
- good illegal by faction
- good restricted by location
- market accepts illegal goods
- enforcement level placeholder

This will support smuggling, patrols, inspections, and faction reputation later.

---

## Debug / Logging Requirements

Sprint 3 must include useful economy debug output.

Recommended logs:

- total markets generated
- total goods tracked
- total production per good
- total consumption per good
- number of locations in shortage
- number of locations in severe shortage
- number of locations with zero essential goods
- number of surplus locations per good
- richest supply source per good
- highest demand location per good

Example log:

Economy Statistics:
- Markets: 1046
- Goods: 12
- Food Producers: 128
- Food Shortages: 44
- Critical Food Shortages: 7
- Medicine Producers: 18
- Medicine Shortages: 61
- Zero Inventory Markets: 22

---

## Debug Visualization Requirements

If practical, extend the debug visualizer.

Possible modes:

- region view
- faction view
- location count view
- population view
- food shortage view
- medicine shortage view
- surplus view
- economy stress view

This is not required to be beautiful.

It is required to help validate the economy.

---

## Validation Requirements

Economy generation must validate:

- every economic location has a market
- every market references valid goods
- quantities are non-negative
- max storage is greater than or equal to current quantity
- production rates are non-negative
- consumption rates are non-negative
- prices are positive
- essential good shortage states calculate correctly
- no invalid faction references
- deterministic generation with same seed

Validation failure should be logged clearly.

---

## Explicitly Out Of Scope

Sprint 3 does not include:

- NPC cargo ships
- physical trade routes
- mission generation
- news reports
- diplomacy decisions
- war decisions
- population loss
- unrest simulation
- player trading UI
- black market UI
- smuggling mechanics
- patrol inspections
- combat
- save/load persistence beyond whatever already exists

Sprint 3 creates the economy state that those systems will use later.

---

## Acceptance Criteria

Sprint 3 succeeds when:

✓ Goods catalog exists.

✓ Every generated economic location has a market.

✓ Markets store real quantities per good.

✓ Goods can reach zero.

✓ Production and consumption exist.

✓ Basic economy tick updates inventories.

✓ Shortage and surplus states are calculated.

✓ Prices respond to supply and demand.

✓ Essential goods can create economic stress flags.

✓ Earth-like surplus and Mars-like shortage scenario is representable in data.

✓ Economy data is deterministic for identical seed and tick count.

✓ Economy validation passes.

✓ Debug logs show useful economy statistics.

✓ Future systems can query markets for supply, demand, price, shortage, and surplus.

---

## Developer Notes

Do not design this as a fake price table.

Do not make markets global.

Do not make goods magically appear unless produced, imported, or intentionally seeded.

Do not create UI-first economy code.

This sprint is backend simulation foundation.

The most important rule:

Locations own real inventory.

Inventory drives price.

Shortage drives demand.

Demand drives logistics.

Logistics drives missions.

Missions create player opportunity.

That chain is the core of Fractured Stars.
