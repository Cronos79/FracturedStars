// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Player/FracturedStarsPlayerController.h"
#include "Player/PlayerSubsystem.h"
#include "Player/UniverseCameraPawn.h"
#include "Universe/UniverseSubsystem.h"
#include "Universe/FogOfWarSubsystem.h"
#include "Input/FracturedStarsInputConfig.h"
#include "UI/MainHUDWidget.h"
#include "Visualization/SystemActor.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"

AFracturedStarsPlayerController::AFracturedStarsPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true; // RTS-style cursor visible
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AFracturedStarsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[PlayerController] BeginPlay - PlayerId: %d"), PlayerId);

	// Setup Enhanced Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, InputMappingPriority);
			UE_LOG(LogTemp, Log, TEXT("[PlayerController] Enhanced Input Mapping Context added"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PlayerController] InputMappingContext not set in Blueprint!"));
		}
	}

	// If this is a client without a PlayerId, we may need to wait for server assignment
	// In a full implementation, this would trigger a login/character selection UI
}

void AFracturedStarsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (!InputConfig)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PlayerController] InputConfig not set! Please assign in Blueprint."));
			return;
		}

		// Bind Enhanced Input Actions
		if (InputConfig->IA_Click)
		{
			EnhancedInput->BindAction(InputConfig->IA_Click, ETriggerEvent::Started, this, &AFracturedStarsPlayerController::OnClick);
		}

		if (InputConfig->IA_RightClick)
		{
			EnhancedInput->BindAction(InputConfig->IA_RightClick, ETriggerEvent::Started, this, &AFracturedStarsPlayerController::OnRightClick);
		}

		if (InputConfig->IA_CameraPan)
		{
			EnhancedInput->BindAction(InputConfig->IA_CameraPan, ETriggerEvent::Triggered, this, &AFracturedStarsPlayerController::OnCameraPan);
		}

		if (InputConfig->IA_CameraZoom)
		{
			EnhancedInput->BindAction(InputConfig->IA_CameraZoom, ETriggerEvent::Triggered, this, &AFracturedStarsPlayerController::OnCameraZoom);
		}

		if (InputConfig->IA_CameraRotate)
		{
			EnhancedInput->BindAction(InputConfig->IA_CameraRotate, ETriggerEvent::Triggered, this, &AFracturedStarsPlayerController::OnCameraRotate);
		}

		if (InputConfig->IA_FocusOnPlayerShip)
		{
			EnhancedInput->BindAction(InputConfig->IA_FocusOnPlayerShip, ETriggerEvent::Started, this, &AFracturedStarsPlayerController::OnFocusOnPlayerShip);
		}

		if (InputConfig->IA_FocusOnSelectedSystem)
		{
			EnhancedInput->BindAction(InputConfig->IA_FocusOnSelectedSystem, ETriggerEvent::Started, this, &AFracturedStarsPlayerController::OnFocusOnSelectedSystem);
		}

		if (InputConfig->IA_DebugPrintPlayerInfo)
		{
			EnhancedInput->BindAction(InputConfig->IA_DebugPrintPlayerInfo, ETriggerEvent::Started, this, &AFracturedStarsPlayerController::OnDebugPrintPlayerInfo);
		}

		if (InputConfig->IA_ToggleDebugUI)
		{
			EnhancedInput->BindAction(InputConfig->IA_ToggleDebugUI, ETriggerEvent::Started, this, &AFracturedStarsPlayerController::OnToggleDebugUI);
		}

		UE_LOG(LogTemp, Log, TEXT("[PlayerController] Enhanced Input bindings configured"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController] Failed to cast to UEnhancedInputComponent!"));
	}
}

void AFracturedStarsPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Future: Update UI, selection highlighting, etc.
}

void AFracturedStarsPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFracturedStarsPlayerController, PlayerId);
}

// ============================================================================
// PLAYER IDENTITY
// ============================================================================

void AFracturedStarsPlayerController::SetPlayerId(int32 NewPlayerId)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerController] SetPlayerId called on CLIENT - ignoring"));
		return;
	}

	PlayerId = NewPlayerId;
	UE_LOG(LogTemp, Log, TEXT("[PlayerController] PlayerId set to %d"), PlayerId);

	// Notify client
	ClientNotifyPlayerCreated(NewPlayerId);
}

// ============================================================================
// SERVER COMMANDS
// ============================================================================

