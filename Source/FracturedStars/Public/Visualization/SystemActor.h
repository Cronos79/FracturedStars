// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SystemActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Visual representation of a star system on the galaxy map.
 * Handles selection, hover, and click interaction for systems.
 */
UCLASS()
class FRACTUREDSTARS_API ASystemActor : public AActor
{
	GENERATED_BODY()

public:	
	ASystemActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// ===========================
	// Components
	// ===========================

	/** Visual mesh for the system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SystemMesh;

	// ===========================
	// System Data
	// ===========================

	/** Unique identifier for this system in UniverseSubsystem */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 SystemId;

	/** Display name of the system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	FString SystemName;

	/** Is this system currently selected? */
	UPROPERTY(BlueprintReadOnly, Category = "System")
	bool bIsSelected;

	/** Is the mouse currently hovering over this system? */
	UPROPERTY(BlueprintReadOnly, Category = "System")
	bool bIsHovered;

	// ===========================
	// Interaction
	// ===========================

	/** Called when this system is clicked */
	UFUNCTION(BlueprintCallable, Category = "System")
	void OnSystemClicked();

	/** Set whether this system is selected */
	UFUNCTION(BlueprintCallable, Category = "System")
	void SetSelected(bool bSelected);

	/** Set whether this system is hovered */
	UFUNCTION(BlueprintCallable, Category = "System")
	void SetHovered(bool bHovered);

	// ===========================
	// Visual Updates
	// ===========================

	/** Update visual state based on selection/hover */
	UFUNCTION(BlueprintCallable, Category = "System")
	void UpdateVisualState();

	/** Dynamic material instance for visual effects */
	UPROPERTY(BlueprintReadOnly, Category = "System")
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	// ===========================
	// Configuration
	// ===========================

	/** Color when system is normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System|Visual")
	FLinearColor NormalColor;

	/** Color when system is hovered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System|Visual")
	FLinearColor HoverColor;

	/** Color when system is selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System|Visual")
	FLinearColor SelectedColor;

	/** Scale multiplier when system is normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System|Visual")
	float NormalScale;

	/** Scale multiplier when system is hovered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System|Visual")
	float HoverScale;

	/** Scale multiplier when system is selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System|Visual")
	float SelectedScale;
};
