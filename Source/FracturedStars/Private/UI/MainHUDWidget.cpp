// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "UI/MainHUDWidget.h"
#include "Universe/UniverseSubsystem.h"
#include "Universe/EconomySubsystem.h"
#include "Universe/LogisticsSubsystem.h"
#include "Universe/LogisticsTypes.h"
#include "Universe/FactionSubsystem.h"
#include "Universe/FogOfWarSubsystem.h"
#include "Player/PlayerSubsystem.h"
#include "Player/FracturedStarsPlayerController.h"
#include "Universe/UniverseTypes.h"
#include "Ship/ShipData.h"

void UMainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Cache subsystem references
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UniverseSubsystem = GameInstance->GetSubsystem<UUniverseSubsystem>();
		EconomySubsystem = GameInstance->GetSubsystem<UEconomySubsystem>();
		LogisticsSubsystem = GameInstance->GetSubsystem<ULogisticsSubsystem>();
		FactionSubsystem = GameInstance->GetSubsystem<UFactionSubsystem>();
		FogOfWarSubsystem = GameInstance->GetSubsystem<UFogOfWarSubsystem>();
		PlayerSubsystem = GameInstance->GetSubsystem<UPlayerSubsystem>();

		UE_LOG(LogTemp, Log, TEXT("MainHUDWidget::NativeConstruct - Cached all subsystem references"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainHUDWidget::NativeConstruct - No GameInstance!"));
	}
}

void UMainHUDWidget::SetSelectedSystem(int32 SystemId)
{
	if (SelectedSystemId != SystemId)
	{
		SelectedSystemId = SystemId;
		SelectedShipId = -1;
		SelectedCrewId = -1;
		SelectedFactionId = -1;

		UE_LOG(LogTemp, Warning, TEXT("MainHUDWidget::SetSelectedSystem - System %d selected, firing Blueprint event..."), SystemId);

		// Notify Blueprint
		OnSystemSelected(SystemId);

		UE_LOG(LogTemp, Warning, TEXT("MainHUDWidget::SetSelectedSystem - Blueprint event fired"));
	}
}

void UMainHUDWidget::ClearSelection()
{
	if (SelectedSystemId != -1 || SelectedShipId != -1)
	{
		SelectedSystemId = -1;
		SelectedShipId = -1;
		SelectedCrewId = -1;
		SelectedFactionId = -1;

		UE_LOG(LogTemp, Log, TEXT("MainHUDWidget::ClearSelection - Selection cleared"));

		// Notify Blueprint
		OnSelectionCleared();
	}
}

void UMainHUDWidget::SetSelectedShip(int32 ShipId)
{
	if (SelectedShipId != ShipId)
	{
		SelectedShipId = ShipId;
		SelectedSystemId = -1;
		SelectedCrewId = -1;
		SelectedFactionId = -1;

		UE_LOG(LogTemp, Log, TEXT("MainHUDWidget::SetSelectedShip - Ship %d selected"), ShipId);

		// Notify Blueprint
		OnShipSelected(ShipId);
	}
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

AFracturedStarsPlayerController* UMainHUDWidget::GetOwningFracturedStarsPlayerController() const
{
	return Cast<AFracturedStarsPlayerController>(GetOwningPlayer());
}

FString UMainHUDWidget::FormatNumber(int32 Number)
{
	// Format with thousands separators
	FString NumberStr = FString::FromInt(FMath::Abs(Number));
	FString Result;

	int32 Count = 0;
	for (int32 i = NumberStr.Len() - 1; i >= 0; --i)
	{
		if (Count > 0 && Count % 3 == 0)
		{
			Result = TEXT(",") + Result;
		}
		Result = FString::Chr(NumberStr[i]) + Result;
		Count++;
	}

	if (Number < 0)
	{
		Result = TEXT("-") + Result;
	}

	return Result;
}

FString UMainHUDWidget::FormatPercentage(float Value, int32 DecimalPlaces)
{
	float Percentage = Value * 100.0f;

	if (DecimalPlaces == 0)
	{
		return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Percentage));
	}
	else if (DecimalPlaces == 1)
	{
		return FString::Printf(TEXT("%.1f%%"), Percentage);
	}
	else if (DecimalPlaces == 2)
	{
		return FString::Printf(TEXT("%.2f%%"), Percentage);
	}
	else
	{
		return FString::Printf(TEXT("%.0f%%"), Percentage);
	}
}

