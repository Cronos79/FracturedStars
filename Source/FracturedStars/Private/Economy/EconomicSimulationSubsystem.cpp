// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Economy/EconomicSimulationSubsystem.h"
#include "Economy/EconomicProfileLibrary.h"
#include "Universe/UniverseSubsystem.h"
#include "TimerManager.h"

void UEconomicSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("[EconomicSimulation] Subsystem initialized"));
}

void UEconomicSimulationSubsystem::Deinitialize()
{
	StopEconomicSimulation();

	Super::Deinitialize();
}

// ============================================================================
// SIMULATION CONTROL
// ============================================================================

void UEconomicSimulationSubsystem::StartEconomicSimulation()
{
	if (bIsSimulationRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EconomicSimulation] Already running"));
		return;
	}

	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe || !Universe->IsAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EconomicSimulation] Cannot start - not server authority"));
		return;
	}

	// Start timer
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			EconomicTickTimerHandle,
			this,
			&UEconomicSimulationSubsystem::OnEconomicTick,
			TickInterval,
			true // Loop
		);

		bIsSimulationRunning = true;
		UE_LOG(LogTemp, Log, TEXT("[EconomicSimulation] Started - Tick interval: %.2f seconds"), TickInterval);
	}
}

void UEconomicSimulationSubsystem::StopEconomicSimulation()
{
	if (!bIsSimulationRunning)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(EconomicTickTimerHandle);
	}

	bIsSimulationRunning = false;
	UE_LOG(LogTemp, Log, TEXT("[EconomicSimulation] Stopped"));
}

void UEconomicSimulationSubsystem::SetTickInterval(float NewInterval)
{
	if (NewInterval <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EconomicSimulation] Invalid tick interval: %.2f"), NewInterval);
		return;
	}

	TickInterval = NewInterval;

	// Restart timer if running
	if (bIsSimulationRunning)
	{
		StopEconomicSimulation();
		StartEconomicSimulation();
	}

	UE_LOG(LogTemp, Log, TEXT("[EconomicSimulation] Tick interval set to %.2f seconds"), TickInterval);
}

// ============================================================================
// SYSTEM ACTIVITY MANAGEMENT (TWO-TIER)
// ============================================================================

void UEconomicSimulationSubsystem::ActivateSystem(int32 SystemId)
{
	FSystemEconomicState& State = SystemStates.FindOrAdd(SystemId);

	if (!State.bIsActive)
	{
		// Catch up this system before activating
		UUniverseSubsystem* Universe = GetUniverseSubsystem();
		if (Universe)
		{
			double CurrentTime = Universe->GetGameTime().TotalElapsedSeconds;
			double TimeSinceLastTick = CurrentTime - State.LastTickSeconds;
			int32 TicksOwed = FMath::FloorToInt(TimeSinceLastTick / TickInterval);

			if (TicksOwed > 0)
			{
				// Catch up (clamped for safety)
				int32 TicksToProcess = FMath::Min(TicksOwed, MaxCatchUpTicks);
				TickSystemEconomy(SystemId, TicksToProcess);

				UE_LOG(LogTemp, Log, TEXT("[EconomicSimulation] System %d activated - caught up %d ticks"), 
					SystemId, TicksToProcess);
			}
		}

		State.bIsActive = true;
		State.TicksOwed = 0;
	}
}

void UEconomicSimulationSubsystem::DeactivateSystem(int32 SystemId)
{
	FSystemEconomicState* State = SystemStates.Find(SystemId);
	if (State && State->bIsActive)
	{
		State->bIsActive = false;
		UE_LOG(LogTemp, Log, TEXT("[EconomicSimulation] System %d deactivated"), SystemId);
	}
}

bool UEconomicSimulationSubsystem::IsSystemActive(int32 SystemId) const
{
	const FSystemEconomicState* State = SystemStates.Find(SystemId);
	return State && State->bIsActive;
}

int32 UEconomicSimulationSubsystem::GetActiveSystemCount() const
{
	int32 Count = 0;
	for (const auto& Pair : SystemStates)
	{
		if (Pair.Value.bIsActive)
		{
			Count++;
		}
	}
	return Count;
}

// ============================================================================
// SIMULATION LOOP
// ============================================================================

