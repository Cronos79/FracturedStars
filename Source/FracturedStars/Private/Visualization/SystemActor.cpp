// Copyright (c) 2026 Matthew / FracturedStars. All Rights Reserved.

#include "Visualization/SystemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ASystemActor::ASystemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create mesh component
	SystemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SystemMesh"));
	RootComponent = SystemMesh;

	// Enable click events
	SystemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SystemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SystemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SystemMesh->SetGenerateOverlapEvents(true);

	// Load default sphere mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		SystemMesh->SetStaticMesh(SphereMeshAsset.Object);
	}

	// Initialize values
	SystemId = -1;
	SystemName = TEXT("Unknown System");
	bIsSelected = false;
	bIsHovered = false;

	// Default colors
	NormalColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);    // Blue
	HoverColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);     // Light Blue
	SelectedColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);  // Gold

	// Default scales
	NormalScale = 1.0f;
	HoverScale = 1.2f;
	SelectedScale = 1.5f;
}

void ASystemActor::BeginPlay()
{
	Super::BeginPlay();

	// Create dynamic material instance
	if (SystemMesh && SystemMesh->GetMaterial(0))
	{
		DynamicMaterial = SystemMesh->CreateDynamicMaterialInstance(0);
		UpdateVisualState();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SystemActor: No material found on SystemMesh for system %d"), SystemId);
	}
}

void ASystemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
		DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), TargetColor);
	}

	// Update scale
	SystemMesh->SetWorldScale3D(FVector(TargetScale));
}
