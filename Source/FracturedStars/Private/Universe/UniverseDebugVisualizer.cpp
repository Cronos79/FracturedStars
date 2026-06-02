// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/UniverseDebugVisualizer.h"
#include "Universe/UniverseSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AUniverseDebugVisualizer::AUniverseDebugVisualizer()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AUniverseDebugVisualizer::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("UniverseDebugVisualizer: Ready! Toggle 'Show Visualization' to draw universe."));
}

void AUniverseDebugVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShowVisualization)
	{
		DrawUniverse();
	}
}

void AUniverseDebugVisualizer::RefreshVisualization()
{
	DrawUniverse();
}

void AUniverseDebugVisualizer::DrawUniverse()
{
	// Get universe subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
		return;

	UUniverseSubsystem* UniverseSubsystem = GameInstance->GetSubsystem<UUniverseSubsystem>();
	if (!UniverseSubsystem || !UniverseSubsystem->IsUniverseGenerated())
		return;

	const FUniverseData& Universe = UniverseSubsystem->GetUniverseData();
	const TArray<FStarSystemData>& Systems = Universe.Systems;

	// Draw connections first (so spheres render on top)
	if (bShowConnections)
	{
		for (const FStarSystemData& System : Systems)
		{
			FVector SystemPos = ScalePosition(System.Coordinates);

			for (int32 ConnectedId : System.ConnectedSystemIds)
			{
				// Only draw each connection once (avoid duplicates)
				if (ConnectedId > System.SystemId)
				{
					const FStarSystemData& ConnectedSystem = Systems[ConnectedId];
					FVector ConnectedPos = ScalePosition(ConnectedSystem.Coordinates);

					DrawDebugLine(
						GetWorld(),
						SystemPos,
						ConnectedPos,
						ConnectionColor,
						false, // Persistent
						-1.0f, // Lifetime (one frame)
						0,     // Depth priority
						ConnectionThickness
					);
				}
			}
		}
	}

	// Draw systems
	if (bShowSystems)
	{
		for (const FStarSystemData& System : Systems)
		{
			FVector SystemPos = ScalePosition(System.Coordinates);
			FColor Color = GetRegionColor(System.RegionType);

			// Sprint 2: Override color based on display mode
			if (bShowOwnership && System.ControllingFactionId != -1)
			{
				// Color by faction (cycle through hues)
				float Hue = (System.ControllingFactionId * 70.0f);
				Color = FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue), 200, 255).ToFColor(true);
			}
			else if (bShowPopulation)
			{
				// Color intensity by population (darker = less pop, brighter = more pop)
				int64 TotalPop = 0;
				for (const FLocationData& Loc : System.Locations)
				{
					TotalPop += Loc.Population;
				}
				// Scale: 0 pop = black, 1M+ = bright white
				float Intensity = FMath::Clamp(TotalPop / 1000000.0f, 0.0f, 1.0f);
				uint8 Brightness = static_cast<uint8>(Intensity * 255);
				Color = FColor(Brightness, Brightness, 255); // Blue gradient
			}

			// Draw sphere
			DrawDebugSphere(
				GetWorld(),
				SystemPos,
				SystemSphereRadius,
				12, // Segments
				Color,
				false, // Persistent
				-1.0f  // Lifetime
			);

			// Draw system name (optional)
			if (bShowSystemNames)
			{
				DrawDebugString(
					GetWorld(),
					SystemPos + FVector(0, 0, SystemSphereRadius + 100.0f),
					System.SystemName,
					nullptr,
					Color,
					-1.0f,
					true // Draw shadow
				);
			}

			// Sprint 2: Show location count
			if (bShowLocationCount)
			{
				FString LocationInfo = FString::Printf(TEXT("%d"), System.Locations.Num());
				DrawDebugString(
					GetWorld(),
					SystemPos + FVector(0, 0, SystemSphereRadius + 50.0f),
					LocationInfo,
					nullptr,
					FColor::White,
					-1.0f,
					true
				);
			}
		}
	}

	// Draw faction home systems with extra highlight
	for (int32 FactionIdx = 0; FactionIdx < Universe.FactionHomeSystems.Num(); ++FactionIdx)
	{
		int32 HomeSystemId = Universe.FactionHomeSystems[FactionIdx];
		const FStarSystemData& HomeSystem = Systems[HomeSystemId];
		FVector HomePos = ScalePosition(HomeSystem.Coordinates);

		// Draw larger ring around faction home
		DrawDebugSphere(
			GetWorld(),
			HomePos,
			SystemSphereRadius * 2.0f,
			16,
			FColor::Cyan,
			false,
			-1.0f,
			0,
			5.0f // Thicker
		);

		// Draw faction label
		FString FactionLabel = FString::Printf(TEXT("FACTION %d HOME"), FactionIdx);
		DrawDebugString(
			GetWorld(),
			HomePos + FVector(0, 0, SystemSphereRadius * 2.5f),
			FactionLabel,
			nullptr,
			FColor::Cyan,
			-1.0f,
			true
		);
	}
}

FColor AUniverseDebugVisualizer::GetRegionColor(ERegionType RegionType) const
{
	if (!bUseRegionColors)
		return FColor::White;

	switch (RegionType)
	{
	case ERegionType::FactionCore:
		return FactionCoreColor;
	case ERegionType::FactionFrontier:
		return FactionFrontierColor;
	case ERegionType::Neutral:
		return NeutralColor;
	case ERegionType::Lawless:
		return LawlessColor;
	case ERegionType::Disputed:
		return DisputedColor;
	default:
		return FColor::Magenta; // Error color
	}
}

FVector AUniverseDebugVisualizer::ScalePosition(const FVector& Position) const
{
	// Apply scale and height offset
	FVector Scaled = Position * VisualizationScale;
	Scaled.Z = HeightOffset; // Keep everything on the same Z plane (2D visualization)

	// Offset by actor location
	return GetActorLocation() + Scaled;
}