void UEconomicSimulationSubsystem::OnEconomicTick()
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	// Tick all active systems (real-time)
	TickActiveSystems();

	// Batch catch-up for inactive systems
	CatchUpInactiveSystems();
}

void UEconomicSimulationSubsystem::TickActiveSystems()
{
	for (auto& Pair : SystemStates)
	{
		if (Pair.Value.bIsActive)
		{
			TickSystemEconomy(Pair.Key, 1);
		}
	}
}

void UEconomicSimulationSubsystem::CatchUpInactiveSystems()
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	double CurrentTime = Universe->GetGameTime().TotalElapsedSeconds;
	int32 SystemsProcessed = 0;

	for (auto& Pair : SystemStates)
	{
		if (!Pair.Value.bIsActive)
		{
			double TimeSinceLastTick = CurrentTime - Pair.Value.LastTickSeconds;
			int32 TicksOwed = FMath::FloorToInt(TimeSinceLastTick / TickInterval);

			if (TicksOwed > 0)
			{
				// Process a portion of owed ticks (spread across multiple frames)
				int32 TicksToProcess = FMath::Min(TicksOwed, 1); // Process 1 tick per batch frame
				TickSystemEconomy(Pair.Key, TicksToProcess);

				Pair.Value.TicksOwed = TicksOwed - TicksToProcess;

				SystemsProcessed++;
				if (SystemsProcessed >= InactiveBatchSize)
				{
					break; // Spread load across frames
				}
			}
		}
	}
}

void UEconomicSimulationSubsystem::TickSystemEconomy(int32 SystemId, int32 TickCount)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	// Get system's locations via UniverseSubsystem API
	TArray<FLocationData> Locations = Universe->GetLocationsInSystem(SystemId);

	// Tick each location in the system
	for (const FLocationData& Location : Locations)
	{
		TickLocationEconomy(SystemId, Location.LocationId, TickCount);
	}

	// Update last tick time
	FSystemEconomicState& State = SystemStates.FindOrAdd(SystemId);
	State.LastTickSeconds = Universe->GetGameTime().TotalElapsedSeconds;
}

void UEconomicSimulationSubsystem::TickLocationEconomy(int32 SystemId, int32 LocationId, int32 TickCount)
{
	// For now, process ticks one at a time
	// Future optimization: batch multiple ticks for inactive systems
	for (int32 i = 0; i < TickCount; ++i)
	{
		// 1. Update production efficiency
		UpdateProductionEfficiency(SystemId, LocationId);

		// 2. Produce goods
		TickProduction(SystemId, LocationId);

		// 3. Consume goods
		TickConsumption(SystemId, LocationId);

		// 4. Update prices
		UpdateMarketPrices(SystemId, LocationId);

		// 5. Calculate economic stress
		CalculateEconomicStress(SystemId, LocationId);
	}
}

// ============================================================================
// PRODUCTION & CONSUMPTION
// ============================================================================

void UEconomicSimulationSubsystem::TickProduction(int32 SystemId, int32 LocationId)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	// Get location data
	TArray<FLocationData> Locations = Universe->GetLocationsInSystem(SystemId);
	const FLocationData* Location = Locations.FindByPredicate([LocationId](const FLocationData& Loc) {
		return Loc.LocationId == LocationId;
	});

	if (!Location)
	{
		return;
	}

	// Get/create economic profile for this location
	FLocationEconomicProfile Profile = UEconomicProfileLibrary::CreateProfileForLocationType(
		Location->LocationType, 
		Location->Population
	);

	// Process each production recipe
	for (FProductionRecipe& Recipe : Profile.ProductionRecipes)
	{
		// Check if we have sufficient inputs
		if (HasSufficientInputs(SystemId, LocationId, Recipe, 1))
		{
			// Consume inputs
			ConsumeProductionInputs(SystemId, LocationId, Recipe, 1);

			// Produce output (scaled by efficiency)
			int32 ProducedAmount = FMath::FloorToInt(Recipe.OutputQuantity * Recipe.CurrentEfficiency);
			if (ProducedAmount > 0)
			{
				AddToInventory(SystemId, LocationId, Recipe.OutputGood, ProducedAmount);
			}
		}
		// If inputs not available, efficiency was already set to 0 by UpdateProductionEfficiency
	}
}

