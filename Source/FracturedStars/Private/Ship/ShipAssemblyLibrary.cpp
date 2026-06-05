// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Ship/ShipAssemblyLibrary.h"
#include "Misc/DateTime.h"

// ============================================================================
// STAT CALCULATION
// ============================================================================

FShipCalculatedStats UShipAssemblyLibrary::CalculateShipStats(
	const FShipFrameDefinition& FrameDef,
	const TArray<FInstalledComponent>& InstalledComponents,
	const TMap<FName, FShipComponentDefinition>& ComponentDefinitions)
{
	FShipCalculatedStats Stats;

	// Start with frame base values
	Stats.TotalMass = FrameDef.BaseMass;
	Stats.CargoCapacity = FrameDef.BaseCargoCapacity;
	Stats.FuelCapacity = FrameDef.BaseFuelCapacity;
	Stats.CrewCapacity = FrameDef.BaseCrewCapacity;

	// Aggregate component contributions
	for (const FInstalledComponent& Installed : InstalledComponents)
	{
		const FShipComponentDefinition* ComponentDef = ComponentDefinitions.Find(Installed.ComponentDefinitionId);
		if (!ComponentDef)
		{
			continue; // Component definition not found
		}

		// Calculate efficiency based on condition
		float Efficiency = CalculateComponentEfficiency(Installed.CurrentCondition, ComponentDef->MaxCondition);

		// Add component mass
		Stats.TotalMass += ComponentDef->Mass;

		// Power generation (power plants)
		if (ComponentDef->PowerGeneration > 0)
		{
			Stats.PowerGeneration += FMath::RoundToInt(ComponentDef->PowerGeneration * Efficiency);
			Stats.bHasPowerPlant = true;
		}

		// Power consumption (all other systems)
		if (ComponentDef->PowerConsumption > 0)
		{
			Stats.PowerConsumption += ComponentDef->PowerConsumption;
		}

		// Engine stats
		if (ComponentDef->ComponentType == EShipComponentType::Engine)
		{
			Stats.SpeedModifier += ComponentDef->SpeedModifier * Efficiency;
			Stats.Acceleration += ComponentDef->SpeedModifier * Efficiency; // Simplified: acceleration = speed for now
			Stats.bHasEngine = true;
		}

		// Shield stats
		if (ComponentDef->ComponentType == EShipComponentType::Shield)
		{
			Stats.ShieldCapacity += FMath::RoundToInt(ComponentDef->ShieldCapacity * Efficiency);
			Stats.ShieldRechargeRate += FMath::RoundToInt(ComponentDef->ShieldRechargeRate * Efficiency);
		}

		// Weapon stats
		if (ComponentDef->ComponentType == EShipComponentType::Laser ||
			ComponentDef->ComponentType == EShipComponentType::Cannon ||
			ComponentDef->ComponentType == EShipComponentType::MissileLauncher)
		{
			Stats.WeaponDamage += FMath::RoundToInt(ComponentDef->WeaponDamage * Efficiency);
		}

		// Cargo expansion
		if (ComponentDef->CargoBonus > 0)
		{
			Stats.CargoCapacity += ComponentDef->CargoBonus;
		}

		// Fuel expansion
		if (ComponentDef->FuelBonus > 0)
		{
			Stats.FuelCapacity += ComponentDef->FuelBonus;
		}

		// Sensor range
		if (ComponentDef->SensorRange > 0)
		{
			Stats.SensorRange = FMath::Max(Stats.SensorRange, ComponentDef->SensorRange);
		}

		// Mining efficiency
		if (ComponentDef->MiningEfficiency > 0.0f)
		{
			Stats.MiningEfficiency = FMath::Max(Stats.MiningEfficiency, ComponentDef->MiningEfficiency * Efficiency);
		}
	}

	// Calculate power budget
	Stats.AvailablePower = Stats.PowerGeneration - Stats.PowerConsumption;
	Stats.bPowerBudgetValid = Stats.AvailablePower >= 0;

	// Determine operational status
	Stats.bIsOperational = Stats.bHasEngine && Stats.bHasPowerPlant && Stats.bPowerBudgetValid;

	return Stats;
}

