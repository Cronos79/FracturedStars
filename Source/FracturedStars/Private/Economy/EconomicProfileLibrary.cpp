// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Economy/EconomicProfileLibrary.h"

FLocationEconomicProfile UEconomicProfileLibrary::CreateProfileForLocationType(ELocationType LocationType, int32 Population)
{
	switch (LocationType)
	{
	case ELocationType::Colony:
		return CreateAgriculturalProfile(Population); // Most colonies are agricultural
	case ELocationType::MiningColony:
		return CreateMiningProfile(Population);
	case ELocationType::Station:
		return CreateStationProfile(Population);
	case ELocationType::TradeHub:
		return CreateTradeHubProfile(Population);
	case ELocationType::ResearchFacility:
		return CreateResearchProfile(Population);
	case ELocationType::MilitaryBase:
		return CreateMilitaryProfile(Population);
	case ELocationType::PirateOutpost:
		return CreatePirateProfile(Population);
	case ELocationType::Shipyard:
		return CreateShipyardProfile(Population);
	case ELocationType::RefuelingDepot:
		return CreateRefuelingDepotProfile(Population);
	case ELocationType::AbandonedFacility:
		return CreateAbandonedProfile(Population);
	default:
		return CreateColonyProfile(Population);
	}
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateAgriculturalProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Food and Water (raw extraction from planetary resources)
	FProductionRecipe FoodProduction;
	FoodProduction.OutputGood = EGoodType::Food;
	FoodProduction.OutputQuantity = 200; // Large food production
	// No inputs required (farming/harvesting)
	Profile.ProductionRecipes.Add(FoodProduction);

	FProductionRecipe WaterProduction;
	WaterProduction.OutputGood = EGoodType::Water;
	WaterProduction.OutputQuantity = 150;
	Profile.ProductionRecipes.Add(WaterProduction);

	// CONSUMPTION: Machinery (farming equipment), Fuel (tractors/harvesters), Consumer Goods
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 10, 0.0005f, false));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 20, 0.001f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::ConsumerGoods, 5, 0.002f, false));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 5, 0.0005f, true));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateMiningProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Ore (raw extraction)
	FProductionRecipe OreProduction;
	OreProduction.OutputGood = EGoodType::Ore;
	OreProduction.OutputQuantity = 300; // High ore output
	// Requires fuel for mining equipment
	OreProduction.Inputs.Add(FProductionInput(EGoodType::Fuel, 30));
	Profile.ProductionRecipes.Add(OreProduction);

	// CONSUMPTION: Food, Water, Fuel, Machinery (mining equipment), Medicine (hazardous work)
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 20, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 15, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 40, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 15, 0.001f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 10, 0.001f, true));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateIndustrialProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Machinery, Electronics, Industrial Parts
	FProductionRecipe MachineryProduction;
	MachineryProduction.OutputGood = EGoodType::Machinery;
	MachineryProduction.OutputQuantity = 100;
	MachineryProduction.Inputs.Add(FProductionInput(EGoodType::Ore, 80));
	MachineryProduction.Inputs.Add(FProductionInput(EGoodType::Fuel, 20));
	Profile.ProductionRecipes.Add(MachineryProduction);

	FProductionRecipe ElectronicsProduction;
	ElectronicsProduction.OutputGood = EGoodType::Electronics;
	ElectronicsProduction.OutputQuantity = 80;
	ElectronicsProduction.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 60));
	ElectronicsProduction.Inputs.Add(FProductionInput(EGoodType::Fuel, 15));
	Profile.ProductionRecipes.Add(ElectronicsProduction);

	FProductionRecipe PartsProduction;
	PartsProduction.OutputGood = EGoodType::IndustrialParts;
	PartsProduction.OutputQuantity = 90;
	PartsProduction.Inputs.Add(FProductionInput(EGoodType::Ore, 70));
	PartsProduction.Inputs.Add(FProductionInput(EGoodType::Machinery, 10));
	Profile.ProductionRecipes.Add(PartsProduction);

	// Also refine metals
	FProductionRecipe MetalRefining;
	MetalRefining.OutputGood = EGoodType::RefinedMetals;
	MetalRefining.OutputQuantity = 120;
	MetalRefining.Inputs.Add(FProductionInput(EGoodType::Ore, 150));
	MetalRefining.Inputs.Add(FProductionInput(EGoodType::Fuel, 25));
	Profile.ProductionRecipes.Add(MetalRefining);

	// CONSUMPTION: Ore, Fuel, Food
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 15, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 10, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 50, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 5, 0.0005f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateShipyardProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Advanced Components (ship-specific)
	FProductionRecipe ComponentProduction;
	ComponentProduction.OutputGood = EGoodType::AdvancedComponents;
	ComponentProduction.OutputQuantity = 50;
	ComponentProduction.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 80));
	ComponentProduction.Inputs.Add(FProductionInput(EGoodType::Electronics, 40));
	ComponentProduction.Inputs.Add(FProductionInput(EGoodType::Machinery, 30));
	Profile.ProductionRecipes.Add(ComponentProduction);

	// CONSUMPTION: Heavy industrial needs
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 10, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 8, 0.001f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 60, 0.004f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::RefinedMetals, 100, 0.005f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Electronics, 50, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 40, 0.002f, true));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateResearchProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Research Materials, Medical Supplies (advanced science)
	FProductionRecipe ResearchProduction;
	ResearchProduction.OutputGood = EGoodType::ResearchMaterials;
	ResearchProduction.OutputQuantity = 30;
	ResearchProduction.Inputs.Add(FProductionInput(EGoodType::Electronics, 20));
	ResearchProduction.Inputs.Add(FProductionInput(EGoodType::Medicine, 10));
	Profile.ProductionRecipes.Add(ResearchProduction);

	FProductionRecipe MedicalProduction;
	MedicalProduction.OutputGood = EGoodType::Medicine;
	MedicalProduction.OutputQuantity = 40;
	MedicalProduction.Inputs.Add(FProductionInput(EGoodType::Water, 15));
	MedicalProduction.Inputs.Add(FProductionInput(EGoodType::Electronics, 10));
	Profile.ProductionRecipes.Add(MedicalProduction);

	// CONSUMPTION: Electronics, Food, Medicine (research/medical staff)
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 8, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 6, 0.001f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 15, 0.001f, false));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Electronics, 25, 0.002f, true));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateMilitaryProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Weapons (limited production, mostly imports)
	FProductionRecipe WeaponProduction;
	WeaponProduction.OutputGood = EGoodType::Weapons;
	WeaponProduction.OutputQuantity = 20;
	WeaponProduction.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 30));
	WeaponProduction.Inputs.Add(FProductionInput(EGoodType::Electronics, 15));
	Profile.ProductionRecipes.Add(WeaponProduction);

	// CONSUMPTION: Heavy consumer (military operations)
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 25, 0.004f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 20, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 80, 0.006f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Weapons, 30, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::AdvancedComponents, 20, 0.002f, false));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 15, 0.002f, true));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreatePirateProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Minimal (maybe some contraband later)
	// For now, no production

	// CONSUMPTION: Basic needs only (stolen/smuggled goods assumed)
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 10, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 8, 0.001f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 40, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Weapons, 15, 0.002f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateStationProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Consumer Goods (light manufacturing/services)
	FProductionRecipe ConsumerProduction;
	ConsumerProduction.OutputGood = EGoodType::ConsumerGoods;
	ConsumerProduction.OutputQuantity = 60;
	ConsumerProduction.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 30));
	ConsumerProduction.Inputs.Add(FProductionInput(EGoodType::Electronics, 20));
	Profile.ProductionRecipes.Add(ConsumerProduction);

	// CONSUMPTION: General station needs
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 12, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 10, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 20, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 8, 0.001f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateTradeHubProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Consumer Goods (from trade/services)
	FProductionRecipe ConsumerProduction;
	ConsumerProduction.OutputGood = EGoodType::ConsumerGoods;
	ConsumerProduction.OutputQuantity = 80;
	ConsumerProduction.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 40));
	Profile.ProductionRecipes.Add(ConsumerProduction);

	// CONSUMPTION: High population/trade activity
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 18, 0.004f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 15, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 30, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::ConsumerGoods, 10, 0.002f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateColonyProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Basic food/consumer goods (self-sufficient small colony)
	FProductionRecipe FoodProduction;
	FoodProduction.OutputGood = EGoodType::Food;
	FoodProduction.OutputQuantity = 50;
	Profile.ProductionRecipes.Add(FoodProduction);

	FProductionRecipe ConsumerProduction;
	ConsumerProduction.OutputGood = EGoodType::ConsumerGoods;
	ConsumerProduction.OutputQuantity = 30;
	ConsumerProduction.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 15));
	Profile.ProductionRecipes.Add(ConsumerProduction);

	// CONSUMPTION: Basic needs
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 8, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 15, 0.001f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 5, 0.0005f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 5, 0.0003f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateRefuelingDepotProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Fuel (refining/storage)
	// For now, assume fuel "produced" from imports or local refining
	// Could add recipe later: Ore -> Fuel refining

	// CONSUMPTION: Minimal (automated depot)
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 5, 0.001f, false));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 4, 0.0008f, false));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 3, 0.0002f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateAbandonedProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// No production
	// No consumption

	return Profile;
}

