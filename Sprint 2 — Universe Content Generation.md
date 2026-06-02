# Sprint 2 — Universe Content Generation

## Goal

Convert generated star systems into structured playable locations that support future economy, factions, missions, and simulation systems.

Sprint 2 does NOT implement gameplay systems yet.

Sprint 2 builds the universe data model required by later systems.

---

## Primary Deliverable

Every generated system must contain meaningful generated content.

Universe generation should produce:

Universe
→ Systems
→ Celestial Bodies
→ Stations
→ Markets
→ Ownership
→ Production / Consumption hooks

---

## System Structure

Each star system should generate:

Required:

* system id
* seed
* name
* region type
* controlling faction
* lawfulness value
* jump connections

Optional generated content:

* stars
* planets
* moons
* asteroid fields
* anomalies
* nebula influence
* derelicts
* pirate presence

---

## Location Generation

Systems generate locations.

Location examples:

* planets
* moons
* orbital stations
* mining colonies
* trade hubs
* research facilities
* pirate outposts
* military bases
* abandoned facilities

Each location requires:

* location id
* location type
* faction ownership
* population estimate
* security rating
* production profile
* consumption profile

---

## Ownership Rules

Ownership should follow region logic.

Faction Core:
strong faction ownership.

Frontier:
mixed ownership possible.

Neutral:
independent / civilian / minor factions.

Lawless:
pirates / criminal groups / abandoned / unclaimed.

Disputed:
multiple claims possible.

---

## Economy Hooks (NOT full economy yet)

Locations must support future economy.

Every location stores:

produces[]
consumes[]
inventory[]

Placeholder data is acceptable.

Example:

Mars:
Consumes:

* Food
* Medicine
* Machinery

Produces:

* Rare Metals
* Industrial Parts

---

## Validation Requirements

Generation must validate:

* every system has at least one location
* ownership consistency
* no invalid faction references
* production/consumption arrays valid
* deterministic output for identical seed

---

## Debug Tools

Expand visualizer support.

Optional display modes:

* faction ownership
* region map
* security map
* population map
* economy preview
* location count
* connection graph

---

## Definition of Done

Sprint 2 complete when:

same seed → same universe content.

Every system contains generated locations.

Ownership rules operate correctly.

Debug visualization displays generated content.

Data structures ready for economy implementation in Sprint 3.