void AFracturedStarsPlayerController::ServerCreatePlayer_Implementation(const FString& PlayerName, int32 StartingSystemId)
{
	UPlayerSubsystem* PlayerSys = GetPlayerSubsystem();
	if (!PlayerSys)
	{
		ClientNotifyCommandResult(false, TEXT("PlayerSubsystem not available"));
		return;
	}

	// Create player
	int32 NewPlayerId = PlayerSys->CreatePlayer(PlayerName, StartingSystemId);
	if (NewPlayerId == -1)
	{
		ClientNotifyCommandResult(false, TEXT("Failed to create player"));
		return;
	}

	// Assign PlayerId to this controller
	SetPlayerId(NewPlayerId);

	// Create starter ship + crew
	const FPlayerProfileData PlayerProfile = PlayerSys->GetPlayerProfile(NewPlayerId);
	int32 ShipId = PlayerSys->CreateStarterShip(NewPlayerId, PlayerProfile.CurrentSystemId, PlayerProfile.CurrentLocationId);

	if (ShipId != -1)
	{
		// Create 3 starter crew
		for (int32 i = 0; i < 3; i++)
		{
			PlayerSys->CreateCrewMember(NewPlayerId, ShipId);
		}
	}

	ClientNotifyCommandResult(true, FString::Printf(TEXT("Welcome, Captain %s!"), *PlayerName));
}

void AFracturedStarsPlayerController::ServerMoveShipToSystem_Implementation(int32 ShipId, int32 TargetSystemId)
{
	UPlayerSubsystem* PlayerSys = GetPlayerSubsystem();
	if (!PlayerSys)
	{
		ClientNotifyCommandResult(false, TEXT("PlayerSubsystem not available"));
		return;
	}

	// Validate ownership
	if (!PlayerSys->PlayerOwnsShip(PlayerId, ShipId))
	{
		ClientNotifyCommandResult(false, TEXT("You do not own that ship"));
		return;
	}

	// Execute move
	bool bSuccess = PlayerSys->MoveShipToConnectedSystem(ShipId, TargetSystemId);
	if (bSuccess)
	{
		ClientNotifyCommandResult(true, FString::Printf(TEXT("Ship %d moved to system %d"), ShipId, TargetSystemId));
	}
	else
	{
		ClientNotifyCommandResult(false, TEXT("Failed to move ship (systems not connected or invalid)"));
	}
}

void AFracturedStarsPlayerController::ServerBoardShip_Implementation(int32 ShipId)
{
	UPlayerSubsystem* PlayerSys = GetPlayerSubsystem();
	if (!PlayerSys)
	{
		ClientNotifyCommandResult(false, TEXT("PlayerSubsystem not available"));
		return;
	}

	bool bSuccess = PlayerSys->BoardShip(PlayerId, ShipId);
	if (bSuccess)
	{
		ClientNotifyCommandResult(true, FString::Printf(TEXT("Boarded ship %d"), ShipId));
	}
	else
	{
		ClientNotifyCommandResult(false, TEXT("Failed to board ship"));
	}
}

void AFracturedStarsPlayerController::ServerDisembarkShip_Implementation()
{
	UPlayerSubsystem* PlayerSys = GetPlayerSubsystem();
	if (!PlayerSys)
	{
		ClientNotifyCommandResult(false, TEXT("PlayerSubsystem not available"));
		return;
	}

	bool bSuccess = PlayerSys->DisembarkShip(PlayerId);
	if (bSuccess)
	{
		ClientNotifyCommandResult(true, TEXT("Disembarked from ship"));
	}
	else
	{
		ClientNotifyCommandResult(false, TEXT("Failed to disembark"));
	}
}

void AFracturedStarsPlayerController::ServerAssignCrewToShip_Implementation(int32 CrewId, int32 ShipId)
{
	UPlayerSubsystem* PlayerSys = GetPlayerSubsystem();
	if (!PlayerSys)
	{
		ClientNotifyCommandResult(false, TEXT("PlayerSubsystem not available"));
		return;
	}

	// Validate crew ownership
	const FCrewMemberData Crew = PlayerSys->GetCrewMember(CrewId);
	if (Crew.CrewId == -1 || Crew.OwnerPlayerId != PlayerId)
	{
		ClientNotifyCommandResult(false, TEXT("Crew not found or not owned by you"));
		return;
	}

	// Validate ship ownership (if assigning, not unassigning)
	if (ShipId != -1)
	{
		if (!PlayerSys->PlayerOwnsShip(PlayerId, ShipId))
		{
			ClientNotifyCommandResult(false, TEXT("You do not own that ship"));
			return;
		}
	}

	bool bSuccess = PlayerSys->AssignCrewToShip(CrewId, ShipId);
	if (bSuccess)
	{
		if (ShipId == -1)
		{
			ClientNotifyCommandResult(true, FString::Printf(TEXT("Crew %d unassigned"), CrewId));
		}
		else
		{
			ClientNotifyCommandResult(true, FString::Printf(TEXT("Crew %d assigned to ship %d"), CrewId, ShipId));
		}
	}
	else
	{
		ClientNotifyCommandResult(false, TEXT("Failed to assign crew"));
	}
}