// ============================================================================
// SPRINT 5.5: Ship Component Manufacturing Profiles
// ============================================================================

FLocationEconomicProfile UEconomicProfileLibrary::CreateEngineFactoryProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Various engine tiers and sizes
	// Tier I Small Engine
	FProductionRecipe EngineT1Small;
	EngineT1Small.OutputGood = EGoodType::Engine_TitanI_Small;
	EngineT1Small.OutputQuantity = 5;
	EngineT1Small.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 20));
	EngineT1Small.Inputs.Add(FProductionInput(EGoodType::Electronics, 10));
	EngineT1Small.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 15));
	EngineT1Small.Inputs.Add(FProductionInput(EGoodType::Fuel, 5));
	Profile.ProductionRecipes.Add(EngineT1Small);

	// Tier II Small Engine (higher quality, more inputs)
	FProductionRecipe EngineT2Small;
	EngineT2Small.OutputGood = EGoodType::Engine_TitanII_Small;
	EngineT2Small.OutputQuantity = 3;
	EngineT2Small.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 30));
	EngineT2Small.Inputs.Add(FProductionInput(EGoodType::Electronics, 20));
	EngineT2Small.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 25));
	EngineT2Small.Inputs.Add(FProductionInput(EGoodType::Fuel, 8));
	Profile.ProductionRecipes.Add(EngineT2Small);

	// Medium engines (higher output, higher inputs)
	FProductionRecipe EngineT1Medium;
	EngineT1Medium.OutputGood = EGoodType::Engine_TitanI_Medium;
	EngineT1Medium.OutputQuantity = 2;
	EngineT1Medium.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 50));
	EngineT1Medium.Inputs.Add(FProductionInput(EGoodType::Electronics, 30));
	EngineT1Medium.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 40));
	EngineT1Medium.Inputs.Add(FProductionInput(EGoodType::Fuel, 10));
	Profile.ProductionRecipes.Add(EngineT1Medium);

	// CONSUMPTION: Factory operations
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 15, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 12, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 30, 0.005f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 8, 0.001f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 10, 0.002f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreatePowerPlantFactoryProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Power plants (various tiers/sizes)
	// Tier I Small Power Plant
	FProductionRecipe PowerPlantT1Small;
	PowerPlantT1Small.OutputGood = EGoodType::PowerPlant_NovaI_Small;
	PowerPlantT1Small.OutputQuantity = 5;
	PowerPlantT1Small.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 25));
	PowerPlantT1Small.Inputs.Add(FProductionInput(EGoodType::Electronics, 20));
	PowerPlantT1Small.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 10));
	PowerPlantT1Small.Inputs.Add(FProductionInput(EGoodType::Fuel, 8));
	Profile.ProductionRecipes.Add(PowerPlantT1Small);

	// Tier II Small Power Plant
	FProductionRecipe PowerPlantT2Small;
	PowerPlantT2Small.OutputGood = EGoodType::PowerPlant_NovaII_Small;
	PowerPlantT2Small.OutputQuantity = 3;
	PowerPlantT2Small.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 40));
	PowerPlantT2Small.Inputs.Add(FProductionInput(EGoodType::Electronics, 35));
	PowerPlantT2Small.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 20));
	PowerPlantT2Small.Inputs.Add(FProductionInput(EGoodType::Fuel, 12));
	Profile.ProductionRecipes.Add(PowerPlantT2Small);

	// Medium Power Plant
	FProductionRecipe PowerPlantT1Medium;
	PowerPlantT1Medium.OutputGood = EGoodType::PowerPlant_NovaI_Medium;
	PowerPlantT1Medium.OutputQuantity = 2;
	PowerPlantT1Medium.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 60));
	PowerPlantT1Medium.Inputs.Add(FProductionInput(EGoodType::Electronics, 50));
	PowerPlantT1Medium.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 30));
	PowerPlantT1Medium.Inputs.Add(FProductionInput(EGoodType::Fuel, 15));
	Profile.ProductionRecipes.Add(PowerPlantT1Medium);

	// CONSUMPTION: Factory operations
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 20, 0.004f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 15, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 40, 0.006f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 10, 0.0015f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 15, 0.003f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateShieldFactoryProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Shield generators (high-tech)
	// Tier I Small Shield
	FProductionRecipe ShieldT1Small;
	ShieldT1Small.OutputGood = EGoodType::Shield_AtlasI_Small;
	ShieldT1Small.OutputQuantity = 4;
	ShieldT1Small.Inputs.Add(FProductionInput(EGoodType::Electronics, 30));
	ShieldT1Small.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 15));
	ShieldT1Small.Inputs.Add(FProductionInput(EGoodType::ResearchMaterials, 10));
	ShieldT1Small.Inputs.Add(FProductionInput(EGoodType::Fuel, 8));
	Profile.ProductionRecipes.Add(ShieldT1Small);

	// Tier II Small Shield
	FProductionRecipe ShieldT2Small;
	ShieldT2Small.OutputGood = EGoodType::Shield_AtlasII_Small;
	ShieldT2Small.OutputQuantity = 2;
	ShieldT2Small.Inputs.Add(FProductionInput(EGoodType::Electronics, 50));
	ShieldT2Small.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 30));
	ShieldT2Small.Inputs.Add(FProductionInput(EGoodType::ResearchMaterials, 20));
	ShieldT2Small.Inputs.Add(FProductionInput(EGoodType::Fuel, 12));
	Profile.ProductionRecipes.Add(ShieldT2Small);

	// Medium Shield
	FProductionRecipe ShieldT1Medium;
	ShieldT1Medium.OutputGood = EGoodType::Shield_AtlasI_Medium;
	ShieldT1Medium.OutputQuantity = 2;
	ShieldT1Medium.Inputs.Add(FProductionInput(EGoodType::Electronics, 70));
	ShieldT1Medium.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 50));
	ShieldT1Medium.Inputs.Add(FProductionInput(EGoodType::ResearchMaterials, 30));
	ShieldT1Medium.Inputs.Add(FProductionInput(EGoodType::Fuel, 15));
	Profile.ProductionRecipes.Add(ShieldT1Medium);

	// CONSUMPTION: High-tech factory
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 18, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 14, 0.0025f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 35, 0.005f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 12, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 12, 0.002f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateWeaponPlantProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Ship weapons
	// Small Laser
	FProductionRecipe LaserSmall;
	LaserSmall.OutputGood = EGoodType::Weapon_Laser_Small;
	LaserSmall.OutputQuantity = 6;
	LaserSmall.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 15));
	LaserSmall.Inputs.Add(FProductionInput(EGoodType::Electronics, 25));
	LaserSmall.Inputs.Add(FProductionInput(EGoodType::Weapons, 10)); // Base weapon components
	LaserSmall.Inputs.Add(FProductionInput(EGoodType::Fuel, 5));
	Profile.ProductionRecipes.Add(LaserSmall);

	// Small Cannon
	FProductionRecipe CannonSmall;
	CannonSmall.OutputGood = EGoodType::Weapon_Cannon_Small;
	CannonSmall.OutputQuantity = 5;
	CannonSmall.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 30));
	CannonSmall.Inputs.Add(FProductionInput(EGoodType::Electronics, 15));
	CannonSmall.Inputs.Add(FProductionInput(EGoodType::Weapons, 15));
	CannonSmall.Inputs.Add(FProductionInput(EGoodType::Fuel, 6));
	Profile.ProductionRecipes.Add(CannonSmall);

	// Missile Launcher
	FProductionRecipe MissileLauncher;
	MissileLauncher.OutputGood = EGoodType::Weapon_MissileLauncher_Small;
	MissileLauncher.OutputQuantity = 3;
	MissileLauncher.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 25));
	MissileLauncher.Inputs.Add(FProductionInput(EGoodType::Electronics, 30));
	MissileLauncher.Inputs.Add(FProductionInput(EGoodType::Weapons, 20));
	MissileLauncher.Inputs.Add(FProductionInput(EGoodType::Fuel, 8));
	Profile.ProductionRecipes.Add(MissileLauncher);

	// CONSUMPTION: Military-industrial facility
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 15, 0.003f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 12, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 30, 0.005f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 10, 0.0015f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 15, 0.003f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateComponentAssemblyProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Utility components
	// Small Mining Laser
	FProductionRecipe MiningLaserSmall;
	MiningLaserSmall.OutputGood = EGoodType::Utility_MiningLaser_Small;
	MiningLaserSmall.OutputQuantity = 5;
	MiningLaserSmall.Inputs.Add(FProductionInput(EGoodType::Machinery, 20));
	MiningLaserSmall.Inputs.Add(FProductionInput(EGoodType::Electronics, 25));
	MiningLaserSmall.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 15));
	MiningLaserSmall.Inputs.Add(FProductionInput(EGoodType::Fuel, 5));
	Profile.ProductionRecipes.Add(MiningLaserSmall);

	// Small Cargo Expansion
	FProductionRecipe CargoSmall;
	CargoSmall.OutputGood = EGoodType::Utility_CargoExpansion_Small;
	CargoSmall.OutputQuantity = 8;
	CargoSmall.Inputs.Add(FProductionInput(EGoodType::Machinery, 15));
	CargoSmall.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 20));
	CargoSmall.Inputs.Add(FProductionInput(EGoodType::Fuel, 3));
	Profile.ProductionRecipes.Add(CargoSmall);

	// Fuel Tank
	FProductionRecipe FuelTank;
	FuelTank.OutputGood = EGoodType::Utility_FuelTank_Small;
	FuelTank.OutputQuantity = 8;
	FuelTank.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 20));
	FuelTank.Inputs.Add(FProductionInput(EGoodType::IndustrialParts, 15));
	FuelTank.Inputs.Add(FProductionInput(EGoodType::Fuel, 3));
	Profile.ProductionRecipes.Add(FuelTank);

	// Sensor Array
	FProductionRecipe Sensor;
	Sensor.OutputGood = EGoodType::Utility_SensorArray_Small;
	Sensor.OutputQuantity = 6;
	Sensor.Inputs.Add(FProductionInput(EGoodType::Electronics, 30));
	Sensor.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 10));
	Sensor.Inputs.Add(FProductionInput(EGoodType::Fuel, 4));
	Profile.ProductionRecipes.Add(Sensor);

	// CONSUMPTION: Assembly plant operations
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 12, 0.0025f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 10, 0.002f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 25, 0.004f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 8, 0.001f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 10, 0.002f, false));

	return Profile;
}