void UEconomicSimulationSubsystem::TickConsumption(int32 SystemId, int32 LocationId)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	// Get location data
	TArray<FLocationData> Locations = Universe->GetLocationsInSystem(SystemId);
	const FLocationData* Location = Locations.FindByPredicate([LocationId](const FLocationData& Loc) {
		return Loc.LocationId == LocationId;
	});

	if (!Location)
	{
		return;
	}

	// Get economic profile
	FLocationEconomicProfile Profile = UEconomicProfileLibrary::CreateProfileForLocationType(
		Location->LocationType, 
		Location->Population
	);

	// Process each consumption entry
	for (const FConsumptionEntry& Entry : Profile.ConsumptionProfile)
	{
		// Calculate total consumption (base + population scaling)
		int32 TotalConsumption = Entry.BaseConsumption + FMath::FloorToInt(Location->Population * Entry.PopulationScale);

		// Try to consume from inventory
		int32 Consumed = RemoveFromInventory(SystemId, LocationId, Entry.GoodType, TotalConsumption);

		// If critical and couldn't consume full amount, contributes to stress
		if (Entry.bIsCritical && Consumed < TotalConsumption)
		{
			// Shortage will be detected in UpdateMarketPrices and CalculateEconomicStress
		}
	}
}

void UEconomicSimulationSubsystem::UpdateProductionEfficiency(int32 SystemId, int32 LocationId)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	// Get location data
	TArray<FLocationData> Locations = Universe->GetLocationsInSystem(SystemId);
	const FLocationData* Location = Locations.FindByPredicate([LocationId](const FLocationData& Loc) {
		return Loc.LocationId == LocationId;
	});

	if (!Location)
	{
		return;
	}

	// Get economic profile
	FLocationEconomicProfile Profile = UEconomicProfileLibrary::CreateProfileForLocationType(
		Location->LocationType, 
		Location->Population
	);

	// Calculate efficiency for each production recipe
	for (FProductionRecipe& Recipe : Profile.ProductionRecipes)
	{
		if (Recipe.Inputs.Num() == 0)
		{
			// No inputs required - always at full efficiency
			Recipe.CurrentEfficiency = 1.0f;
		}
		else
		{
			// Calculate efficiency based on input availability
			float TotalEfficiency = 1.0f;
			for (const FProductionInput& Input : Recipe.Inputs)
			{
				int32 Available = GetStock(SystemId, LocationId, Input.GoodType);
				int32 Required = Input.UnitsRequired;

				if (Available >= Required)
				{
					// Full availability for this input
					// Efficiency remains 1.0 for this input
				}
				else if (Available > 0)
				{
					// Partial availability - reduce efficiency proportionally
					float InputEfficiency = (float)Available / (float)Required;
					TotalEfficiency *= InputEfficiency;
				}
				else
				{
					// No availability - production stops
					TotalEfficiency = 0.0f;
					break;
				}
			}

			Recipe.CurrentEfficiency = TotalEfficiency;
		}
	}
}

float UEconomicSimulationSubsystem::CalculateEconomicStress(int32 SystemId, int32 LocationId)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return 0.0f;
	}

	// Get location data
	TArray<FLocationData> Locations = Universe->GetLocationsInSystem(SystemId);
	const FLocationData* Location = Locations.FindByPredicate([LocationId](const FLocationData& Loc) {
		return Loc.LocationId == LocationId;
	});

	if (!Location)
	{
		return 0.0f;
	}

	// Get economic profile
	FLocationEconomicProfile Profile = UEconomicProfileLibrary::CreateProfileForLocationType(
		Location->LocationType, 
		Location->Population
	);

	float TotalStress = 0.0f;
	int32 CriticalShortages = 0;

	// Check critical consumption needs
	for (const FConsumptionEntry& Entry : Profile.ConsumptionProfile)
	{
		if (Entry.bIsCritical)
		{
			int32 Stock = GetStock(SystemId, LocationId, Entry.GoodType);
			int32 RequiredConsumption = Entry.BaseConsumption + FMath::FloorToInt(Location->Population * Entry.PopulationScale);

			// Stress increases when stock is below 2x consumption rate (running low)
			int32 SafetyStock = RequiredConsumption * 2;

			if (Stock < RequiredConsumption)
			{
				// Critical shortage - cannot meet current consumption
				TotalStress += 0.3f;
				CriticalShortages++;
			}
			else if (Stock < SafetyStock)
			{
				// Running low - will run out soon
				float ShortageRatio = (float)(SafetyStock - Stock) / (float)SafetyStock;
				TotalStress += (0.1f * ShortageRatio);
			}
		}
	}

	// Cap stress at 1.0
	return FMath::Clamp(TotalStress, 0.0f, 1.0f);
}

