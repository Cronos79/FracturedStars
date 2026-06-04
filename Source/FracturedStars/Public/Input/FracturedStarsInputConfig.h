// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "FracturedStarsInputConfig.generated.h"

/**
 * FracturedStars Input Configuration
 * 
 * Purpose:
 * - Central repository for all Enhanced Input Actions
 * - Used by FracturedStarsPlayerController to bind inputs
 * - Create a DataAsset in the editor based on this class
 * 
 * Usage:
 * 1. Create DataAsset: Content Browser → Right Click → Input → Input Config
 * 2. Assign InputAction assets to each field
 * 3. Reference this config in PlayerController or GameMode
 */
UCLASS(BlueprintType)
class FRACTUREDSTARS_API UFracturedStarsInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Camera Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> IA_CameraPan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> IA_CameraZoom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> IA_CameraRotate;

	// Mouse/Click
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Mouse")
	TObjectPtr<UInputAction> IA_Click;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Mouse")
	TObjectPtr<UInputAction> IA_RightClick;

	// Camera Focus
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Focus")
	TObjectPtr<UInputAction> IA_FocusOnPlayerShip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Focus")
	TObjectPtr<UInputAction> IA_FocusOnSelectedSystem;

	// Debug/Dev
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> IA_DebugPrintPlayerInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> IA_ToggleDebugUI;
};
