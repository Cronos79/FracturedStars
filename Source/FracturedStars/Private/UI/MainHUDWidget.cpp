// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "UI/MainHUDWidget.h"
#include "Universe/UniverseSubsystem.h"
#include "Universe/EconomySubsystem.h"
#include "Universe/LogisticsSubsystem.h"
#include "Universe/FactionSubsystem.h"
#include "Universe/FogOfWarSubsystem.h"
#include "Player/PlayerSubsystem.h"
#include "Player/FracturedStarsPlayerController.h"
#include "Universe/UniverseTypes.h"

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
