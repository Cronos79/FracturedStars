// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/EconomySubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create static goods catalog
	GoodsCatalog = CreateGoodsCatalog();

	UE_LOG(LogTemp, Log, TEXT("[EconomySubsystem] Initialized with %d goods"), GoodsCatalog.Num());
}

void UEconomySubsystem::Deinitialize()
{
	// Clear timer
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWorld* World = GameInstance->GetWorld())
		{
			World->GetTimerManager().ClearTimer(ActiveSystemTickTimer);
		}
	}

	Super::Deinitialize();
}

TMap<EGoodType, FGoodDefinition> UEconomySubsystem::CreateGoodsCatalog() const
{
	TMap<EGoodType, FGoodDefinition> Catalog;

	// Survival goods
	{
		FGoodDefinition Food;
		Food.GoodType = EGoodType::Food;
		Food.DisplayName = TEXT("Food");
		Food.Category = EGoodCategory::Survival;
		Food.BasePrice = 5.0f;
		Food.bIsEssential = true;
		Food.bIsLegal = true;
		Food.CargoUnitSize = 1.0f;
		Catalog.Add(EGoodType::Food, Food);
	}

	{
		FGoodDefinition Water;
		Water.GoodType = EGoodType::Water;
		Water.DisplayName = TEXT("Water");
		Water.Category = EGoodCategory::Survival;
		Water.BasePrice = 3.0f;
		Water.bIsEssential = true;
		Water.bIsLegal = true;
		Water.CargoUnitSize = 1.0f;
		Catalog.Add(EGoodType::Water, Water);
	}

	{
		FGoodDefinition Fuel;
		Fuel.GoodType = EGoodType::Fuel;
		Fuel.DisplayName = TEXT("Fuel");
		Fuel.Category = EGoodCategory::Survival;
		Fuel.BasePrice = 15.0f;
		Fuel.bIsEssential = true;
		Fuel.bIsLegal = true;
		Fuel.CargoUnitSize = 2.0f;
		Catalog.Add(EGoodType::Fuel, Fuel);
	}

	{
		FGoodDefinition Medicine;
		Medicine.GoodType = EGoodType::Medicine;
		Medicine.DisplayName = TEXT("Medicine");
		Medicine.Category = EGoodCategory::Survival;
		Medicine.BasePrice = 50.0f;
		Medicine.bIsEssential = true;
		Medicine.bIsLegal = true;
		Medicine.CargoUnitSize = 0.5f;
		Catalog.Add(EGoodType::Medicine, Medicine);
	}

	// Industrial goods
	{
		FGoodDefinition Ore;
		Ore.GoodType = EGoodType::Ore;
		Ore.DisplayName = TEXT("Ore");
		Ore.Category = EGoodCategory::Industrial;
		Ore.BasePrice = 10.0f;
		Ore.bIsEssential = false;
		Ore.bIsLegal = true;
		Ore.CargoUnitSize = 5.0f;
		Catalog.Add(EGoodType::Ore, Ore);
	}

	{
		FGoodDefinition RefinedMetals;
		RefinedMetals.GoodType = EGoodType::RefinedMetals;
		RefinedMetals.DisplayName = TEXT("Refined Metals");
		RefinedMetals.Category = EGoodCategory::Industrial;
		RefinedMetals.BasePrice = 30.0f;
		RefinedMetals.bIsEssential = false;
		RefinedMetals.bIsLegal = true;
		RefinedMetals.CargoUnitSize = 3.0f;
		Catalog.Add(EGoodType::RefinedMetals, RefinedMetals);
	}

	{
		FGoodDefinition Machinery;
		Machinery.GoodType = EGoodType::Machinery;
		Machinery.DisplayName = TEXT("Machinery");
		Machinery.Category = EGoodCategory::Industrial;
		Machinery.BasePrice = 100.0f;
		Machinery.bIsEssential = false;
		Machinery.bIsLegal = true;
		Machinery.CargoUnitSize = 10.0f;
		Catalog.Add(EGoodType::Machinery, Machinery);
	}

	{
		FGoodDefinition IndustrialParts;
		IndustrialParts.GoodType = EGoodType::IndustrialParts;
		IndustrialParts.DisplayName = TEXT("Industrial Parts");
		IndustrialParts.Category = EGoodCategory::Industrial;
		IndustrialParts.BasePrice = 40.0f;
		IndustrialParts.bIsEssential = false;
		IndustrialParts.bIsLegal = true;
		IndustrialParts.CargoUnitSize = 2.0f;
		Catalog.Add(EGoodType::IndustrialParts, IndustrialParts);
	}

	// High tech goods
	{
		FGoodDefinition Electronics;
		Electronics.GoodType = EGoodType::Electronics;
		Electronics.DisplayName = TEXT("Electronics");
		Electronics.Category = EGoodCategory::HighTech;
		Electronics.BasePrice = 80.0f;
		Electronics.bIsEssential = false;
		Electronics.bIsLegal = true;
		Electronics.CargoUnitSize = 1.0f;
		Catalog.Add(EGoodType::Electronics, Electronics);
	}

	{
		FGoodDefinition AdvancedComponents;
		AdvancedComponents.GoodType = EGoodType::AdvancedComponents;
		AdvancedComponents.DisplayName = TEXT("Advanced Components");
		AdvancedComponents.Category = EGoodCategory::HighTech;
		AdvancedComponents.BasePrice = 200.0f;
		AdvancedComponents.bIsEssential = false;
		AdvancedComponents.bIsLegal = true;
		AdvancedComponents.CargoUnitSize = 1.5f;
		Catalog.Add(EGoodType::AdvancedComponents, AdvancedComponents);
	}

	{
		FGoodDefinition ResearchMaterials;
		ResearchMaterials.GoodType = EGoodType::ResearchMaterials;
		ResearchMaterials.DisplayName = TEXT("Research Materials");
		ResearchMaterials.Category = EGoodCategory::HighTech;
		ResearchMaterials.BasePrice = 150.0f;
		ResearchMaterials.bIsEssential = false;
		ResearchMaterials.bIsLegal = true;
		ResearchMaterials.CargoUnitSize = 1.0f;
		Catalog.Add(EGoodType::ResearchMaterials, ResearchMaterials);
	}

	// Military goods
	{
		FGoodDefinition Weapons;
		Weapons.GoodType = EGoodType::Weapons;
		Weapons.DisplayName = TEXT("Weapons");
		Weapons.Category = EGoodCategory::Military;
		Weapons.BasePrice = 120.0f;
		Weapons.bIsEssential = false;
		Weapons.bIsLegal = true; // Legal in most places
		Weapons.CargoUnitSize = 2.0f;
		Catalog.Add(EGoodType::Weapons, Weapons);
	}

	// Luxury goods
	{
		FGoodDefinition ConsumerGoods;
		ConsumerGoods.GoodType = EGoodType::ConsumerGoods;
		ConsumerGoods.DisplayName = TEXT("Consumer Goods");
		ConsumerGoods.Category = EGoodCategory::Luxury;
		ConsumerGoods.BasePrice = 25.0f;
		ConsumerGoods.bIsEssential = false;
		ConsumerGoods.bIsLegal = true;
		ConsumerGoods.CargoUnitSize = 1.5f;
		Catalog.Add(EGoodType::ConsumerGoods, ConsumerGoods);
	}

	// Illegal goods
	{
		FGoodDefinition Contraband;
		Contraband.GoodType = EGoodType::Contraband;
		Contraband.DisplayName = TEXT("Contraband");
		Contraband.Category = EGoodCategory::Illegal;
		Contraband.BasePrice = 300.0f;
		Contraband.bIsEssential = false;
		Contraband.bIsLegal = false;
		Contraband.CargoUnitSize = 1.0f;
		Catalog.Add(EGoodType::Contraband, Contraband);
	}

	return Catalog;
}

