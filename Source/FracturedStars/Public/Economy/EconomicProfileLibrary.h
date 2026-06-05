// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Universe/UniverseTypes.h"
#include "Universe/EconomyTypes.h"
#include "EconomicProfileLibrary.generated.h"

/**
 * Economic Profile Library
 * 
 * Sprint 5: Defines production and consumption profiles for each location type
 * 
 * Based on Sprint 5 spec location profiles:
 * - Agricultural World: Produces Food/Water, Consumes Machinery/Fuel/Consumer Goods
 * - Mining Colony: Produces Ore, Consumes Food/Water/Fuel/Machinery/Medicine
 * - Industrial World: Produces Machinery/Electronics/Parts, Consumes Ore/Fuel/Food
 * - Shipyard: Produces Ship Parts/Advanced Components, Consumes Metals/Machinery/Electronics/Fuel
 * - Research Station: Produces Research Materials/Medical Supplies, Consumes Electronics/Food/Medicine
 * - Military Base: Minimal production, Consumes Food/Fuel/Weapons/Ship Parts/Medicine
 * - Pirate Outpost: Minimal production, Consumes Food/Fuel/Weapons
 */
UCLASS()
class FRACTUREDSTARS_API UEconomicProfileLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Create economic profile for a location type
	 * @param LocationType - Type of location
	 * @param Population - Population size (affects consumption scaling)
	 * @return Economic profile with production and consumption
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Profiles")
	static FLocationEconomicProfile CreateProfileForLocationType(ELocationType LocationType, int32 Population);

	/**
	 * Create profile for Agricultural World
	 */
	static FLocationEconomicProfile CreateAgriculturalProfile(int32 Population);

	/**
	 * Create profile for Mining Colony
	 */
	static FLocationEconomicProfile CreateMiningProfile(int32 Population);

	/**
	 * Create profile for Industrial World
	 */
	static FLocationEconomicProfile CreateIndustrialProfile(int32 Population);

	/**
	 * Create profile for Shipyard
	 */
	static FLocationEconomicProfile CreateShipyardProfile(int32 Population);

	/**
	 * Create profile for Research Station
	 */
	static FLocationEconomicProfile CreateResearchProfile(int32 Population);

	/**
	 * Create profile for Military Base
	 */
	static FLocationEconomicProfile CreateMilitaryProfile(int32 Population);

	/**
	 * Create profile for Pirate Outpost
	 */
	static FLocationEconomicProfile CreatePirateProfile(int32 Population);

	/**
	 * Create profile for Space Station (general purpose)
	 */
	static FLocationEconomicProfile CreateStationProfile(int32 Population);

	/**
	 * Create profile for Trade Hub
	 */
	static FLocationEconomicProfile CreateTradeHubProfile(int32 Population);

	/**
	 * Create profile for Colony (civilian settlement)
	 */
	static FLocationEconomicProfile CreateColonyProfile(int32 Population);

	/**
	 * Create profile for Refueling Depot
	 */
	static FLocationEconomicProfile CreateRefuelingDepotProfile(int32 Population);

	/**
	 * Create profile for Abandoned Facility (no production/consumption)
	 */
	static FLocationEconomicProfile CreateAbandonedProfile(int32 Population);

	// ========================================================================
	// SPRINT 5.5: Ship Component Manufacturing Profiles
	// ========================================================================

	/**
	 * Create profile for Engine Factory
	 * Produces: Engines (all tiers/sizes)
	 * Consumes: Refined Metals, Electronics, Industrial Parts, Fuel
	 */
	static FLocationEconomicProfile CreateEngineFactoryProfile(int32 Population);

	/**
	 * Create profile for Power Plant Factory
	 * Produces: Power Plants (all tiers/sizes)
	 * Consumes: Refined Metals, Electronics, Advanced Components, Fuel
	 */
	static FLocationEconomicProfile CreatePowerPlantFactoryProfile(int32 Population);

	/**
	 * Create profile for Shield Generator Factory
	 * Produces: Shield Generators (all tiers/sizes)
	 * Consumes: Electronics, Advanced Components, Research Materials, Fuel
	 */
	static FLocationEconomicProfile CreateShieldFactoryProfile(int32 Population);

	/**
	 * Create profile for Weapon Manufacturing Plant
	 * Produces: Ship Weapons (lasers, cannons, missiles)
	 * Consumes: Refined Metals, Electronics, Weapons (base components), Fuel
	 */
	static FLocationEconomicProfile CreateWeaponPlantProfile(int32 Population);

	/**
	 * Create profile for Component Assembly Plant (utility components)
	 * Produces: Utility components (mining lasers, cargo, fuel tanks, sensors)
	 * Consumes: Machinery, Electronics, Industrial Parts, Fuel
	 */
	static FLocationEconomicProfile CreateComponentAssemblyProfile(int32 Population);

	/**
	 * Create profile for Advanced Shipyard (builds frames + final assembly)
	 * Produces: Ship Frames (all classes)
	 * Consumes: Refined Metals, Machinery, Electronics, Advanced Components, Fuel
	 */
	static FLocationEconomicProfile CreateAdvancedShipyardProfile(int32 Population);
};