void UEconomicSimulationSubsystem::UpdateMarketPrices(int32 SystemId, int32 LocationId)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	FMarketState Market = Universe->GetMarketState(SystemId, LocationId);

	// For each good in the market
	for (FMarketGoodEntry& Good : Market.Goods)
	{
		// Calculate target stock (could be enhanced with demand prediction)
		int32 TargetStock = Good.TargetStock;
		if (TargetStock == 0)
		{
			TargetStock = 100; // Default target
		}

		// Update shortage/surplus flags
		Good.bIsShortage = (Good.Stock < TargetStock * 0.3f); // Below 30% = shortage
		Good.bIsSurplus = (Good.Stock > TargetStock * 1.5f); // Above 150% = surplus

		// Adjust price based on stock level
		float StockRatio = (float)Good.Stock / (float)TargetStock;

		if (Good.bIsShortage)
		{
			// Shortage - increase price (up to 3x base)
			float PriceMultiplier = FMath::Lerp(3.0f, 1.0f, StockRatio / 0.3f);
			Good.CurrentPrice = Good.CurrentPrice * 1.05f; // Gradual increase
			Good.CurrentPrice = FMath::Clamp(Good.CurrentPrice, 1.0f, 1000.0f);
		}
		else if (Good.bIsSurplus)
		{
			// Surplus - decrease price (down to 0.5x base)
			float PriceMultiplier = FMath::Lerp(1.0f, 0.5f, (StockRatio - 1.5f) / 1.5f);
			Good.CurrentPrice = Good.CurrentPrice * 0.95f; // Gradual decrease
			Good.CurrentPrice = FMath::Max(Good.CurrentPrice, 1.0f);
		}
		else
		{
			// Normal range - gentle price adjustment toward base
			// TODO: Could add more sophisticated price equilibrium logic
		}

		// Persist the updated price back to universe
		Universe->UpdateMarketGood(SystemId, LocationId, Good.GoodType, Good.Stock, Good.CurrentPrice);
	}
}

// ============================================================================
// DEBUG & REPORTS
// ============================================================================

FEconomicSimulationReport UEconomicSimulationSubsystem::GenerateReport()
{
	FEconomicSimulationReport Report;

	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (Universe)
	{
		Report.ReportTime = Universe->GetGameTime().TotalElapsedSeconds;
	}

	Report.ActiveSystemCount = GetActiveSystemCount();

	// TODO: Gather shortages/surpluses
	// TODO: Calculate production/consumption totals

	return Report;
}

void UEconomicSimulationSubsystem::PrintEconomicSummary()
{
	FEconomicSimulationReport Report = GenerateReport();

	UE_LOG(LogTemp, Log, TEXT("=== ECONOMIC SIMULATION SUMMARY ==="));
	UE_LOG(LogTemp, Log, TEXT("Report Time: %.2f"), Report.ReportTime);
	UE_LOG(LogTemp, Log, TEXT("Active Systems: %d"), Report.ActiveSystemCount);
	UE_LOG(LogTemp, Log, TEXT("Total Shortages: %d"), Report.TotalShortages);
	UE_LOG(LogTemp, Log, TEXT("Total Surpluses: %d"), Report.TotalSurpluses);
	UE_LOG(LogTemp, Log, TEXT("==================================="));
}

void UEconomicSimulationSubsystem::PrintLocationEconomy(int32 SystemId, int32 LocationId)
{
	// TODO: Print detailed location economy state
	UE_LOG(LogTemp, Log, TEXT("[EconomicSimulation] Location %d in System %d economy state (NOT IMPLEMENTED)"), 
		LocationId, SystemId);
}

TArray<FEconomicShortage> UEconomicSimulationSubsystem::GetAllShortages()
{
	TArray<FEconomicShortage> Shortages;
	// TODO: Collect all shortages across universe
	return Shortages;
}