void UShipAssemblyLibrary::UpdateShipStats(
	FShipData& ShipData,
	const FShipFrameDefinition& FrameDef,
	const TMap<FName, FShipComponentDefinition>& ComponentDefinitions)
{
	FShipCalculatedStats Stats = CalculateShipStats(FrameDef, ShipData.InstalledComponents, ComponentDefinitions);

	// Update ship data with calculated values
	ShipData.CargoCapacity = Stats.CargoCapacity;
	ShipData.FuelCapacity = Stats.FuelCapacity;
	ShipData.CrewCapacity = Stats.CrewCapacity;
	ShipData.PowerGeneration = Stats.PowerGeneration;
	ShipData.PowerConsumption = Stats.PowerConsumption;
	ShipData.SpeedModifier = Stats.SpeedModifier;
	ShipData.ShieldCapacity = Stats.ShieldCapacity;
	ShipData.WeaponDamage = Stats.WeaponDamage;
	ShipData.MiningEfficiency = Stats.MiningEfficiency;
	ShipData.SensorRange = Stats.SensorRange;
	ShipData.bIsOperational = Stats.bIsOperational;
}

// ============================================================================
// COMPONENT VALIDATION
// ============================================================================

bool UShipAssemblyLibrary::CanInstallComponent(
	const FComponentSlot& SlotDef,
	const FShipComponentDefinition& ComponentDef,
	FString& OutReason)
{
	// Check size compatibility
	if (ComponentDef.ComponentSize != SlotDef.SlotSize)
	{
		OutReason = FString::Printf(
			TEXT("Component size mismatch: Slot requires %s, component is %s"),
			*UEnum::GetValueAsString(SlotDef.SlotSize),
			*UEnum::GetValueAsString(ComponentDef.ComponentSize));
		return false;
	}

	// Check type compatibility
	if (!IsComponentTypeCompatible(SlotDef.SlotType, ComponentDef.ComponentType))
	{
		OutReason = FString::Printf(
			TEXT("Component type incompatible: Slot type is %s, component type is %s"),
			*UEnum::GetValueAsString(SlotDef.SlotType),
			*UEnum::GetValueAsString(ComponentDef.ComponentType));
		return false;
	}

	OutReason = TEXT("Component can be installed");
	return true;
}

bool UShipAssemblyLibrary::IsShipOperational(
	const FShipCalculatedStats& Stats,
	FString& OutReason)
{
	if (!Stats.bHasEngine)
	{
		OutReason = TEXT("Ship is not operational: No engine installed");
		return false;
	}

	if (!Stats.bHasPowerPlant)
	{
		OutReason = TEXT("Ship is not operational: No power plant installed");
		return false;
	}

	if (!Stats.bPowerBudgetValid)
	{
		OutReason = FString::Printf(
			TEXT("Ship is not operational: Insufficient power (Generation: %d, Consumption: %d, Deficit: %d)"),
			Stats.PowerGeneration, Stats.PowerConsumption, -Stats.AvailablePower);
		return false;
	}

	OutReason = TEXT("Ship is operational");
	return true;
}

bool UShipAssemblyLibrary::IsPowerBudgetValid(const FShipCalculatedStats& Stats)
{
	return Stats.bPowerBudgetValid;
}

// ============================================================================
// COMPONENT MANAGEMENT
// ============================================================================

bool UShipAssemblyLibrary::InstallComponent(
	FShipData& ShipData,
	FName SlotId,
	FName ComponentDefId,
	int32 ComponentCondition)
{
	// Check if slot already has a component
	for (const FInstalledComponent& Installed : ShipData.InstalledComponents)
	{
		if (Installed.SlotId == SlotId)
		{
			UE_LOG(LogTemp, Warning, TEXT("InstallComponent: Slot %s already occupied"), *SlotId.ToString());
			return false;
		}
	}

	// Create and add new installed component
	FInstalledComponent NewComponent;
	NewComponent.SlotId = SlotId;
	NewComponent.ComponentDefinitionId = ComponentDefId;
	NewComponent.CurrentCondition = FMath::Clamp(ComponentCondition, 0, 100);
	NewComponent.InstalledTime = FDateTime::UtcNow().ToUnixTimestamp();

	ShipData.InstalledComponents.Add(NewComponent);

	UE_LOG(LogTemp, Log, TEXT("InstallComponent: Installed %s into slot %s (condition: %d%%)"),
		*ComponentDefId.ToString(), *SlotId.ToString(), NewComponent.CurrentCondition);

	return true;
}