FLocationEconomicProfile UEconomicProfileLibrary::CreateAdvancedShipyardProfile(int32 Population)
{
	FLocationEconomicProfile Profile;

	// PRODUCTION: Ship frames (capital-intensive, long production chains)
	// Starter Mining Frame
	FProductionRecipe FrameStarterMining;
	FrameStarterMining.OutputGood = EGoodType::ShipFrame_StarterMining;
	FrameStarterMining.OutputQuantity = 1; // Ships are expensive and slow to build
	FrameStarterMining.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 100));
	FrameStarterMining.Inputs.Add(FProductionInput(EGoodType::Machinery, 50));
	FrameStarterMining.Inputs.Add(FProductionInput(EGoodType::Electronics, 30));
	FrameStarterMining.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 20));
	FrameStarterMining.Inputs.Add(FProductionInput(EGoodType::Fuel, 15));
	Profile.ProductionRecipes.Add(FrameStarterMining);

	// Starter Trade Frame
	FProductionRecipe FrameStarterTrade;
	FrameStarterTrade.OutputGood = EGoodType::ShipFrame_StarterTrade;
	FrameStarterTrade.OutputQuantity = 1;
	FrameStarterTrade.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 100));
	FrameStarterTrade.Inputs.Add(FProductionInput(EGoodType::Machinery, 50));
	FrameStarterTrade.Inputs.Add(FProductionInput(EGoodType::Electronics, 30));
	FrameStarterTrade.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 20));
	FrameStarterTrade.Inputs.Add(FProductionInput(EGoodType::Fuel, 15));
	Profile.ProductionRecipes.Add(FrameStarterTrade);

	// Medium Freighter Frame (more expensive)
	FProductionRecipe FrameMediumFreighter;
	FrameMediumFreighter.OutputGood = EGoodType::ShipFrame_MediumFreighter;
	FrameMediumFreighter.OutputQuantity = 1;
	FrameMediumFreighter.Inputs.Add(FProductionInput(EGoodType::RefinedMetals, 250));
	FrameMediumFreighter.Inputs.Add(FProductionInput(EGoodType::Machinery, 120));
	FrameMediumFreighter.Inputs.Add(FProductionInput(EGoodType::Electronics, 80));
	FrameMediumFreighter.Inputs.Add(FProductionInput(EGoodType::AdvancedComponents, 60));
	FrameMediumFreighter.Inputs.Add(FProductionInput(EGoodType::Fuel, 30));
	Profile.ProductionRecipes.Add(FrameMediumFreighter);

	// CONSUMPTION: Massive shipyard operations
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Food, 50, 0.01f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Water, 40, 0.008f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Fuel, 100, 0.015f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Medicine, 25, 0.005f, true));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Machinery, 40, 0.008f, false));
	Profile.ConsumptionProfile.Add(FConsumptionEntry(EGoodType::Electronics, 30, 0.006f, false));

	return Profile;
}