void UEconomySubsystem::InitializeEconomy(FUniverseData& UniverseData)
{
	if (UniverseData.Systems.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EconomySubsystem] Cannot initialize economy: no systems in universe"));
		return;
	}

	int32 TotalLocations = 0;
	int32 TotalMarkets = 0;

	// Create deterministic RNG from universe seed
	FRandomStream RNG(UniverseData.Config.Seed);

	// Initialize markets for all locations
	for (FStarSystemData& System : UniverseData.Systems)
	{
		for (FLocationData& Location : System.Locations)
		{
			InitializeLocationMarket(Location, RNG);
			TotalLocations++;

			if (Location.Market.Goods.Num() > 0)
			{
				TotalMarkets++;
			}
		}
	}

	// Initialize game time
	UniverseData.CurrentGameTime = 0.0;
	UniverseData.ActiveSystemId = -1;

	UE_LOG(LogTemp, Log, TEXT("[EconomySubsystem] Economy initialized: %d locations, %d active markets"),
		TotalLocations, TotalMarkets);
}

void UEconomySubsystem::InitializeLocationMarket(FLocationData& Location, FRandomStream& RNG)
{
	Location.Market.Goods.Empty();
	Location.Market.LastUpdateTime = 0.0;
	Location.Market.bIsActiveSimulation = false;

	// Determine which goods this location trades based on type
	TArray<EGoodType> TradedGoods;

	switch (Location.LocationType)
	{
	case ELocationType::TradeHub:
		// Trade hubs carry everything
		for (const auto& Pair : GoodsCatalog)
		{
			TradedGoods.Add(Pair.Key);
		}
		break;

	case ELocationType::MiningColony:
		// Mining colonies produce ore, consume food/water/fuel/machinery
		TradedGoods.Add(EGoodType::Ore);
		TradedGoods.Add(EGoodType::Food);
		TradedGoods.Add(EGoodType::Water);
		TradedGoods.Add(EGoodType::Fuel);
		TradedGoods.Add(EGoodType::Machinery);
		TradedGoods.Add(EGoodType::IndustrialParts);
		break;

	case ELocationType::ResearchFacility:
		// Research facilities produce/consume research materials and high-tech goods
		TradedGoods.Add(EGoodType::ResearchMaterials);
		TradedGoods.Add(EGoodType::Electronics);
		TradedGoods.Add(EGoodType::AdvancedComponents);
		TradedGoods.Add(EGoodType::Food);
		TradedGoods.Add(EGoodType::Water);
		TradedGoods.Add(EGoodType::Medicine);
		break;

	case ELocationType::MilitaryBase:
		// Military bases consume weapons, fuel, and supplies
		TradedGoods.Add(EGoodType::Weapons);
		TradedGoods.Add(EGoodType::Fuel);
		TradedGoods.Add(EGoodType::Food);
		TradedGoods.Add(EGoodType::Water);
		TradedGoods.Add(EGoodType::Medicine);
		TradedGoods.Add(EGoodType::Electronics);
		break;

	case ELocationType::Colony:
		// Colonies consume survival goods and consumer goods
		TradedGoods.Add(EGoodType::Food);
		TradedGoods.Add(EGoodType::Water);
		TradedGoods.Add(EGoodType::Medicine);
		TradedGoods.Add(EGoodType::ConsumerGoods);
		TradedGoods.Add(EGoodType::Fuel);
		break;

	case ELocationType::Shipyard:
		// Shipyards consume metals, machinery, electronics
		TradedGoods.Add(EGoodType::RefinedMetals);
		TradedGoods.Add(EGoodType::Machinery);
		TradedGoods.Add(EGoodType::Electronics);
		TradedGoods.Add(EGoodType::AdvancedComponents);
		TradedGoods.Add(EGoodType::IndustrialParts);
		TradedGoods.Add(EGoodType::Food);
		TradedGoods.Add(EGoodType::Water);
		break;

	case ELocationType::PirateOutpost:
		// Pirate outposts trade contraband and weapons
		TradedGoods.Add(EGoodType::Contraband);
		TradedGoods.Add(EGoodType::Weapons);
		TradedGoods.Add(EGoodType::Fuel);
		TradedGoods.Add(EGoodType::Food);
		TradedGoods.Add(EGoodType::Water);
		break;

	case ELocationType::RefuelingDepot:
		// Refueling depots focus on fuel
		TradedGoods.Add(EGoodType::Fuel);
		TradedGoods.Add(EGoodType::Food);
		TradedGoods.Add(EGoodType::Water);
		break;

	default:
		// Generic station: survival goods + common trade items
		TradedGoods.Add(EGoodType::Food);
		TradedGoods.Add(EGoodType::Water);
		TradedGoods.Add(EGoodType::Fuel);
		TradedGoods.Add(EGoodType::Medicine);
		TradedGoods.Add(EGoodType::ConsumerGoods);
		break;
	}

	// Create market entries for traded goods
	for (EGoodType GoodType : TradedGoods)
	{
		if (const FGoodDefinition* GoodDef = GoodsCatalog.Find(GoodType))
		{
			FMarketGoodEntry Entry;
			Entry.GoodType = GoodType;
			Entry.CurrentPrice = GoodDef->BasePrice * RNG.FRandRange(0.8f, 1.2f); // ±20% initial variance

			// Set production/consumption rates based on location's Produces/Consumes arrays
			Entry.ProductionRate = 0;
			Entry.ConsumptionRate = 0;

			for (const FResourceEntry& Prod : Location.Produces)
			{
				if (Prod.ResourceType == GoodType)
				{
					Entry.ProductionRate = Prod.Quantity;
					break;
				}
			}

			for (const FResourceEntry& Cons : Location.Consumes)
			{
				if (Cons.ResourceType == GoodType)
				{
					Entry.ConsumptionRate = Cons.Quantity;
					break;
				}
			}

			// Calculate target stock based on population and consumption
			int32 BaseTarget = FMath::Max(100, Location.Population / 100);
			Entry.TargetStock = BaseTarget * FMath::Max(1, Entry.ConsumptionRate);

			// Initialize stock near target with variance
			Entry.Stock = FMath::RandRange(
				FMath::Max(0, Entry.TargetStock - Entry.TargetStock / 4),
				Entry.TargetStock + Entry.TargetStock / 4
			);

			Entry.bIsShortage = false;
			Entry.bIsSurplus = false;

			Location.Market.Goods.Add(Entry);
		}
	}
}

