// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Universe/UniverseTypes.h"
#include "UniverseDebugVisualizer.generated.h"

/**
 * Universe Debug Visualizer
 * Draws the universe as colored spheres and lines in the 3D viewport
 * Perfect for development and testing - no UI setup required!
 */
UCLASS()
class FRACTUREDSTARS_API AUniverseDebugVisualizer : public AActor
{
	GENERATED_BODY()

public:	
	AUniverseDebugVisualizer();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	// ====== Blueprint Properties ======

	/** Enable/disable visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug")
	bool bShowVisualization = true;

	/** Draw system spheres */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug")
	bool bShowSystems = true;

	/** Draw jump connections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug")
	bool bShowConnections = true;

	/** Draw system names */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug")
	bool bShowSystemNames = false;

	// Sprint 2: Content display modes

	/** Show location count overlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Content")
	bool bShowLocationCount = false;

	/** Show population overlay (color intensity = population) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Content")
	bool bShowPopulation = false;

	/** Show ownership overlay (faction colors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Content")
	bool bShowOwnership = false;

	/** Scale factor for visualization (adjust to fit viewport) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float VisualizationScale = 1.0f;

	/** Sphere radius for systems */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float SystemSphereRadius = 200.0f;

	/** Line thickness for connections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float ConnectionThickness = 2.0f;

	/** Height offset (Z-axis) for visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug")
	float HeightOffset = 0.0f;

	/** Draw region colors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Colors")
	bool bUseRegionColors = true;

	// Region Colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Colors")
	FColor FactionCoreColor = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Colors")
	FColor FactionFrontierColor = FColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Colors")
	FColor NeutralColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Colors")
	FColor LawlessColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Colors")
	FColor DisputedColor = FColor::Orange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe Debug|Colors")
	FColor ConnectionColor = FColor(128, 128, 128, 128); // Gray, semi-transparent

	// ====== Functions ======

	/** Manually refresh visualization */
	UFUNCTION(BlueprintCallable, Category = "Universe Debug")
	void RefreshVisualization();

private:
	void DrawUniverse();
	FColor GetRegionColor(ERegionType RegionType) const;
	FVector ScalePosition(const FVector& Position) const;
};
