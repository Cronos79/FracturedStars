// Test console command to verify UniverseCore works
// Add this to your game mode or player controller's BeginPlay for testing

// To test in game:
// 1. Open Visual Studio solution (FracturedStars.sln)
// 2. Build from VS (should bypass the security issue)
// 3. Launch editor
// 4. Press ` (tilde) to open console
// 5. Type: TestUniverse

/*
Example usage in your game:

void AYourGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Spawn Universe Manager
	AUniverseManager* Manager = GetWorld()->SpawnActor<AUniverseManager>();
	Manager->GenerateUniverse(12345, 50);
	Manager->PrintUniverseInfo();

	// Query a system
	FStarSystemInfo SystemInfo = Manager->GetSystemInfo(0);
	UE_LOG(LogTemp, Log, TEXT("System 0: %s"), *SystemInfo.SystemName);
}
*/