void UEconomySubsystem::SetActiveSystem(FUniverseData& UniverseData, int32 SystemId)
{
	// Validate system ID
	if (!UniverseData.Systems.IsValidIndex(SystemId))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EconomySubsystem] Invalid system ID: %d"), SystemId);
		return;
	}

	// Clear old active system flag
	if (UniverseData.ActiveSystemId >= 0 && UniverseData.Systems.IsValidIndex(UniverseData.ActiveSystemId))
	{
		FStarSystemData& OldSystem = UniverseData.Systems[UniverseData.ActiveSystemId];
		for (FLocationData& Location : OldSystem.Locations)
		{
			Location.Market.bIsActiveSimulation = false;
		}
	}

	// Set new active system
	UniverseData.ActiveSystemId = SystemId;
	FStarSystemData& NewSystem = UniverseData.Systems[SystemId];

	// Catch up the new system to current time
	CatchUpSystemEconomy(UniverseData, SystemId);

	// Mark as active
	for (FLocationData& Location : NewSystem.Locations)
	{
		Location.Market.bIsActiveSimulation = true;
	}

	// Set up real-time tick timer
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWorld* World = GameInstance->GetWorld())
		{
			World->GetTimerManager().ClearTimer(ActiveSystemTickTimer);
			World->GetTimerManager().SetTimer(
				ActiveSystemTickTimer,
				FTimerDelegate::CreateLambda([this, &UniverseData]()
				{
					TickActiveSystemEconomy(UniverseData, UniverseData.Config.EconomyTickRate);
				}),
				UniverseData.Config.EconomyTickRate,
				true // Loop
			);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[EconomySubsystem] Active system set to: %s (%d)"),
		*NewSystem.SystemName, SystemId);
}

