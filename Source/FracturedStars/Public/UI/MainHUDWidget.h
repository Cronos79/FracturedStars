// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Universe/UniverseTypes.h"
#include "MainHUDWidget.generated.h"

class UUniverseSubsystem;
class UEconomySubsystem;
class ULogisticsSubsystem;
class UFactionSubsystem;
class UFogOfWarSubsystem;
class UPlayerSubsystem;
class AFracturedStarsPlayerController;

/**
 * Main HUD Widget - C++ Base Class
 * 
 * PURPOSE:
 * Provides subsystem access and selection state management for the main game HUD.
 * Blueprint children (WBP_MainHUD) handle visual layout and styling.
 * 
 * ARCHITECTURE:
 * - C++ base: Subsystem references, data getters, selection logic
 * - Blueprint child: Layout, styling, visual design, iteration
 * 
 * Sprint 8: Foundation for galaxy map UI
 */
UCLASS()
class FRACTUREDSTARS_API UMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Initialization
	// ========================================================================

	virtual void NativeConstruct() override;

	// ========================================================================
	// Subsystem Access
	// ========================================================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subsystems")
	UUniverseSubsystem* GetUniverseSubsystem() const { return UniverseSubsystem; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subsystems")
	UEconomySubsystem* GetEconomySubsystem() const { return EconomySubsystem; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subsystems")
	ULogisticsSubsystem* GetLogisticsSubsystem() const { return LogisticsSubsystem; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subsystems")
	UFactionSubsystem* GetFactionSubsystem() const { return FactionSubsystem; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subsystems")
	UFogOfWarSubsystem* GetFogOfWarSubsystem() const { return FogOfWarSubsystem; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subsystems")
	UPlayerSubsystem* GetPlayerSubsystem() const { return PlayerSubsystem; }

	// ========================================================================
	// Helper Functions for Blueprint
	// ========================================================================

	/**
	 * Get the player controller owning this HUD
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	class AFracturedStarsPlayerController* GetOwningFracturedStarsPlayerController() const;

	/**
	 * Format a number with thousands separators (e.g., 1000000 → "1,000,000")
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI|Formatting")
	static FString FormatNumber(int32 Number);

	/**
	 * Format a float as percentage (e.g., 0.75 → "75%")
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI|Formatting")
	static FString FormatPercentage(float Value, int32 DecimalPlaces = 0);

	/**
	 * Get region type as display string
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI|Formatting")
	static FString GetRegionTypeDisplayName(ERegionType RegionType);

	/**
	 * Get month name from month number (1-12)
	 * @param Month - Month number (1 = January, 12 = December)
	 * @return Month name as string (e.g., "January", "December")
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI|Formatting")
	static FString GetMonthName(int32 Month);

	// ========================================================================
	// Selection State
	// ========================================================================

	/**
	 * Get currently selected system ID
	 * @return SystemId, or -1 if none selected
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selection")
	int32 GetSelectedSystemId() const { return SelectedSystemId; }

	/**
	 * Set selected system (called by player controller on click)
	 */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SetSelectedSystem(int32 SystemId);

	/**
	 * Clear selection
	 */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearSelection();

	/**
	 * Get currently selected ship ID
	 * @return ShipId, or -1 if none selected
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selection")
	int32 GetSelectedShipId() const { return SelectedShipId; }

	/**
	 * Set selected ship
	 */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SetSelectedShip(int32 ShipId);

	// ========================================================================
	// Blueprint Events (for context panel updates)
	// ========================================================================

	/**
	 * Called when a system is selected
	 * Blueprint should update context panel
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Selection")
	void OnSystemSelected(int32 SystemId);

	/**
	 * Called when a ship is selected
	 * Blueprint should update context panel
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Selection")
	void OnShipSelected(int32 ShipId);

	/**
	 * Called when selection is cleared
	 * Blueprint should show player overview
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Selection")
	void OnSelectionCleared();

protected:
	// Cached subsystem references
	UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
	TObjectPtr<UUniverseSubsystem> UniverseSubsystem;

	UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
	TObjectPtr<UEconomySubsystem> EconomySubsystem;

	UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
	TObjectPtr<ULogisticsSubsystem> LogisticsSubsystem;

	UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
	TObjectPtr<UFactionSubsystem> FactionSubsystem;

	UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
	TObjectPtr<UFogOfWarSubsystem> FogOfWarSubsystem;

	UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
	TObjectPtr<UPlayerSubsystem> PlayerSubsystem;

	// Selection state
	UPROPERTY(BlueprintReadOnly, Category = "Selection")
	int32 SelectedSystemId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Selection")
	int32 SelectedShipId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Selection")
	int32 SelectedCrewId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Selection")
	int32 SelectedFactionId = -1;
};