FName UShipAssemblyLibrary::RemoveComponent(
	FShipData& ShipData,
	FName SlotId)
{
	for (int32 i = 0; i < ShipData.InstalledComponents.Num(); ++i)
	{
		if (ShipData.InstalledComponents[i].SlotId == SlotId)
		{
			FName RemovedComponentId = ShipData.InstalledComponents[i].ComponentDefinitionId;
			ShipData.InstalledComponents.RemoveAt(i);

			UE_LOG(LogTemp, Log, TEXT("RemoveComponent: Removed %s from slot %s"),
				*RemovedComponentId.ToString(), *SlotId.ToString());

			return RemovedComponentId;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("RemoveComponent: Slot %s has no component installed"), *SlotId.ToString());
	return NAME_None;
}

bool UShipAssemblyLibrary::GetInstalledComponent(
	const FShipData& ShipData,
	FName SlotId,
	FInstalledComponent& OutComponent)
{
	for (const FInstalledComponent& Installed : ShipData.InstalledComponents)
	{
		if (Installed.SlotId == SlotId)
		{
			OutComponent = Installed;
			return true;
		}
	}

	return false;
}

// ============================================================================
// DEBUG / TESTING
// ============================================================================

FShipData UShipAssemblyLibrary::CreateTestShip(
	FName FrameId,
	const FString& ShipName,
	int32 OwnerPlayerId,
	int32 SystemId,
	int32 LocationId)
{
	FShipData TestShip;
	TestShip.ShipId = -1; // Will be assigned by subsystem
	TestShip.ShipName = ShipName;
	TestShip.OwnerId = OwnerPlayerId;
	TestShip.OwnerType = FName("Player");
	TestShip.CurrentSystemId = SystemId;
	TestShip.CurrentLocationId = LocationId;
	TestShip.Status = EShipStatus::Docked;
	TestShip.FrameId = FrameId;
	TestShip.HullCondition = 100.0f;

	UE_LOG(LogTemp, Log, TEXT("CreateTestShip: Created test ship '%s' with frame %s"),
		*ShipName, *FrameId.ToString());

	return TestShip;
}

void UShipAssemblyLibrary::PrintShipStats(
	const FShipData& ShipData,
	const FShipCalculatedStats& Stats)
{
	UE_LOG(LogTemp, Display, TEXT("========================================"));
	UE_LOG(LogTemp, Display, TEXT("Ship Stats: %s (ID: %d)"), *ShipData.ShipName, ShipData.ShipId);
	UE_LOG(LogTemp, Display, TEXT("========================================"));
	UE_LOG(LogTemp, Display, TEXT("Frame: %s"), *ShipData.FrameId.ToString());
	UE_LOG(LogTemp, Display, TEXT("Hull Condition: %.1f%%"), ShipData.HullCondition);
	UE_LOG(LogTemp, Display, TEXT("Operational: %s"), Stats.bIsOperational ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("--- Capacity ---"));
	UE_LOG(LogTemp, Display, TEXT("Cargo: %d / %d"), ShipData.CurrentCargoUsed, Stats.CargoCapacity);
	UE_LOG(LogTemp, Display, TEXT("Fuel: %d / %d"), ShipData.CurrentFuel, Stats.FuelCapacity);
	UE_LOG(LogTemp, Display, TEXT("Crew: %d / %d"), ShipData.AssignedCrewIds.Num(), Stats.CrewCapacity);
	UE_LOG(LogTemp, Display, TEXT("Mass: %d tons"), Stats.TotalMass);
	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("--- Power ---"));
	UE_LOG(LogTemp, Display, TEXT("Generation: %d"), Stats.PowerGeneration);
	UE_LOG(LogTemp, Display, TEXT("Consumption: %d"), Stats.PowerConsumption);
	UE_LOG(LogTemp, Display, TEXT("Available: %d"), Stats.AvailablePower);
	UE_LOG(LogTemp, Display, TEXT("Budget Valid: %s"), Stats.bPowerBudgetValid ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("--- Performance ---"));
	UE_LOG(LogTemp, Display, TEXT("Speed Modifier: %.2f"), Stats.SpeedModifier);
	UE_LOG(LogTemp, Display, TEXT("Acceleration: %.2f"), Stats.Acceleration);
	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("--- Combat ---"));
	UE_LOG(LogTemp, Display, TEXT("Shield Capacity: %d"), Stats.ShieldCapacity);
	UE_LOG(LogTemp, Display, TEXT("Shield Recharge: %d/s"), Stats.ShieldRechargeRate);
	UE_LOG(LogTemp, Display, TEXT("Weapon Damage: %d"), Stats.WeaponDamage);
	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("--- Utility ---"));
	UE_LOG(LogTemp, Display, TEXT("Sensor Range: %d"), Stats.SensorRange);
	UE_LOG(LogTemp, Display, TEXT("Mining Efficiency: %.2f"), Stats.MiningEfficiency);
	UE_LOG(LogTemp, Display, TEXT("========================================"));
}

