# Sprint04_PlayerCrewShipFoundation_Spec.md

# Sprint 4 — Player, Crew, and Ship Foundation Specification

## Purpose

This document defines the detailed requirements for Sprint 4.

The goal is to create the first playable identity layer for Fractured Stars.

This sprint must establish the difference between:

- Unreal Engine player/controller objects
- the actual in-universe game player
- owned ships
- crew members
- player location and visibility

Fractured Stars is not a traditional FPS/TPS game.

The player is not primarily a humanoid pawn running around a 3D world.

The player experience is closer to RTS / management / simulation.

The Unreal-controlled object should mainly represent camera, UI, input, selection, and command control.

The in-universe player should be represented as persistent simulation data.

---

## Sprint Goal

Create persistent server-authoritative data structures for:

- player profile
- in-universe player character
- starter ship
- basic crew members
- ownership relationships
- current location tracking
- fog-of-war integration hooks

By the end of Sprint 4, the game should be able to say:

A player exists in the universe.

That player owns a ship.

That ship exists at a real generated location.

The player can have crew.

The player / ship location grants visibility.

The Unreal camera/controller is separate from the simulation player.

---

## Core Design Philosophy

Unreal's PlayerController is not the in-universe player.

Unreal's Pawn is not necessarily the in-universe player.

For Fractured Stars:

PlayerController
- receives input
- controls UI
- sends commands
- owns client connection
- interacts with camera/map

Camera Pawn
- moves around galaxy/system view
- zooms and pans
- supports RTS-style viewing

Game Player
- persistent in-universe identity
- captain/operator/person
- has credits, skills, reputation, location, owned assets

Ship
- persistent owned asset
- has location, cargo, crew capacity, fuel, status

Crew
- persistent people
- assigned to ships or locations
- have skills and traits

The player's physical existence in the universe is represented by data.

The player's view of the universe is represented by camera/UI.

These must stay separate.

---

## Required Architecture Separation

### Unreal Runtime Layer

Suggested classes / responsibilities:

AFracturedStarsPlayerController
- handles input
- handles UI commands
- requests server actions
- receives filtered data
- owns client connection

AUniverseCameraPawn
- map camera
- zoom
- pan
- selection cursor if needed
- RTS-style viewing

GameMode / GameState / replicated bridge
- server session coordination
- player login/spawn hooks
- future replication support

### Simulation Data Layer

Suggested data structures:

FPlayerProfileData
- PlayerId
- DisplayName
- Credits
- CurrentSystemId
- CurrentLocationId
- CurrentShipId
- Skill data placeholder
- Reputation placeholder
- OwnedShipIds
- CrewIds

FShipData
- ShipId
- ShipName
- OwnerPlayerId
- CurrentSystemId
- CurrentLocationId
- CargoCapacity
- CurrentCargo
- FuelCapacity
- CurrentFuel
- CrewCapacity
- AssignedCrewIds
- ShipRole placeholder
- ShipStatus

FCrewMemberData
- CrewId
- Name
- OwnerPlayerId
- AssignedShipId
- CurrentSystemId
- CurrentLocationId
- Skills
- Traits
- Morale placeholder
- Loyalty placeholder

The exact class/struct names can change, but this separation must remain.

---

## Server Authority Requirements

The server owns all player, ship, and crew simulation data.

Clients may request actions.

Clients must not directly modify authoritative simulation state.

Examples:

Client request:
- create player
- select starter ship
- move ship
- assign crew
- query visible systems

Server validates:
- player owns ship
- ship can move
- destination is connected
- player has enough fuel/credits if required
- crew assignment is legal

Server updates:
- player location
- ship location
- fog of war
- known information

---

## Player Profile Requirements

Each player should have a persistent profile.

Minimum required fields:

- PlayerId
- PlayerName
- Credits
- CurrentSystemId
- CurrentLocationId
- CurrentShipId
- OwnedShips
- CrewMembers
- CreatedTime or placeholder
- LastActiveTime or placeholder

Initial values may be simple.

Example:

Player:
- PlayerId: 1
- Name: Test Captain
- Credits: 5000
- CurrentSystemId: Starting system
- CurrentLocationId: Starting station
- CurrentShipId: Starter ship

---

## In-Universe Player Character

The in-universe player character is a person in the universe, but not a required Unreal pawn.

The player should have:

- skills placeholder
- traits placeholder
- backstory placeholder
- current physical location
- assigned ship

Skill details can be minimal in Sprint 4.

However, the structure must support future skill systems.

Example future skills:

- Piloting
- Trade
- Command
- Engineering
- Negotiation
- Smuggling
- Combat
- Leadership

Do not implement full skill progression yet.

Only create the structure.

---

## Starter Ship Requirements

Every player should start with one basic ship.

Starter ship should be simple.

Minimum required fields:

- ShipId
- ShipName
- OwnerPlayerId
- CurrentSystemId
- CurrentLocationId
- CargoCapacity
- FuelCapacity
- CrewCapacity
- AssignedCrewIds
- CurrentCargo
- CurrentFuel

Initial starter ship values may be basic.

Example:

Starter Ship:
- CargoCapacity: 100
- FuelCapacity: 100
- CrewCapacity: 3
- CurrentFuel: 100
- CurrentCargo: empty

No combat required.

No ship component system required yet.

No detailed equipment required yet.

This sprint only creates the asset foundation.

---

## Crew Foundation Requirements

Crew are persistent people, not disposable modifiers.

Sprint 4 should support basic crew records.

Minimum fields:

- CrewId
- Name
- OwnerPlayerId
- AssignedShipId
- CurrentSystemId
- CurrentLocationId
- Skill placeholders
- Trait placeholders
- Status

Initial crew can be generated simply.

Example traits:

- Hardworking
- Lazy
- Friendly
- Loner
- Night Owl
- Early Riser

Example skills:

- Piloting
- Engineering
- Trade
- Security
- Medical

No full crew gameplay required yet.

No morale simulation required yet.

No crew leveling required yet.

But data structures must support those systems later.

---

## Ownership Rules

Ownership must be explicit.

Player owns ships.

Player employs or controls crew.

Ships contain crew assignments.

Crew may be assigned to:

- player ship
- other owned ship
- location
- unassigned reserve

Sprint 4 only needs simple assignment.

Future systems will expand this.

---

## Location Tracking

Player, ships, and crew must exist at real universe locations.

They should reference generated IDs:

- SystemId
- LocationId

Avoid fake detached player state.

Bad:

Player has no real universe position.

Good:

Player is physically associated with a generated location or ship.

Example:

Player:
- CurrentSystemId: 184
- CurrentLocationId: 0
- CurrentShipId: 1001

Ship:
- CurrentSystemId: 184
- CurrentLocationId: 0

Crew:
- AssignedShipId: 1001

---

## Basic Movement Requirement

Sprint 4 should support a minimal movement command.

The player or ship should be able to move from one system to a connected system.

Minimum behavior:

- validate current system
- validate target system is connected by jump network
- update ship current system
- update player current system if player is aboard
- update crew current system if assigned to ship
- trigger fog-of-war update for destination

No travel time required yet.

No fuel cost required yet.

No interception required yet.

No UI travel planner required yet.

Those come later.

---

## Fog-of-War Integration

Sprint 4 must connect player/ship location to fog of war.

When player starts:

- starting system becomes known/surveyed/active depending on current design

When player moves to a new system:

- destination system becomes discovered
- destination may become surveyed or active observation
- previous system may downgrade from active to surveyed if no owned asset remains

Owned ships and crew should eventually extend visibility.

For Sprint 4:

- player current ship grants active observation
- player entering a system updates visibility
- player leaving a system removes active observation unless another asset remains

This is critical to the Fractured Stars vision.

Player knowledge must come from presence and assets.

---

## Economy Integration

Sprint 4 does not implement trading yet.

However, the player's ship must be ready to carry cargo.

Ship data should support:

- cargo capacity
- current cargo inventory
- future buy/sell operations

Player should be able to query markets only through visibility rules.

The market system remains server authoritative.

---

## Save / Load Considerations

Full save/load may be a later sprint, but Sprint 4 data must be serializable.

Data should be structured so future persistence can save:

- player profile
- player credits
- ship ownership
- ship location
- ship cargo
- crew records
- crew assignments
- player known information references

Do not create temporary-only player data that cannot be persisted.

---

## Multiplayer Considerations

Design now for multiple players later.

Even if only one player is tested in Sprint 4, data structures should support:

- multiple PlayerId values
- multiple players in same system
- multiple players owning separate ships
- visibility per player
- server-authoritative commands

Do not assume there is only one permanent player.

---

## Blueprint / Debug Requirements

Expose useful Blueprint/debug functions for testing.

Suggested functions:

- CreateTestPlayer
- CreateStarterShipForPlayer
- AddTestCrewMember
- GetPlayerProfile
- GetPlayerShip
- GetPlayerCrew
- MovePlayerShipToConnectedSystem
- PrintPlayerInfo
- PrintShipInfo
- PrintCrewInfo
- PrintPlayerLocation
- PrintPlayerFogOfWarState

Debug logs should clearly show:

- player id
- current system
- current location
- ship id
- ship location
- crew count
- known systems count

---

## Test Scenario

Sprint 4 should support this test:

1. Generate universe.
2. Initialize economy.
3. Initialize fog of war.
4. Create test player.
5. Create starter ship.
6. Place player and ship at valid generated starting location.
7. Add 1-3 crew members.
8. Mark starting system visible.
9. Move ship to connected system.
10. Verify player/ship/crew location changes.
11. Verify fog-of-war updates.
12. Print player, ship, crew, and visibility state.

Expected result:

Player exists inside generated universe.

Player owns a real ship.

Crew are assigned to ship.

Ship is located at a real system/location.

Movement updates location.

Fog of war updates from presence.

---

## Explicitly Out Of Scope

Sprint 4 does not include:

- character creation UI
- detailed backstory selection
- ship equipment system
- ship component customization
- ship combat
- crew morale
- crew leveling
- crew salaries
- trading UI
- missions
- news
- factions reacting to player
- reputation
- save/load implementation unless already planned
- multiplayer lobby UI
- direct FPS/TPS character control

This sprint is the foundation only.

---

## Acceptance Criteria

Sprint 4 succeeds when:

- Unreal PlayerController / Pawn are clearly separate from in-universe player data.
- A persistent player profile can be created.
- Player starts at a valid generated location.
- Player owns a starter ship.
- Starter ship has location, cargo capacity, fuel capacity, and crew capacity.
- Crew records can be created.
- Crew can be assigned to player's ship.
- Player, ship, and crew reference valid SystemId / LocationId data.
- Basic connected-system movement works.
- Moving updates player/ship/crew location.
- Fog of war updates based on player/ship presence.
- Data is server-authoritative.
- Client is command/request based, not state-authoritative.
- Debug logs can print player, ship, crew, and location state.
- Data structures are ready for future save/load.

---

## Developer Notes

Do not build the game player as an FPS character pawn.

Do not make gameplay depend on possessing a humanoid actor.

The Unreal pawn is mainly the camera/view/control object.

The game player is persistent simulation data.

Fractured Stars is closer to RTS / management / living universe simulation than FPS/TPS.

The player should feel like a person commanding ships and crew inside a living universe, not like a camera pretending to be a body.

This distinction is critical for the long-term architecture.