FString UMainHUDWidget::GetRegionTypeDisplayName(ERegionType RegionType)
{
	switch (RegionType)
	{
	case ERegionType::FactionCore:
		return TEXT("Faction Core");
	case ERegionType::FactionFrontier:
		return TEXT("Faction Frontier");
	case ERegionType::Neutral:
		return TEXT("Neutral");
	case ERegionType::Lawless:
		return TEXT("Lawless");
	case ERegionType::Disputed:
		return TEXT("Disputed");
	case ERegionType::Unknown:
	default:
		return TEXT("Unknown");
	}
}

FString UMainHUDWidget::GetMonthName(int32 Month)
{
	static const TArray<FString> MonthNames = {
		TEXT("January"), TEXT("February"), TEXT("March"), TEXT("April"),
		TEXT("May"), TEXT("June"), TEXT("July"), TEXT("August"),
		TEXT("September"), TEXT("October"), TEXT("November"), TEXT("December")
	};

	if (Month >= 1 && Month <= 12)
	{
		return MonthNames[Month - 1];
	}

	return TEXT("Unknown");
}

// ============================================================================
// SYSTEM DATA HELPERS
// ============================================================================

int32 UMainHUDWidget::GetSystemPlanetCount(int32 SystemId) const
{
	if (!UniverseSubsystem)
	{
		return 0;
	}

	FStarSystemData SystemData;
	if (!UniverseSubsystem->GetSystemById(SystemId, SystemData))
	{
		return 0;
	}

	int32 Count = 0;
	for (const FCelestialBodyData& Body : SystemData.CelestialBodies)
	{
		if (Body.BodyType == ECelestialBodyType::Planet || 
			Body.BodyType == ECelestialBodyType::GasGiant || 
			Body.BodyType == ECelestialBodyType::IceGiant ||
			Body.BodyType == ECelestialBodyType::DwarfPlanet)
		{
			Count++;
		}
	}

	return Count;
}

int32 UMainHUDWidget::GetSystemStationCount(int32 SystemId) const
{
	if (!UniverseSubsystem)
	{
		return 0;
	}

	FStarSystemData SystemData;
	if (!UniverseSubsystem->GetSystemById(SystemId, SystemData))
	{
		return 0;
	}

	return SystemData.Locations.Num();
}

int32 UMainHUDWidget::GetSystemAsteroidFieldCount(int32 SystemId) const
{
	if (!UniverseSubsystem)
	{
		return 0;
	}

	FStarSystemData SystemData;
	if (!UniverseSubsystem->GetSystemById(SystemId, SystemData))
	{
		return 0;
	}

	int32 Count = 0;
	for (const FCelestialBodyData& Body : SystemData.CelestialBodies)
	{
		if (Body.BodyType == ECelestialBodyType::AsteroidField)
		{
			Count++;
		}
	}

	return Count;
}

FString UMainHUDWidget::GetFactionName(int32 FactionId) const
{
	if (!FactionSubsystem || FactionId < 0)
	{
		return TEXT("Independent");
	}

	FFactionData FactionData;
	if (FactionSubsystem->GetFactionById(FactionId, FactionData))
	{
		return FactionData.FactionName;
	}

	return TEXT("Unknown");
}

