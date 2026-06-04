// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UniverseCameraPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;

/**
 * Universe Camera Pawn
 * 
 * DESIGN PHILOSOPHY:
 * "The camera is not the player. The camera is the view."
 * 
 * This pawn provides RTS-style camera control:
 * - Pan (WASD or edge scrolling)
 * - Zoom (mouse wheel or keys)
 * - Rotate (Q/E or middle mouse drag)
 * - Focus on targets (ships, systems, stations)
 * 
 * The camera is NOT:
 * - A humanoid character
 * - The player's physical representation
 * - A possessed actor with health/combat
 * 
 * Sprint 4 Foundation - minimal free-cam movement
 */
UCLASS()
class FRACTUREDSTARS_API AUniverseCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AUniverseCameraPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ========================================================================
	// Camera Movement
	// ========================================================================

	/**
	 * Pan camera forward/backward
	 * @param Value -1.0 to 1.0 (negative = backward, positive = forward)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PanForward(float Value);

	/**
	 * Pan camera right/left
	 * @param Value -1.0 to 1.0 (negative = left, positive = right)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PanRight(float Value);

	/**
	 * Pan camera up/down
	 * @param Value -1.0 to 1.0 (negative = down, positive = up)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PanUp(float Value);

	/**
	 * Zoom camera in/out
	 * @param Value Positive = zoom in, negative = zoom out
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void Zoom(float Value);

	/**
	 * Rotate camera around Z axis (yaw)
	 * @param Value Rotation speed
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RotateYaw(float Value);

	/**
	 * Rotate camera pitch (look up/down)
	 * @param Value Rotation speed
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RotatePitch(float Value);

	/**
	 * Focus camera on a specific location
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void FocusOnLocation(FVector TargetLocation, float TransitionTime = 1.0f);

	/**
	 * Get current camera distance from focus point
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	float GetCameraDistance() const;

	/**
	 * Set camera distance from focus point
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraDistance(float NewDistance);

	// ========================================================================
	// Camera Settings
	// ========================================================================

	/** Movement speed for panning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float PanSpeed = 2000.0f;

	/** Zoom speed (distance change per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float ZoomSpeed = 500.0f;

	/** Rotation speed (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float RotationSpeed = 90.0f;

	/** Minimum zoom distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float MinZoomDistance = 500.0f;

	/** Maximum zoom distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float MaxZoomDistance = 50000.0f;

	/** Minimum pitch angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float MinPitchAngle = -80.0f;

	/** Maximum pitch angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float MaxPitchAngle = -10.0f;

	/** Enable edge scrolling (pan when mouse near screen edge) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	bool bEnableEdgeScrolling = true;

	/** Edge scrolling activation distance (pixels from screen edge) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings")
	float EdgeScrollThreshold = 20.0f;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// Internal state
	float CurrentZoomDistance = 5000.0f;
	float CurrentPitch = -45.0f;
	float CurrentYaw = 0.0f;

	// Edge scrolling
	void HandleEdgeScrolling(float DeltaTime);
	FVector2D LastMousePosition;
};