void UEconomySubsystem::TickActiveSystemEconomy(FUniverseData& UniverseData, float DeltaTime)
{
	if (UniverseData.ActiveSystemId < 0 || !UniverseData.Systems.IsValidIndex(UniverseData.ActiveSystemId))
	{
		return;
	}

	// Advance game time
	float DeltaHours = DeltaTime / 3600.0f; // Convert seconds to hours
	UniverseData.CurrentGameTime += DeltaTime;

	// Simulate active system
	FStarSystemData& ActiveSystem = UniverseData.Systems[UniverseData.ActiveSystemId];

	for (FLocationData& Location : ActiveSystem.Locations)
	{
		if (Location.Market.Goods.Num() > 0)
		{
			SimulateLocationEconomy(Location, DeltaHours, GoodsCatalog);
			Location.Market.LastUpdateTime = UniverseData.CurrentGameTime;
		}
	}
}

void UEconomySubsystem::CatchUpSystemEconomy(FUniverseData& UniverseData, int32 SystemId)
{
	if (!UniverseData.Systems.IsValidIndex(SystemId))
	{
		return;
	}

	FStarSystemData& System = UniverseData.Systems[SystemId];

	// Find earliest last update time in system
	double EarliestUpdate = UniverseData.CurrentGameTime;
	for (const FLocationData& Location : System.Locations)
	{
		if (Location.Market.Goods.Num() > 0)
		{
			EarliestUpdate = FMath::Min(EarliestUpdate, Location.Market.LastUpdateTime);
		}
	}

	// Calculate time delta
	double TimeDelta = UniverseData.CurrentGameTime - EarliestUpdate;
	if (TimeDelta <= 0.0)
	{
		return; // Already up to date
	}

	// Simulate in chunks to avoid huge numbers
	float ChunkHours = UniverseData.Config.BackgroundSimulationChunk;
	float TotalHours = TimeDelta / 3600.0f;
	int32 NumChunks = FMath::CeilToInt(TotalHours / ChunkHours);

	for (int32 ChunkIdx = 0; ChunkIdx < NumChunks; ++ChunkIdx)
	{
		float DeltaHours = FMath::Min(ChunkHours, TotalHours - (ChunkIdx * ChunkHours));

		for (FLocationData& Location : System.Locations)
		{
			if (Location.Market.Goods.Num() > 0)
			{
				SimulateLocationEconomy(Location, DeltaHours, GoodsCatalog);
			}
		}
	}

	// Update last simulation time
	for (FLocationData& Location : System.Locations)
	{
		Location.Market.LastUpdateTime = UniverseData.CurrentGameTime;
	}

	UE_LOG(LogTemp, Verbose, TEXT("[EconomySubsystem] Caught up system %s: %.1f hours in %d chunks"),
		*System.SystemName, TotalHours, NumChunks);
}