// ============================================================================
// CLIENT NOTIFICATIONS
// ============================================================================

void AFracturedStarsPlayerController::ClientNotifyPlayerCreated_Implementation(int32 NewPlayerId)
{
	UE_LOG(LogTemp, Log, TEXT("[PlayerController] Player created: PlayerId %d"), NewPlayerId);

	// Future: Trigger UI update, show welcome message, etc.
}

void AFracturedStarsPlayerController::ClientNotifyCommandResult_Implementation(bool bSuccess, const FString& Message)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("[PlayerController] Command SUCCESS: %s"), *Message);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Command FAILED: %s"), *Message);
	}

	// Future: Show UI notification/toast
}

// ============================================================================
// CAMERA & SELECTION
// ============================================================================

AUniverseCameraPawn* AFracturedStarsPlayerController::GetCameraPawn() const
{
	return Cast<AUniverseCameraPawn>(GetPawn());
}

void AFracturedStarsPlayerController::FocusCameraOnSystem(int32 SystemId)
{
	// Future: Move camera to system location in 3D space
	// For now, just log
	UE_LOG(LogTemp, Log, TEXT("[PlayerController] Focus camera on system %d"), SystemId);
}

void AFracturedStarsPlayerController::FocusCameraOnPlayerShip()
{
	UPlayerSubsystem* PlayerSys = GetPlayerSubsystem();
	if (!PlayerSys || !HasPlayer())
	{
		return;
	}

	const FPlayerProfileData PlayerProfile = PlayerSys->GetPlayerProfile(PlayerId);
	if (PlayerProfile.CurrentShipId != -1)
	{
		const FShipData Ship = PlayerSys->GetShip(PlayerProfile.CurrentShipId);
		FocusCameraOnSystem(Ship.CurrentSystemId);
	}
}

// ============================================================================
// INPUT HANDLERS
// ============================================================================

void AFracturedStarsPlayerController::OnClick(const FInputActionValue& Value)
{
	// Perform raycast to detect system actor clicks
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		// Check if we hit a system actor
		if (ASystemActor* SystemActor = Cast<ASystemActor>(HitResult.GetActor()))
		{
			UE_LOG(LogTemp, Log, TEXT("[PlayerController] Clicked system %d (%s)"), SystemActor->SystemId, *SystemActor->SystemName);
			SelectSystem(SystemActor->SystemId, SystemActor);
			return;
		}
	}

	// Clicked empty space - clear selection
	UE_LOG(LogTemp, Verbose, TEXT("[PlayerController] Click on empty space - clearing selection"));
	ClearSelection();
}

void AFracturedStarsPlayerController::OnRightClick(const FInputActionValue& Value)
{
	// Future: Context menu, move commands, etc.
	UE_LOG(LogTemp, Verbose, TEXT("[PlayerController] Right click"));
}

void AFracturedStarsPlayerController::OnCameraPan(const FInputActionValue& Value)
{
	AUniverseCameraPawn* Camera = GetCameraPawn();
	if (Camera)
	{
		FVector2D PanValue = Value.Get<FVector2D>();
		if (PanValue.SizeSquared() > 0.01f)
		{
			Camera->PanForward(PanValue.Y * CameraPanSpeed * GetWorld()->GetDeltaSeconds());
			Camera->PanRight(PanValue.X * CameraPanSpeed * GetWorld()->GetDeltaSeconds());
		}
	}
}

void AFracturedStarsPlayerController::OnCameraZoom(const FInputActionValue& Value)
{
	AUniverseCameraPawn* Camera = GetCameraPawn();
	if (Camera)
	{
		float ZoomValue = Value.Get<float>();
		Camera->Zoom(ZoomValue * CameraZoomSpeed * GetWorld()->GetDeltaSeconds());
	}
}

void AFracturedStarsPlayerController::OnCameraRotate(const FInputActionValue& Value)
{
	AUniverseCameraPawn* Camera = GetCameraPawn();
	if (Camera)
	{
		FVector2D RotateValue = Value.Get<FVector2D>();
		if (RotateValue.SizeSquared() > 0.01f)
		{
			Camera->RotateYaw(RotateValue.X * CameraRotationSpeed * GetWorld()->GetDeltaSeconds());
			Camera->RotatePitch(RotateValue.Y * CameraRotationSpeed * GetWorld()->GetDeltaSeconds());
		}
	}
}

void AFracturedStarsPlayerController::OnFocusOnPlayerShip(const FInputActionValue& Value)
{
	FocusCameraOnPlayerShip();
}