void UShipAssemblyLibrary::PrintPowerUsage(
	const FShipCalculatedStats& Stats,
	const TArray<FInstalledComponent>& InstalledComponents,
	const TMap<FName, FShipComponentDefinition>& ComponentDefinitions)
{
	UE_LOG(LogTemp, Display, TEXT("========================================"));
	UE_LOG(LogTemp, Display, TEXT("Power Usage Breakdown"));
	UE_LOG(LogTemp, Display, TEXT("========================================"));
	UE_LOG(LogTemp, Display, TEXT("Total Generation: %d"), Stats.PowerGeneration);
	UE_LOG(LogTemp, Display, TEXT("Total Consumption: %d"), Stats.PowerConsumption);
	UE_LOG(LogTemp, Display, TEXT("Available: %d"), Stats.AvailablePower);
	UE_LOG(LogTemp, Display, TEXT(""));

	// Power generators
	UE_LOG(LogTemp, Display, TEXT("--- Power Plants ---"));
	for (const FInstalledComponent& Installed : InstalledComponents)
	{
		const FShipComponentDefinition* Def = ComponentDefinitions.Find(Installed.ComponentDefinitionId);
		if (Def && Def->PowerGeneration > 0)
		{
			float Efficiency = CalculateComponentEfficiency(Installed.CurrentCondition, Def->MaxCondition);
			int32 ActualPower = FMath::RoundToInt(Def->PowerGeneration * Efficiency);
			UE_LOG(LogTemp, Display, TEXT("%s: +%d (%.0f%% condition)"),
				*Installed.ComponentDefinitionId.ToString(), ActualPower, Efficiency * 100.0f);
		}
	}

	// Power consumers
	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("--- Power Consumers ---"));
	for (const FInstalledComponent& Installed : InstalledComponents)
	{
		const FShipComponentDefinition* Def = ComponentDefinitions.Find(Installed.ComponentDefinitionId);
		if (Def && Def->PowerConsumption > 0)
		{
			UE_LOG(LogTemp, Display, TEXT("%s: -%d"),
				*Installed.ComponentDefinitionId.ToString(), Def->PowerConsumption);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("========================================"));
}

void UShipAssemblyLibrary::PrintInstalledComponents(
	const FShipData& ShipData,
	const TMap<FName, FShipComponentDefinition>& ComponentDefinitions)
{
	UE_LOG(LogTemp, Display, TEXT("========================================"));
	UE_LOG(LogTemp, Display, TEXT("Installed Components: %s"), *ShipData.ShipName);
	UE_LOG(LogTemp, Display, TEXT("========================================"));

	if (ShipData.InstalledComponents.Num() == 0)
	{
		UE_LOG(LogTemp, Display, TEXT("No components installed"));
	}
	else
	{
		for (const FInstalledComponent& Installed : ShipData.InstalledComponents)
		{
			const FShipComponentDefinition* Def = ComponentDefinitions.Find(Installed.ComponentDefinitionId);
			if (Def)
			{
				UE_LOG(LogTemp, Display, TEXT("Slot: %s | Component: %s | Condition: %d%% | Type: %s"),
					*Installed.SlotId.ToString(),
					*Installed.ComponentDefinitionId.ToString(),
					Installed.CurrentCondition,
					*UEnum::GetValueAsString(Def->ComponentType));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Slot: %s | Component: %s | [DEFINITION NOT FOUND]"),
					*Installed.SlotId.ToString(),
					*Installed.ComponentDefinitionId.ToString());
			}
		}
	}

	UE_LOG(LogTemp, Display, TEXT("========================================"));
}