void UEconomySubsystem::SimulateLocationEconomy(FLocationData& Location, float DeltaHours, const TMap<EGoodType, FGoodDefinition>& Catalog)
{
	// Step 1: Apply production and consumption
	for (FMarketGoodEntry& Good : Location.Market.Goods)
	{
		// Production adds to stock
		if (Good.ProductionRate > 0)
		{
			float Production = Good.ProductionRate * DeltaHours;
			Good.Stock += FMath::RoundToInt(Production);
		}

		// Consumption removes from stock (but never negative)
		if (Good.ConsumptionRate > 0)
		{
			float Consumption = Good.ConsumptionRate * DeltaHours;
			int32 ConsumedAmount = FMath::Min(Good.Stock, FMath::RoundToInt(Consumption));
			Good.Stock -= ConsumedAmount;
		}

		// Clamp stock to reasonable bounds
		Good.Stock = FMath::Clamp(Good.Stock, 0, Good.TargetStock * 10);
	}

	// Step 2: Update shortages and surpluses
	UpdateShortagesAndSurpluses(Location.Market);

	// Step 3: Update prices
	UpdateMarketPrices(Location.Market, Catalog);
}

void UEconomySubsystem::UpdateShortagesAndSurpluses(FMarketState& Market)
{
	for (FMarketGoodEntry& Good : Market.Goods)
	{
		// Shortage threshold: 25% of target stock
		int32 ShortageThreshold = FMath::Max(1, Good.TargetStock / 4);
		Good.bIsShortage = (Good.Stock < ShortageThreshold);

		// Surplus threshold: 150% of target stock
		int32 SurplusThreshold = Good.TargetStock + (Good.TargetStock / 2);
		Good.bIsSurplus = (Good.Stock > SurplusThreshold);
	}
}