TArray<FEconomicSurplus> UEconomicSimulationSubsystem::GetAllSurpluses()
{
	TArray<FEconomicSurplus> Surpluses;
	// TODO: Collect all surpluses across universe
	return Surpluses;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

UUniverseSubsystem* UEconomicSimulationSubsystem::GetUniverseSubsystem() const
{
	if (!UniverseSubsystem)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			UEconomicSimulationSubsystem* MutableThis = const_cast<UEconomicSimulationSubsystem*>(this);
			MutableThis->UniverseSubsystem = GameInstance->GetSubsystem<UUniverseSubsystem>();
		}
	}

	return UniverseSubsystem;
}

bool UEconomicSimulationSubsystem::HasSufficientInputs(int32 SystemId, int32 LocationId, const FProductionRecipe& Recipe, int32 Multiplier)
{
	for (const FProductionInput& Input : Recipe.Inputs)
	{
		int32 Required = Input.UnitsRequired * Multiplier;
		int32 Available = GetStock(SystemId, LocationId, Input.GoodType);

		if (Available < Required)
		{
			return false;
		}
	}

	return true;
}

void UEconomicSimulationSubsystem::ConsumeProductionInputs(int32 SystemId, int32 LocationId, const FProductionRecipe& Recipe, int32 Multiplier)
{
	for (const FProductionInput& Input : Recipe.Inputs)
	{
		int32 Required = Input.UnitsRequired * Multiplier;
		RemoveFromInventory(SystemId, LocationId, Input.GoodType, Required);
	}
}

void UEconomicSimulationSubsystem::AddToInventory(int32 SystemId, int32 LocationId, EGoodType GoodType, int32 Quantity)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	// Use the new UpdateMarketGood API to add to stock
	FMarketState Market = Universe->GetMarketState(SystemId, LocationId);

	FMarketGoodEntry* Entry = Market.Goods.FindByPredicate([GoodType](const FMarketGoodEntry& Good) {
		return Good.GoodType == GoodType;
	});

	int32 NewStock = (Entry ? Entry->Stock : 0) + Quantity;
	float CurrentPrice = Entry ? Entry->CurrentPrice : 10.0f;

	Universe->UpdateMarketGood(SystemId, LocationId, GoodType, NewStock, CurrentPrice);
}

int32 UEconomicSimulationSubsystem::RemoveFromInventory(int32 SystemId, int32 LocationId, EGoodType GoodType, int32 Quantity)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return 0;
	}

	FMarketState Market = Universe->GetMarketState(SystemId, LocationId);

	FMarketGoodEntry* Entry = Market.Goods.FindByPredicate([GoodType](const FMarketGoodEntry& Good) {
		return Good.GoodType == GoodType;
	});

	if (!Entry)
	{
		return 0; // No stock
	}

	int32 ActualRemoved = FMath::Min(Entry->Stock, Quantity);
	int32 NewStock = Entry->Stock - ActualRemoved;

	Universe->UpdateMarketGood(SystemId, LocationId, GoodType, NewStock, Entry->CurrentPrice);

	return ActualRemoved;
}

int32 UEconomicSimulationSubsystem::GetStock(int32 SystemId, int32 LocationId, EGoodType GoodType)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return 0;
	}

	FMarketState Market = Universe->GetMarketState(SystemId, LocationId);

	const FMarketGoodEntry* Entry = Market.Goods.FindByPredicate([GoodType](const FMarketGoodEntry& Good) {
		return Good.GoodType == GoodType;
	});

	return Entry ? Entry->Stock : 0;
}

void UEconomicSimulationSubsystem::EnsureMarketEntry(int32 SystemId, int32 LocationId, EGoodType GoodType)
{
	UUniverseSubsystem* Universe = GetUniverseSubsystem();
	if (!Universe)
	{
		return;
	}

	FMarketState Market = Universe->GetMarketState(SystemId, LocationId);

	bool bExists = Market.Goods.ContainsByPredicate([GoodType](const FMarketGoodEntry& Good) {
		return Good.GoodType == GoodType;
	});

	if (!bExists)
	{
		// Create entry with stock=0 and default price
		Universe->UpdateMarketGood(SystemId, LocationId, GoodType, 0, 10.0f);
	}
}