void AFracturedStarsPlayerController::OnFocusOnSelectedSystem(const FInputActionValue& Value)
{
	// Future: Focus on currently selected system
	UE_LOG(LogTemp, Log, TEXT("[PlayerController] Focus on selected system (not yet implemented)"));
}

void AFracturedStarsPlayerController::OnDebugPrintPlayerInfo(const FInputActionValue& Value)
{
	UPlayerSubsystem* PlayerSys = GetPlayerSubsystem();
	if (PlayerSys && HasPlayer())
	{
		PlayerSys->PrintAllPlayerState(PlayerId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerController] No player to print info for"));
	}
}

void AFracturedStarsPlayerController::OnToggleDebugUI(const FInputActionValue& Value)
{
	// Future: Toggle debug overlay UI
	UE_LOG(LogTemp, Log, TEXT("[PlayerController] Toggle debug UI (not yet implemented)"));
}

// ============================================================================
// UI & SELECTION
// ============================================================================

UMainHUDWidget* AFracturedStarsPlayerController::GetMainHUD()
{
	// Create HUD widget if needed
	if (!MainHUDWidget && MainHUDClass)
	{
		// Create widget from the Blueprint class (MainHUDClass points to WBP_MainHUD)
		MainHUDWidget = CreateWidget<UMainHUDWidget>(this, MainHUDClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("[PlayerController] MainHUD created from class %s and added to viewport"), 
				*MainHUDClass->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[PlayerController] Failed to create MainHUD widget from class %s!"), 
				MainHUDClass ? *MainHUDClass->GetName() : TEXT("NULL"));
		}
	}
	else if (!MainHUDClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController] MainHUDClass is not set! Assign WBP_MainHUD in the controller Blueprint!"));
	}

	return MainHUDWidget;
}

void AFracturedStarsPlayerController::SetMainHUD(UMainHUDWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		MainHUDWidget = InHUDWidget;
		UE_LOG(LogTemp, Log, TEXT("[PlayerController] MainHUD widget reference set from Blueprint: %s"), 
			*InHUDWidget->GetClass()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Attempted to set NULL MainHUD widget!"));
	}
}

void AFracturedStarsPlayerController::SelectSystem(int32 SystemId, ASystemActor* SystemActor)
{
	// Deselect previous system
	if (SelectedSystemActor && SelectedSystemActor != SystemActor)
	{
		SelectedSystemActor->SetSelected(false);
	}

	// Update selection
	SelectedSystemId = SystemId;
	SelectedSystemActor = SystemActor;

	// Update visual state
	if (SelectedSystemActor)
	{
		SelectedSystemActor->SetSelected(true);
	}

	// Update HUD
	UMainHUDWidget* HUD = GetMainHUD();
	if (HUD)
	{
		HUD->SetSelectedSystem(SystemId);
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerController] Selected system %d"), SystemId);
}

void AFracturedStarsPlayerController::ClearSelection()
{
	// Deselect previous system actor
	if (SelectedSystemActor)
	{
		SelectedSystemActor->SetSelected(false);
		SelectedSystemActor = nullptr;
	}

	SelectedSystemId = -1;

	// Update HUD
	UMainHUDWidget* HUD = GetMainHUD();
	if (HUD)
	{
		HUD->ClearSelection();
	}

	UE_LOG(LogTemp, Verbose, TEXT("[PlayerController] Selection cleared"));
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

UPlayerSubsystem* AFracturedStarsPlayerController::GetPlayerSubsystem() const
{
	if (!CachedPlayerSubsystem)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			AFracturedStarsPlayerController* MutableThis = const_cast<AFracturedStarsPlayerController*>(this);
			MutableThis->CachedPlayerSubsystem = GameInstance->GetSubsystem<UPlayerSubsystem>();
		}
	}

	return CachedPlayerSubsystem;
}

UUniverseSubsystem* AFracturedStarsPlayerController::GetUniverseSubsystem() const
{
	if (!CachedUniverseSubsystem)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			AFracturedStarsPlayerController* MutableThis = const_cast<AFracturedStarsPlayerController*>(this);
			MutableThis->CachedUniverseSubsystem = GameInstance->GetSubsystem<UUniverseSubsystem>();
		}
	}

	return CachedUniverseSubsystem;
}

UFogOfWarSubsystem* AFracturedStarsPlayerController::GetFogOfWarSubsystem() const
{
	if (!CachedFogOfWarSubsystem)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			AFracturedStarsPlayerController* MutableThis = const_cast<AFracturedStarsPlayerController*>(this);
			MutableThis->CachedFogOfWarSubsystem = GameInstance->GetSubsystem<UFogOfWarSubsystem>();
		}
	}

	return CachedFogOfWarSubsystem;
}