void UShipAssemblyLibrary::PrintShipEconomyData(
	const FShipFrameDefinition& FrameDef,
	const TArray<FInstalledComponent>& InstalledComponents,
	const TMap<FName, FShipComponentDefinition>& ComponentDefinitions)
{
	UE_LOG(LogTemp, Display, TEXT("========================================"));
	UE_LOG(LogTemp, Display, TEXT("Ship Economic Data"));
	UE_LOG(LogTemp, Display, TEXT("========================================"));

	int32 TotalCost = FrameDef.BaseManufacturingCost;
	UE_LOG(LogTemp, Display, TEXT("Frame: %s - %d credits"), *FrameDef.FrameName.ToString(), FrameDef.BaseManufacturingCost);

	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("--- Components ---"));
	for (const FInstalledComponent& Installed : InstalledComponents)
	{
		const FShipComponentDefinition* Def = ComponentDefinitions.Find(Installed.ComponentDefinitionId);
		if (Def)
		{
			TotalCost += Def->BaseManufacturingCost;
			UE_LOG(LogTemp, Display, TEXT("%s: %d credits"),
				*Installed.ComponentDefinitionId.ToString(), Def->BaseManufacturingCost);
		}
	}

	UE_LOG(LogTemp, Display, TEXT(""));
	UE_LOG(LogTemp, Display, TEXT("Total Ship Value: %d credits"), TotalCost);
	UE_LOG(LogTemp, Display, TEXT("========================================"));
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

EShipComponentType UShipAssemblyLibrary::GetComponentTypeFromSlotType(ESlotType SlotType)
{
	switch (SlotType)
	{
	case ESlotType::Engine:      return EShipComponentType::Engine;
	case ESlotType::PowerPlant:  return EShipComponentType::PowerPlant;
	case ESlotType::Shield:      return EShipComponentType::Shield;
	case ESlotType::Weapon:      return EShipComponentType::Laser; // Default weapon type
	case ESlotType::Utility:     return EShipComponentType::MiningLaser; // Default utility type
	default:                     return EShipComponentType::Engine;
	}
}

bool UShipAssemblyLibrary::IsComponentTypeCompatible(ESlotType SlotType, EShipComponentType ComponentType)
{
	switch (SlotType)
	{
	case ESlotType::Engine:
		return ComponentType == EShipComponentType::Engine;

	case ESlotType::PowerPlant:
		return ComponentType == EShipComponentType::PowerPlant;

	case ESlotType::Shield:
		return ComponentType == EShipComponentType::Shield;

	case ESlotType::Weapon:
		return ComponentType == EShipComponentType::Laser ||
			   ComponentType == EShipComponentType::Cannon ||
			   ComponentType == EShipComponentType::MissileLauncher;

	case ESlotType::Utility:
		// Utility slots accept ALL secondary systems - player choice!
		return ComponentType == EShipComponentType::MiningLaser ||
			   ComponentType == EShipComponentType::CargoExpansion ||
			   ComponentType == EShipComponentType::FuelTank ||
			   ComponentType == EShipComponentType::SensorArray ||
			   ComponentType == EShipComponentType::TractorBeam ||
			   ComponentType == EShipComponentType::RefineryModule ||
			   ComponentType == EShipComponentType::SalvageEquipment ||
			   ComponentType == EShipComponentType::RepairDrone;

	default:
		return false;
	}
}

float UShipAssemblyLibrary::CalculateComponentEfficiency(int32 CurrentCondition, int32 MaxCondition)
{
	if (MaxCondition <= 0)
	{
		return 0.0f;
	}

	float ConditionPercent = static_cast<float>(CurrentCondition) / static_cast<float>(MaxCondition);
	return FMath::Clamp(ConditionPercent, 0.0f, 1.0f);
}