void UEconomySubsystem::UpdateMarketPrices(FMarketState& Market, const TMap<EGoodType, FGoodDefinition>& Catalog)
{
	for (FMarketGoodEntry& Good : Market.Goods)
	{
		const FGoodDefinition* GoodDef = Catalog.Find(Good.GoodType);
		if (!GoodDef)
		{
			continue;
		}

		float BasePrice = GoodDef->BasePrice;
		float TargetPrice = BasePrice;

		// Calculate supply ratio
		float SupplyRatio = (Good.TargetStock > 0) ? (float)Good.Stock / (float)Good.TargetStock : 1.0f;

		// Price formula: inversely proportional to supply
		// Low supply (shortage) = high price
		// High supply (surplus) = low price
		if (SupplyRatio < 0.25f)
		{
			// Severe shortage: 2x to 4x base price
			TargetPrice = BasePrice * FMath::Lerp(4.0f, 2.0f, SupplyRatio / 0.25f);
		}
		else if (SupplyRatio < 0.75f)
		{
			// Moderate shortage: 1x to 2x base price
			TargetPrice = BasePrice * FMath::Lerp(2.0f, 1.0f, (SupplyRatio - 0.25f) / 0.5f);
		}
		else if (SupplyRatio < 1.25f)
		{
			// Normal range: 0.8x to 1x base price
			TargetPrice = BasePrice * FMath::Lerp(1.0f, 0.8f, (SupplyRatio - 0.75f) / 0.5f);
		}
		else
		{
			// Surplus: 0.8x to 0.4x base price
			float ExcessRatio = FMath::Clamp(SupplyRatio - 1.25f, 0.0f, 1.25f);
			TargetPrice = BasePrice * FMath::Lerp(0.8f, 0.4f, ExcessRatio / 1.25f);
		}

		// Smooth price changes (don't snap immediately)
		float PriceDelta = TargetPrice - Good.CurrentPrice;
		Good.CurrentPrice += PriceDelta * 0.3f; // 30% adjustment per tick

		// Clamp to reasonable bounds
		Good.CurrentPrice = FMath::Clamp(Good.CurrentPrice, BasePrice * 0.2f, BasePrice * 5.0f);
	}
}

const FMarketState& UEconomySubsystem::GetMarketState(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId) const
{
	static FMarketState EmptyMarket;

	if (!UniverseData.Systems.IsValidIndex(SystemId))
	{
		return EmptyMarket;
	}

	const FStarSystemData& System = UniverseData.Systems[SystemId];
	for (const FLocationData& Location : System.Locations)
	{
		if (Location.LocationId == LocationId)
		{
			return Location.Market;
		}
	}

	return EmptyMarket;
}

