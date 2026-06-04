// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Player/UniverseCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AUniverseCameraPawn::AUniverseCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root scene component
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// Create spring arm for camera distance control
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootScene);
	SpringArm->TargetArmLength = CurrentZoomDistance;
	SpringArm->bDoCollisionTest = false; // No collision for RTS camera
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 5.0f;
	SpringArm->SetRelativeRotation(FRotator(CurrentPitch, 0.0f, 0.0f));

	// Create camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void AUniverseCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	// Initialize camera position
	SetCameraDistance(CurrentZoomDistance);
}

void AUniverseCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle edge scrolling if enabled
	if (bEnableEdgeScrolling)
	{
		HandleEdgeScrolling(DeltaTime);
	}
}

void AUniverseCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Note: Input bindings are handled by PlayerController
	// This pawn receives movement commands via direct function calls
}

// ============================================================================
// CAMERA MOVEMENT
// ============================================================================

void AUniverseCameraPawn::PanForward(float Value)
{
	if (FMath::Abs(Value) < 0.01f)
	{
		return;
	}

	// Pan in camera's forward direction (XY plane, ignore Z)
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();

	FVector NewLocation = GetActorLocation() + (Forward * Value);
	SetActorLocation(NewLocation);
}

void AUniverseCameraPawn::PanRight(float Value)
{
	if (FMath::Abs(Value) < 0.01f)
	{
		return;
	}

	// Pan in camera's right direction (XY plane, ignore Z)
	FVector Right = GetActorRightVector();
	Right.Z = 0.0f;
	Right.Normalize();

	FVector NewLocation = GetActorLocation() + (Right * Value);
	SetActorLocation(NewLocation);
}

void AUniverseCameraPawn::PanUp(float Value)
{
	if (FMath::Abs(Value) < 0.01f)
	{
		return;
	}

	// Pan vertically (world Z axis)
	FVector NewLocation = GetActorLocation() + (FVector::UpVector * Value);
	SetActorLocation(NewLocation);
}

void AUniverseCameraPawn::Zoom(float Value)
{
	if (FMath::Abs(Value) < 0.01f)
	{
		return;
	}

	// Adjust zoom distance
	CurrentZoomDistance = FMath::Clamp(CurrentZoomDistance - Value, MinZoomDistance, MaxZoomDistance);

	if (SpringArm)
	{
		SpringArm->TargetArmLength = CurrentZoomDistance;
	}
}

void AUniverseCameraPawn::RotateYaw(float Value)
{
	if (FMath::Abs(Value) < 0.01f)
	{
		return;
	}

	// Rotate camera around Z axis
	CurrentYaw += Value;
	CurrentYaw = FMath::Fmod(CurrentYaw, 360.0f);

	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw = CurrentYaw;
	SetActorRotation(NewRotation);
}

void AUniverseCameraPawn::RotatePitch(float Value)
{
	if (FMath::Abs(Value) < 0.01f)
	{
		return;
	}

	// Adjust pitch (look up/down)
	CurrentPitch = FMath::Clamp(CurrentPitch + Value, MinPitchAngle, MaxPitchAngle);

	if (SpringArm)
	{
		FRotator NewRotation = SpringArm->GetRelativeRotation();
		NewRotation.Pitch = CurrentPitch;
		SpringArm->SetRelativeRotation(NewRotation);
	}
}

void AUniverseCameraPawn::FocusOnLocation(FVector TargetLocation, float TransitionTime)
{
	// Immediate snap for now (future: smooth transition)
	SetActorLocation(TargetLocation);

	// Future: Implement smooth camera transition using timeline or interpolation
	UE_LOG(LogTemp, Log, TEXT("[UniverseCamera] Focus on location: %s"), *TargetLocation.ToString());
}

float AUniverseCameraPawn::GetCameraDistance() const
{
	return CurrentZoomDistance;
}

void AUniverseCameraPawn::SetCameraDistance(float NewDistance)
{
	CurrentZoomDistance = FMath::Clamp(NewDistance, MinZoomDistance, MaxZoomDistance);

	if (SpringArm)
	{
		SpringArm->TargetArmLength = CurrentZoomDistance;
	}
}

// ============================================================================
// EDGE SCROLLING
// ============================================================================

void AUniverseCameraPawn::HandleEdgeScrolling(float DeltaTime)
{
	// Get player controller
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// Get mouse position
	float MouseX, MouseY;
	if (!PC->GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	// Get viewport size
	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

	// Check if mouse is near screen edges
	FVector2D PanDirection = FVector2D::ZeroVector;

	// Left edge
	if (MouseX < EdgeScrollThreshold)
	{
		PanDirection.X = -1.0f;
	}
	// Right edge
	else if (MouseX > ViewportSizeX - EdgeScrollThreshold)
	{
		PanDirection.X = 1.0f;
	}

	// Top edge
	if (MouseY < EdgeScrollThreshold)
	{
		PanDirection.Y = 1.0f;
	}
	// Bottom edge
	else if (MouseY > ViewportSizeY - EdgeScrollThreshold)
	{
		PanDirection.Y = -1.0f;
	}

	// Apply panning if near edge
	if (!PanDirection.IsNearlyZero())
	{
		PanDirection.Normalize();
		PanRight(PanDirection.X * PanSpeed * DeltaTime);
		PanForward(PanDirection.Y * PanSpeed * DeltaTime);
	}

	LastMousePosition = FVector2D(MouseX, MouseY);
}
