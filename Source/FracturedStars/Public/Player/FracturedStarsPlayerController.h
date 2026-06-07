// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FracturedStarsPlayerController.generated.h"

class UPlayerSubsystem;
class UFogOfWarSubsystem;
class UUniverseSubsystem;
class UInputMappingContext;
class UFracturedStarsInputConfig;
class UMainHUDWidget;
class ASystemActor;
struct FInputActionValue;

/**
 * Fractured Stars Player Controller
 * 
 * DESIGN PHILOSOPHY:
 * "The PlayerController is NOT the in-universe player."
 * 
 * This controller handles:
 * - Client input (mouse, keyboard, gamepad)
 * - UI commands and interactions
 * - Camera control
 * - Server command requests
 * - Client-side feedback and visualization
 * 
 * The controller does NOT:
 * - Directly own ships or crew (PlayerSubsystem does)
 * - Directly mutate game state (server validates via subsystems)
 * - Represent physical presence in the universe (ships/crew do)
 * 
 * Sprint 4 Foundation - minimal RTS-style control layer
 */
UCLASS()
class FRACTUREDSTARS_API AFracturedStarsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFracturedStarsPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================================================
	// Player Identity & State
	// ========================================================================

	/**
	 * Get the in-universe PlayerId this controller represents
	 * @return PlayerId, or -1 if not assigned yet
	 */
	UFUNCTION(BlueprintCallable, Category = "Player")
	int32 GetPlayerId() const { return PlayerId; }

	/**
	 * Set the in-universe PlayerId (SERVER ONLY)
	 * Called during player spawn/login
	 */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerId(int32 NewPlayerId);

	/**
	 * Check if this controller has a valid in-universe player
	 */
	UFUNCTION(BlueprintCallable, Category = "Player")
	bool HasPlayer() const { return PlayerId != -1; }

	// ========================================================================
	// Server Commands (Client → Server RPC)
	// ========================================================================

	/**
	 * Request to create a new player profile (Client → Server)
	 * Server validates and creates player via PlayerSubsystem
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Player|Commands")
	void ServerCreatePlayer(const FString& PlayerName, int32 StartingSystemId = -1);

	/**
	 * Request to move ship to connected system (Client → Server)
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Player|Commands")
	void ServerMoveShipToSystem(int32 ShipId, int32 TargetSystemId);

	/**
	 * Request to board a ship (Client → Server)
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Player|Commands")
	void ServerBoardShip(int32 ShipId);

	/**
	 * Request to disembark from ship (Client → Server)
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Player|Commands")
	void ServerDisembarkShip();

	/**
	 * Request to assign crew to ship (Client → Server)
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Player|Commands")
	void ServerAssignCrewToShip(int32 CrewId, int32 ShipId);

	// ========================================================================
	// Client Notifications (Server → Client RPC)
	// ========================================================================

	/**
	 * Notify client that player was successfully created (Server → Client)
	 */
	UFUNCTION(Client, Reliable, Category = "Player|Notifications")
	void ClientNotifyPlayerCreated(int32 NewPlayerId);

	/**
	 * Notify client of command result (Server → Client)
	 */
	UFUNCTION(Client, Reliable, Category = "Player|Notifications")
	void ClientNotifyCommandResult(bool bSuccess, const FString& Message);

	// ========================================================================
	// Camera & Selection (Client-Side)
	// ========================================================================

	/**
	 * Get current camera pawn (if possessed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	class AUniverseCameraPawn* GetCameraPawn() const;

	/**
	 * Set camera focus on a system (move camera to system location)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	void FocusCameraOnSystem(int32 SystemId);

	/**
	 * Set camera focus on player's current ship
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	void FocusCameraOnPlayerShip();

	/**
	 * Get the main HUD widget (creates if needed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|UI")
	UMainHUDWidget* GetMainHUD();

	/**
	 * Set the main HUD widget (for Blueprint-created widgets)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|UI")
	void SetMainHUD(UMainHUDWidget* InHUDWidget);

	/**
	 * Get currently selected system actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Selection")
	ASystemActor* GetSelectedSystemActor() const { return SelectedSystemActor; }

	/**
	 * Get currently selected system ID (-1 if none)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Selection")
	int32 GetSelectedSystemId() const { return SelectedSystemId; }

	/**
	 * Select a system by ID (updates HUD and visual state)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Selection")
	void SelectSystem(int32 SystemId, ASystemActor* SystemActor);

	/**
	 * Clear current selection
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Selection")
	void ClearSelection();

	// ========================================================================
	// Input Handlers (Client-Side)
	// ========================================================================

protected:
	/** Enhanced Input Mapping Context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	/** Input Configuration DataAsset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UFracturedStarsInputConfig> InputConfig;

	/** Input mapping priority */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 InputMappingPriority = 0;

	/** Handle mouse click for selection */
	void OnClick(const FInputActionValue& Value);

	/** Handle mouse right-click for context commands */
	void OnRightClick(const FInputActionValue& Value);

	/** Handle camera pan (WASD or arrow keys) */
	void OnCameraPan(const FInputActionValue& Value);

	/** Handle camera zoom */
	void OnCameraZoom(const FInputActionValue& Value);

	/** Handle camera rotation */
	void OnCameraRotate(const FInputActionValue& Value);

	/** Focus camera on player's current ship */
	void OnFocusOnPlayerShip(const FInputActionValue& Value);

	/** Focus camera on selected system */
	void OnFocusOnSelectedSystem(const FInputActionValue& Value);

	/** Debug: Print player info */
	void OnDebugPrintPlayerInfo(const FInputActionValue& Value);

	/** Debug: Toggle debug UI */
	void OnToggleDebugUI(const FInputActionValue& Value);

	// ========================================================================
	// Helper Functions
	// ========================================================================

	/** Get reference to PlayerSubsystem (lazy initialization) */
	UPlayerSubsystem* GetPlayerSubsystem() const;

	/** Get reference to UniverseSubsystem (lazy initialization) */
	UUniverseSubsystem* GetUniverseSubsystem() const;

	/** Get reference to FogOfWarSubsystem (lazy initialization) */
	UFogOfWarSubsystem* GetFogOfWarSubsystem() const;

private:
	/** The in-universe PlayerId this controller represents */
	UPROPERTY(Replicated)
	int32 PlayerId = -1;

	/** Cached subsystem references (lazy initialization) */
	mutable TObjectPtr<UPlayerSubsystem> CachedPlayerSubsystem;
	mutable TObjectPtr<UUniverseSubsystem> CachedUniverseSubsystem;
	mutable TObjectPtr<UFogOfWarSubsystem> CachedFogOfWarSubsystem;

	/** Main HUD widget (lazy initialization) */
	UPROPERTY()
	TObjectPtr<UMainHUDWidget> MainHUDWidget;

	/** Currently selected system actor */
	UPROPERTY()
	TObjectPtr<ASystemActor> SelectedSystemActor;

	/** Currently selected system ID (-1 if none) */
	int32 SelectedSystemId = -1;

	/** HUD widget class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainHUDWidget> MainHUDClass;

	// Camera movement parameters (editable for tuning)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Speed", meta = (AllowPrivateAccess = "true"))
	float CameraPanSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Speed", meta = (AllowPrivateAccess = "true"))
	float CameraZoomSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Speed", meta = (AllowPrivateAccess = "true"))
	float CameraRotationSpeed = 90.0f;
};