float UEconomySubsystem::GetGoodPrice(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId, EGoodType GoodType) const
{
	const FMarketState& Market = GetMarketState(UniverseData, SystemId, LocationId);

	for (const FMarketGoodEntry& Good : Market.Goods)
	{
		if (Good.GoodType == GoodType)
		{
			return Good.CurrentPrice;
		}
	}

	// Not found: return base price from catalog
	if (const FGoodDefinition* GoodDef = GoodsCatalog.Find(GoodType))
	{
		return GoodDef->BasePrice;
	}

	return 0.0f;
}

bool UEconomySubsystem::HasShortage(const FUniverseData& UniverseData, int32 SystemId, int32 LocationId, EGoodType GoodType) const
{
	const FMarketState& Market = GetMarketState(UniverseData, SystemId, LocationId);

	for (const FMarketGoodEntry& Good : Market.Goods)
	{
		if (Good.GoodType == GoodType)
		{
			return Good.bIsShortage;
		}
	}

	return false;
}

TArray<int32> UEconomySubsystem::FindShortageLocations(const FUniverseData& UniverseData, EGoodType GoodType) const
{
	TArray<int32> ShortageLocations;

	for (const FStarSystemData& System : UniverseData.Systems)
	{
		for (const FLocationData& Location : System.Locations)
		{
			for (const FMarketGoodEntry& Good : Location.Market.Goods)
			{
				if (Good.GoodType == GoodType && Good.bIsShortage)
				{
					ShortageLocations.Add(Location.LocationId);
					break;
				}
			}
		}
	}

	return ShortageLocations;
}

void UEconomySubsystem::PrintEconomyStats(const FUniverseData& UniverseData) const
{
	int32 TotalLocations = 0;
	int32 ActiveMarkets = 0;
	int32 TotalShortages = 0;
	int32 TotalSurpluses = 0;

	TMap<EGoodType, int32> ShortagesByGood;
	TMap<EGoodType, int32> SurplusByGood;

	for (const FStarSystemData& System : UniverseData.Systems)
	{
		for (const FLocationData& Location : System.Locations)
		{
			TotalLocations++;

			if (Location.Market.Goods.Num() > 0)
			{
				ActiveMarkets++;

				for (const FMarketGoodEntry& Good : Location.Market.Goods)
				{
					if (Good.bIsShortage)
					{
						TotalShortages++;
						ShortagesByGood.FindOrAdd(Good.GoodType)++;
					}

					if (Good.bIsSurplus)
					{
						TotalSurpluses++;
						SurplusByGood.FindOrAdd(Good.GoodType)++;
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("=== Economy Statistics ==="));
	UE_LOG(LogTemp, Log, TEXT("Game Time: %.1f hours"), UniverseData.CurrentGameTime / 3600.0f);
	UE_LOG(LogTemp, Log, TEXT("Active System: %d"), UniverseData.ActiveSystemId);
	UE_LOG(LogTemp, Log, TEXT("Total Locations: %d"), TotalLocations);
	UE_LOG(LogTemp, Log, TEXT("Active Markets: %d"), ActiveMarkets);
	UE_LOG(LogTemp, Log, TEXT("Total Shortages: %d"), TotalShortages);
	UE_LOG(LogTemp, Log, TEXT("Total Surpluses: %d"), TotalSurpluses);

	if (TotalShortages > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("--- Shortages by Good ---"));
		for (const auto& Pair : ShortagesByGood)
		{
			if (const FGoodDefinition* GoodDef = GoodsCatalog.Find(Pair.Key))
			{
				UE_LOG(LogTemp, Log, TEXT("  %s: %d locations"), *GoodDef->DisplayName, Pair.Value);
			}
		}
	}

	if (TotalSurpluses > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("--- Surpluses by Good ---"));
		for (const auto& Pair : SurplusByGood)
		{
			if (const FGoodDefinition* GoodDef = GoodsCatalog.Find(Pair.Key))
			{
				UE_LOG(LogTemp, Log, TEXT("  %s: %d locations"), *GoodDef->DisplayName, Pair.Value);
			}
		}
	}
}
