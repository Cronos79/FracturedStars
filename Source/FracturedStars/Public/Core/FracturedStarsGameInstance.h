// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FracturedStarsGameInstance.generated.h"

/**
 * FracturedStars Game Instance
 * 
 * Purpose:
 * - Persistent across level transitions
 * - Future home for save/load coordination
 * - Future home for session management
 * - Future home for meta-game state
 * 
 * Currently a placeholder for future Sprint expansion.
 */
UCLASS()
class FRACTUREDSTARS_API UFracturedStarsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFracturedStarsGameInstance();

	// UGameInstance interface
	virtual void Init() override;
	virtual void Shutdown() override;

protected:
	// Future: Save/Load management
	// Future: Session management
	// Future: Meta-game state (campaign progress, unlocks, etc.)
};