FLinearColor UMainHUDWidget::GetFactionColor(int32 FactionId) const
{
	// Placeholder: Return a color based on faction type
	// TODO: Add faction color property to FFactionData in future sprint
	if (!FactionSubsystem || FactionId < 0)
	{
		return FLinearColor::White;
	}

	FFactionData FactionData;
	if (FactionSubsystem->GetFactionById(FactionId, FactionData))
	{
		// Generate color based on faction type
		switch (FactionData.FactionType)
		{
		case EFactionType::Human:
			return FLinearColor::Blue;
		case EFactionType::Alien:
			return FLinearColor::Green;
		case EFactionType::Corporate:
			return FLinearColor(1.0f, 0.8f, 0.0f); // Gold
		case EFactionType::Pirate:
			return FLinearColor::Red;
		case EFactionType::Independent:
			return FLinearColor::Gray;
		case EFactionType::Dissonance:
			return FLinearColor(0.5f, 0.0f, 0.5f); // Purple
		default:
			return FLinearColor::White;
		}
	}

	return FLinearColor::White;
}

TArray<FString> UMainHUDWidget::GetSystemShortages(int32 SystemId) const
{
	TArray<FString> Shortages;

	if (!UniverseSubsystem)
	{
		return Shortages;
	}

	FStarSystemData SystemData;
	if (!UniverseSubsystem->GetSystemById(SystemId, SystemData))
	{
		return Shortages;
	}

	// Aggregate shortages from all locations in the system
	TSet<EGoodType> ShortageSet;
	for (const FLocationData& Location : SystemData.Locations)
	{
		for (const FMarketGoodEntry& GoodEntry : Location.Market.Goods)
		{
			if (GoodEntry.bIsShortage && !ShortageSet.Contains(GoodEntry.GoodType))
			{
				ShortageSet.Add(GoodEntry.GoodType);

				// Use enum to display name conversion
				FString GoodName = UEnum::GetDisplayValueAsText(GoodEntry.GoodType).ToString();
				Shortages.Add(GoodName);
			}
		}
	}

	return Shortages;
}

TArray<FString> UMainHUDWidget::GetSystemSurpluses(int32 SystemId) const
{
	TArray<FString> Surpluses;

	if (!UniverseSubsystem)
	{
		return Surpluses;
	}

	FStarSystemData SystemData;
	if (!UniverseSubsystem->GetSystemById(SystemId, SystemData))
	{
		return Surpluses;
	}

	// Aggregate surpluses from all locations in the system
	TSet<EGoodType> SurplusSet;
	for (const FLocationData& Location : SystemData.Locations)
	{
		for (const FMarketGoodEntry& GoodEntry : Location.Market.Goods)
		{
			if (GoodEntry.bIsSurplus && !SurplusSet.Contains(GoodEntry.GoodType))
			{
				SurplusSet.Add(GoodEntry.GoodType);

				// Use enum to display name conversion
				FString GoodName = UEnum::GetDisplayValueAsText(GoodEntry.GoodType).ToString();
				Surpluses.Add(GoodName);
			}
		}
	}

	return Surpluses;
}

int32 UMainHUDWidget::GetSystemTotalPopulation(int32 SystemId) const
{
	if (!UniverseSubsystem)
	{
		return 0;
	}

	FStarSystemData SystemData;
	if (!UniverseSubsystem->GetSystemById(SystemId, SystemData))
	{
		return 0;
	}

	int32 TotalPopulation = 0;
	for (const FLocationData& Location : SystemData.Locations)
	{
		TotalPopulation += Location.Population;
	}

	return TotalPopulation;
}

int32 UMainHUDWidget::GetSystemTradeRouteCount(int32 SystemId) const
{
	if (!LogisticsSubsystem)
	{
		return 0;
	}

	// Use top trade route statistics to count routes involving this system
	TArray<FTradeRouteStatistics> TopRoutes = LogisticsSubsystem->GetTopTradeRoutes(100);
	int32 Count = 0;

	for (const FTradeRouteStatistics& RouteStat : TopRoutes)
	{
		if (RouteStat.SourceSystemId == SystemId || RouteStat.DestinationSystemId == SystemId)
		{
			Count++;
		}
	}

	return Count;
}

