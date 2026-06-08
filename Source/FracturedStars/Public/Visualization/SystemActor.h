// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SystemActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EGalaxyMapColorMode : uint8
{
	Default,
	Lawfulness,
	Faction,
	Region,
	FogOfWar,
	Economy
};

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

	/** Visual mesh for the system (small dot) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SystemMesh;

	/** Glow ring mesh (shown on hover) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GlowRingMesh;

	/** Selection ring mesh (shown when selected, with pulse) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SelectionRingMesh;

	/** Homeworld icon mesh (shown for faction homeworlds) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HomeworldIconMesh;

	/** Player asset icon mesh (shown when player has ships/stations here) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlayerAssetIconMesh;

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

	/** Is this system a faction homeworld? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	bool bIsHomeworld;

	/** Does the player have assets (ships/stations) in this system? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	bool bHasPlayerAssets;

	/** Faction ID that owns this system (for homeworld icon color) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 OwningFactionId;

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

	/** Set whether this is a homeworld */
	UFUNCTION(BlueprintCallable, Category = "System")
	void SetHomeworld(bool bHomeworld, int32 FactionId = -1);

	/** Set whether player has assets here */
	UFUNCTION(BlueprintCallable, Category = "System")
	void SetPlayerAssets(bool bHasAssets);

	// ===========================
	// Visual Updates
	// ===========================

	/** */
	UFUNCTION(BlueprintCallable, Category = "System")
	FLinearColor GetLawfulnessColor(float Lawfulness) const;

	UFUNCTION(BlueprintCallable, Category = "System|Visual")
	void SetBaseColor(FLinearColor NewColor);

	UFUNCTION(BlueprintCallable, Category = "System|Visual")
	void ApplyLawfulnessColor(float Lawfulness);

	UFUNCTION(BlueprintCallable, Category = "System|Visual")
	void ApplyFactionColor(int32 FactionId);

	UFUNCTION(BlueprintCallable, Category = "System|Visual")
	void SetMapColorMode(EGalaxyMapColorMode Mode);

	/** Update visual state based on selection/hover */
	UFUNCTION(BlueprintCallable, Category = "System")
	void UpdateVisualState();

	/** Dynamic material instance for visual effects */
	UPROPERTY(BlueprintReadOnly, Category = "System")
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	/** Dynamic material instance for glow ring */
	UPROPERTY(BlueprintReadOnly, Category = "System")
	TObjectPtr<UMaterialInstanceDynamic> GlowRingMaterial;

	/** Dynamic material instance for selection ring */
	UPROPERTY(BlueprintReadOnly, Category = "System")
	TObjectPtr<UMaterialInstanceDynamic> SelectionRingMaterial;

	// ===========================
	// Pulse Effect
	// ===========================

	/** Current pulse time (for animating selection ring) */
	float PulseTime;

	/** Pulse speed (cycles per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System|Visual")
	float PulseSpeed;

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
