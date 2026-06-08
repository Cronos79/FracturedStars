// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Visualization/SystemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ASystemActor::ASystemActor()
{
	PrimaryActorTick.bCanEverTick = true; // Enable tick for pulse animation

	// Create root scene component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create system mesh component (small dot)
	SystemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SystemMesh"));
	SystemMesh->SetupAttachment(RootComponent);

	// Create glow ring mesh (for hover)
	GlowRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlowRingMesh"));
	GlowRingMesh->SetupAttachment(RootComponent);

	// Create selection ring mesh (for selection + pulse)
	SelectionRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionRingMesh"));
	SelectionRingMesh->SetupAttachment(RootComponent);

	// Create homeworld icon mesh (cube above the system)
	HomeworldIconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HomeworldIconMesh"));
	HomeworldIconMesh->SetupAttachment(RootComponent);

	// Create player asset icon mesh (cylinder above the system)
	PlayerAssetIconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerAssetIconMesh"));
	PlayerAssetIconMesh->SetupAttachment(RootComponent);

	// Enable click events on the system mesh
	SystemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SystemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SystemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SystemMesh->SetGenerateOverlapEvents(true);

	// Glow ring, selection ring, and icons should not block clicks
	GlowRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HomeworldIconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerAssetIconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Load default meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));

	if (SphereMeshAsset.Succeeded())
	{
		SystemMesh->SetStaticMesh(SphereMeshAsset.Object);
		GlowRingMesh->SetStaticMesh(SphereMeshAsset.Object);
		SelectionRingMesh->SetStaticMesh(SphereMeshAsset.Object);

		// Scale the rings to be larger than the dot
		GlowRingMesh->SetRelativeScale3D(FVector(1.5f));       // 1.5x size for hover glow
		SelectionRingMesh->SetRelativeScale3D(FVector(2.0f));  // 2.0x size for selection ring
	}

	if (CubeMeshAsset.Succeeded())
	{
		HomeworldIconMesh->SetStaticMesh(CubeMeshAsset.Object);
		// Position above the system
		HomeworldIconMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
		HomeworldIconMesh->SetRelativeScale3D(FVector(0.3f));
	}

	if (CylinderMeshAsset.Succeeded())
	{
		PlayerAssetIconMesh->SetStaticMesh(CylinderMeshAsset.Object);
		// Position above the system (higher than homeworld icon)
		PlayerAssetIconMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
		PlayerAssetIconMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.5f));
	}

	// Initially hide the rings and icons
	GlowRingMesh->SetVisibility(false);
	SelectionRingMesh->SetVisibility(false);
	HomeworldIconMesh->SetVisibility(false);
	PlayerAssetIconMesh->SetVisibility(false);

	// Initialize values
	SystemId = -1;
	SystemName = TEXT("Unknown System");
	bIsSelected = false;
	bIsHovered = false;
	bIsHomeworld = false;
	bHasPlayerAssets = false;
	OwningFactionId = -1;
	PulseTime = 0.0f;
	PulseSpeed = 1.0f; // 1 cycle per second

	// Default colors
	NormalColor = FLinearColor(1.0f, 0.4f, 0.0f, 1.0f);    // Orange
	HoverColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);     // Yellow
	SelectedColor = FLinearColor(0.2f, 0.8f, 1.0f, 1.0f);  // Cyan

	// Default scales
	NormalScale = 1.0f;
	HoverScale = 1.1f;
	SelectedScale = 1.2f;
}

void ASystemActor::BeginPlay()
{
	Super::BeginPlay();

	// Explicitly load BasicShapeMaterial so we control the parameter name ("Color")
	UMaterial* BaseMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	if (BaseMat)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		if (DynamicMaterial && SystemMesh)
		{
			SystemMesh->SetMaterial(0, DynamicMaterial);
		}

		GlowRingMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		if (GlowRingMaterial && GlowRingMesh)
		{
			GlowRingMesh->SetMaterial(0, GlowRingMaterial);
		}

		SelectionRingMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		if (SelectionRingMaterial && SelectionRingMesh)
		{
			SelectionRingMesh->SetMaterial(0, SelectionRingMaterial);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SystemActor %d: Failed to load BasicShapeMaterial, falling back"), SystemId);
		if (SystemMesh && SystemMesh->GetMaterial(0))
			DynamicMaterial = SystemMesh->CreateDynamicMaterialInstance(0);
		if (GlowRingMesh && GlowRingMesh->GetMaterial(0))
			GlowRingMaterial = GlowRingMesh->CreateDynamicMaterialInstance(0);
		if (SelectionRingMesh && SelectionRingMesh->GetMaterial(0))
			SelectionRingMaterial = SelectionRingMesh->CreateDynamicMaterialInstance(0);
	}

	UpdateVisualState();
}

void ASystemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Animate pulse effect if selected
	if (bIsSelected && SelectionRingMesh && SelectionRingMesh->IsVisible())
	{
		PulseTime += DeltaTime * PulseSpeed;

		// Create a pulsing scale effect (sine wave between 0.9 and 1.1)
		float PulseValue = FMath::Sin(PulseTime * 2.0f * PI) * 0.1f + 1.0f;
		SelectionRingMesh->SetRelativeScale3D(FVector(2.0f * PulseValue));

		// Pulse the color brightness (sine wave between 0.6 and 1.4x)
		if (SelectionRingMaterial)
		{
			float PulseValue2 = FMath::Sin(PulseTime * 2.0f * PI) * 0.4f + 1.0f; // 0.6 to 1.4
			FLinearColor PulsedColor = SelectedColor * PulseValue2;
			SelectionRingMaterial->SetVectorParameterValue(FName("Color"), PulsedColor);
		}
	}
}

void ASystemActor::OnSystemClicked()
{
	UE_LOG(LogTemp, Log, TEXT("SystemActor: System %d (%s) clicked"), SystemId, *SystemName);

	// The player controller will handle selection state
	// We just notify that we were clicked
}

void ASystemActor::SetSelected(bool bSelected)
{
	if (bIsSelected != bSelected)
	{
		bIsSelected = bSelected;
		UpdateVisualState();
		UE_LOG(LogTemp, Log, TEXT("SystemActor: System %d (%s) selection set to %s"), 
			SystemId, *SystemName, bIsSelected ? TEXT("true") : TEXT("false"));
	}
}

void ASystemActor::SetHovered(bool bHovered)
{
	if (bIsHovered != bHovered)
	{
		bIsHovered = bHovered;
		UpdateVisualState();
	}
}

void ASystemActor::SetHomeworld(bool bHomeworld, int32 FactionId)
{
	bIsHomeworld = bHomeworld;
	OwningFactionId = FactionId;
	UpdateVisualState();
}

void ASystemActor::SetPlayerAssets(bool bHasAssets)
{
	bHasPlayerAssets = bHasAssets;
	UpdateVisualState();
}

void ASystemActor::SetBaseColor(FLinearColor NewColor)
{
	NormalColor = NewColor;
	UpdateVisualState();
}

void ASystemActor::ApplyLawfulnessColor(float Lawfulness)
{
	SetBaseColor(GetLawfulnessColor(Lawfulness));
}

void ASystemActor::ApplyFactionColor(int32 FactionId)
{}

void ASystemActor::SetMapColorMode(EGalaxyMapColorMode Mode)
{}

FLinearColor ASystemActor::GetLawfulnessColor(float Lawfulness) const
{
	// Accept either 0-1 or 0-100 input.
	if (Lawfulness > 1.0f)
	{
		Lawfulness /= 100.0f;
	}

	Lawfulness = FMath::Clamp(Lawfulness, 0.0f, 1.0f);

	const FLinearColor Red = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
	const FLinearColor Yellow = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
	const FLinearColor Green = FLinearColor(0.0f, 1.0f, 0.2f, 1.0f);

	if (Lawfulness < 0.5f)
	{
		return FLinearColor::LerpUsingHSV(Red, Yellow, Lawfulness / 0.5f);
	}

	return FLinearColor::LerpUsingHSV(Yellow, Green, (Lawfulness - 0.5f) / 0.5f);
}

void ASystemActor::UpdateVisualState()
{
	if (!SystemMesh)
	{
		return;
	}

	// Determine color and scale based on state
	FLinearColor TargetColor = NormalColor;
	float TargetScale = NormalScale;

	if (bIsSelected)
	{
		TargetColor = SelectedColor;
		TargetScale = SelectedScale;
	}
	else if (bIsHovered)
	{
		TargetColor = HoverColor;
		TargetScale = HoverScale;
	}

	// Update material color if we have a dynamic material
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(FName("Color"), TargetColor);
	}

	// Update scale
	SystemMesh->SetWorldScale3D(FVector(TargetScale));

	// Show/hide glow ring based on hover state
	if (GlowRingMesh)
	{
		GlowRingMesh->SetVisibility(bIsHovered && !bIsSelected); // Only show on hover if not selected

		if (bIsHovered && GlowRingMaterial)
		{
			// Bright yellow/gold for hover glow
			GlowRingMaterial->SetVectorParameterValue(FName("Color"), HoverColor);
		}
	}

	// Show/hide selection ring based on selection state
	if (SelectionRingMesh)
	{
		SelectionRingMesh->SetVisibility(bIsSelected);

		if (bIsSelected && SelectionRingMaterial)
		{
			// Cyan/blue for selection ring
			SelectionRingMaterial->SetVectorParameterValue(FName("Color"), SelectedColor);
			PulseTime = 0.0f; // Reset pulse when selection changes
		}
	}

	// Show/hide homeworld icon
	if (HomeworldIconMesh)
	{
		HomeworldIconMesh->SetVisibility(bIsHomeworld);
	}

	// Show/hide player asset icon
	if (PlayerAssetIconMesh)
	{
		PlayerAssetIconMesh->SetVisibility(bHasPlayerAssets);
	}
}