int32 UMainHUDWidget::GetSystemShipCount(int32 SystemId) const
{
	if (!PlayerSubsystem)
	{
		return 0;
	}

	int32 Count = 0;
	TArray<int32> AllPlayerIds = PlayerSubsystem->GetAllPlayerIds();
	for (int32 PlayerId : AllPlayerIds)
	{
		TArray<FShipData> Ships = PlayerSubsystem->GetPlayerShips(PlayerId);
		for (const FShipData& Ship : Ships)
		{
			if (Ship.CurrentSystemId == SystemId)
			{
				Count++;
			}
		}
	}

	return Count;
}

int32 UMainHUDWidget::GetSystemFactionId(int32 SystemId) const
{
	if (!UniverseSubsystem)
	{
		return -1;
	}

	FStarSystemData SystemData;
	if (UniverseSubsystem->GetSystemById(SystemId, SystemData))
	{
		return SystemData.ControllingFactionId;
	}

	return -1;
}

bool UMainHUDWidget::PlayerHasShipsInSystem(int32 PlayerId, int32 SystemId) const
{
	if (!PlayerSubsystem)
	{
		return false;
	}

	TArray<FShipData> Ships = PlayerSubsystem->GetPlayerShips(PlayerId);
	for (const FShipData& Ship : Ships)
	{
		if (Ship.CurrentSystemId == SystemId)
		{
			return true;
		}
	}

	return false;
}

TArray<FString> UMainHUDWidget::GetSystemImports(int32 SystemId) const
{
	TArray<FString> Imports;

	if (!UniverseSubsystem || !FactionSubsystem)
	{
		return Imports;
	}

	FStarSystemData SystemData;
	if (!UniverseSubsystem->GetSystemById(SystemId, SystemData) || SystemData.ControllingFactionId < 0)
	{
		return Imports;
	}

	FFactionData FactionData;
	if (!FactionSubsystem->GetFactionById(SystemData.ControllingFactionId, FactionData))
	{
		return Imports;
	}

	// Return the faction's tracked import goods (top 5 by volume)
	TArray<TPair<EGoodType, int32>> SortedImports;
	for (const TPair<EGoodType, int32>& Pair : FactionData.CurrentImports)
	{
		SortedImports.Add(Pair);
	}
	SortedImports.Sort([](const TPair<EGoodType, int32>& A, const TPair<EGoodType, int32>& B)
	{
		return A.Value > B.Value;
	});

	int32 Limit = FMath::Min(SortedImports.Num(), 5);
	for (int32 i = 0; i < Limit; ++i)
	{
		Imports.Add(UEnum::GetDisplayValueAsText(SortedImports[i].Key).ToString());
	}

	return Imports;
}

TArray<FString> UMainHUDWidget::GetSystemExports(int32 SystemId) const
{
	TArray<FString> Exports;

	if (!UniverseSubsystem || !FactionSubsystem)
	{
		return Exports;
	}

	FStarSystemData SystemData;
	if (!UniverseSubsystem->GetSystemById(SystemId, SystemData) || SystemData.ControllingFactionId < 0)
	{
		return Exports;
	}

	FFactionData FactionData;
	if (!FactionSubsystem->GetFactionById(SystemData.ControllingFactionId, FactionData))
	{
		return Exports;
	}

	// Return the faction's tracked export goods (top 5 by volume)
	TArray<TPair<EGoodType, int32>> SortedExports;
	for (const TPair<EGoodType, int32>& Pair : FactionData.CurrentExports)
	{
		SortedExports.Add(Pair);
	}
	SortedExports.Sort([](const TPair<EGoodType, int32>& A, const TPair<EGoodType, int32>& B)
	{
		return A.Value > B.Value;
	});

	int32 Limit = FMath::Min(SortedExports.Num(), 5);
	for (int32 i = 0; i < Limit; ++i)
	{
		Exports.Add(UEnum::GetDisplayValueAsText(SortedExports[i].Key).ToString());
	}

	return Exports;
}

