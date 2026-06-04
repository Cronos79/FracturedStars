# Sprint03.5_FogOfWar_Spec.md

# Fog of War and Information Visibility Specification

## Purpose

This document defines the information visibility system for Fractured Stars.

The goal is to separate:

* Reality
* Player Knowledge

The player should never possess perfect information.

The universe exists independently of the player's awareness.

A critical design principle of Fractured Stars is:

The player makes decisions using incomplete information.

---

# Core Design Philosophy

There are always two states:

## Real State

The actual universe.

Examples:

* actual inventories
* actual shortages
* actual prices
* actual faction activity
* actual conflicts
* actual ship movement
* actual economic conditions

The server always knows the real state.

---

## Known State

What the player believes to be true.

Known state may be:

* accurate
* delayed
* incomplete
* outdated
* intentionally misleading

The player interacts with known state.

Not real state.

---

# Visibility Sources

Players gain information through visibility sources.

Examples:

* player location
* player ship
* crew members
* owned stations
* owned colonies
* owned fleets
* intelligence assets
* purchased reports
* news networks

Information quality depends on source.

---

# System Visibility

Each star system has a visibility state.

## Hidden

Player has never discovered system.

Player knows nothing.

May appear as:

* unknown space
* unexplored region

---

## Known

Player knows system exists.

Player knows:

* name
* location
* jump connections

Player does not know current conditions.

---

## Surveyed

Player has visited system.

Player knows static information.

Examples:

* planets
* stations
* factions
* locations

Dynamic information may be outdated.

---

## Active Observation

Player or trusted asset currently present.

Information updates in near real-time.

Examples:

* market prices
* shortages
* ship activity
* missions
* local events

---

# Information Freshness

All information should include age.

Examples:

Food shortage report:

* 2 hours old

Market prices:

* 4 days old

Faction activity:

* 7 days old

Old information may be inaccurate.

---

# Crew Visibility

Crew extend visibility.

Examples:

Player in Sol.

Crew operating in Sirius.

Player receives current information from both locations.

Crew act as remote eyes and ears.

This is a major progression mechanic.

More assets = better information.

---

# Fleet Visibility

Owned ships provide visibility.

Examples:

Cargo vessel
Mining vessel
Escort vessel
Patrol vessel

Presence creates information.

Information should update automatically while assets remain active.

---

# Economic Visibility

Market information follows fog-of-war rules.

Player should not instantly know:

* every price
* every shortage
* every surplus

Information becomes available through:

* visits
* crew
* owned assets
* purchased trade data
* intelligence services

---

# News System Integration

News is information.

Not truth.

News reports may be:

* accurate
* delayed
* incomplete
* biased
* propaganda

News should reveal opportunities without revealing everything.

Example:

News reports:
"Food shortages worsening on Mars."

Player learns:

* something is wrong

Player does not automatically learn:

* exact food quantity
* exact prices
* exact inventory

Investigation may be required.

---

# Intelligence Layer

Future system.

Information sources may include:

* spies
* informants
* intelligence agencies
* purchased reports
* faction contacts

Better intelligence reduces uncertainty.

---

# Multiplayer Visibility

Clients must never receive full universe state.

Clients only receive information allowed by visibility rules.

The server remains authoritative.

Client visibility should be filtered before transmission.

Fog of war is both:

* gameplay system
* bandwidth optimization system

---

# Data Model Requirements

Each visible object should track:

* IsKnown
* LastObserved
* ObservationSource
* InformationConfidence
* InformationAge

Markets should support:

* LastKnownInventory
* LastKnownPrices
* LastKnownShortages

Separate from real values.

---

# Example Scenario

Player located far from Sol.

News reports:

"Growing tensions between Earth and Mars."

Player receives:

* headline
* summary
* general concern

Player does NOT receive:

* exact inventories
* exact shortages
* exact political actions

Player travels to Mars.

Visibility improves.

Player discovers:

* severe food shortage
* black market activity
* smuggling opportunities
* political unrest

The information system created the opportunity for discovery.

This is the intended gameplay experience.

---

# Explicitly Out Of Scope

This sprint does not include:

* news generation
* faction diplomacy
* missions
* espionage gameplay
* intelligence agencies
* reputation systems

Only visibility architecture and information filtering.

---

# Acceptance Criteria

✓ Hidden systems exist.

✓ Known systems exist.

✓ Surveyed systems exist.

✓ Active observation exists.

✓ Information age is tracked.

✓ Crew extend visibility.

✓ Owned ships extend visibility.

✓ Market data supports stale information.

✓ Real state and known state are separate.

✓ Server filters information before sending to clients.

✓ Fog of war works in single-player and multiplayer.

✓ Future systems can integrate cleanly.
