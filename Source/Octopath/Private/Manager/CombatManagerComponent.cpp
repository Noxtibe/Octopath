#include "Manager/CombatManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "Game/OctopathGameInstance.h"

UCombatManagerComponent::UCombatManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default enemy setup values
	EnemySpawnDistance = 600.f;
	MaxSpreadAngle = 60.f; // Total spread angle in degrees

	// Default combat camera settings
	CombatCameraOffset = FVector(-600.f, -600.f, 1200.f);
	CameraBlendTime = 1.0f;
	bUseDynamicCameraFocus = true;
	FixedFocusPoint = FVector::ZeroVector;
}

void UCombatManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set up the combat scene when the component initializes.
	SetupCombat();
}

TArray<FVector> UCombatManagerComponent::ComputeEnemySpawnPositions(const FVector& PlayerLocation, const FVector& PlayerForward, int32 NumEnemies, float BaseDistance, float TotalSpreadAngle)
{
	TArray<FVector> SpawnPositions;

	if (NumEnemies <= 0)
	{
		return SpawnPositions;
	}

	// If only one enemy, spawn it directly in front.
	if (NumEnemies == 1)
	{
		SpawnPositions.Add(PlayerLocation + PlayerForward * BaseDistance);
		return SpawnPositions;
	}

	// Compute the angle step and starting angle.
	float AngleStep = TotalSpreadAngle / (NumEnemies - 1);
	float StartAngle = -TotalSpreadAngle / 2.0f;

	// Loop through and calculate each enemy's spawn position.
	for (int32 i = 0; i < NumEnemies; i++)
	{
		float AngleOffset = StartAngle + i * AngleStep;
		// Create a rotation offset about the Up axis.
		FRotator RotationOffset(0.f, AngleOffset, 0.f);
		// Rotate the forward vector by the angle offset.
		FVector SpawnDirection = RotationOffset.RotateVector(PlayerForward);
		SpawnPositions.Add(PlayerLocation + SpawnDirection * BaseDistance);
	}

	return SpawnPositions;
}

void UCombatManagerComponent::SetupCombat()
{
	// Get the player character.
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerCharacter)
	{
		// Position the player at the designated spawn location and rotation.
		PlayerCharacter->SetActorLocation(PlayerSpawnLocation);
		PlayerCharacter->SetActorRotation(PlayerSpawnRotation);

		// Disable free movement and camera look input during combat.
		if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
		{
			PC->SetIgnoreMoveInput(true);
			PC->SetIgnoreLookInput(true);
		}
	}

	// Retrieve the enemy count from the GameInstance.
	UOctopathGameInstance* GI = Cast<UOctopathGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	int32 NumEnemies = (GI) ? GI->NumberOfEnemies : 1;

	// Compute enemy spawn positions dynamically.
	FVector PlayerForward = PlayerSpawnRotation.Vector();
	TArray<FVector> SpawnPositions = ComputeEnemySpawnPositions(PlayerSpawnLocation, PlayerForward, NumEnemies, EnemySpawnDistance, MaxSpreadAngle);

	// Spawn enemies at the computed positions.
	for (const FVector& SpawnLocation : SpawnPositions)
	{
		// Compute rotation so that the enemy faces the player.
		FRotator SpawnRotation = (PlayerSpawnLocation - SpawnLocation).Rotation();
		GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLocation, SpawnRotation);
	}

	// --- Configure the Combat Camera ---

	// Determine the focus point.
	FVector FocusPoint = PlayerSpawnLocation; // Default focus is the player's location.
	if (bUseDynamicCameraFocus && SpawnPositions.Num() > 0)
	{
		FVector SumPositions = FVector::ZeroVector;
		for (const FVector& Pos : SpawnPositions)
		{
			SumPositions += Pos;
		}
		FVector AvgEnemyPos = SumPositions / SpawnPositions.Num();
		FocusPoint = (PlayerSpawnLocation + AvgEnemyPos) * 0.5f;
	}
	else
	{
		FocusPoint = FixedFocusPoint;
	}

	// Calculate the camera's world location using the focus point and the specified offset.
	FVector CombatCameraLocation = FocusPoint + CombatCameraOffset;
	// Compute the rotation for the camera so that it looks at the focus point.
	FRotator CombatCameraRotation = (FocusPoint - CombatCameraLocation).Rotation();

	// Spawn a CameraActor to serve as the combat camera.
	ACameraActor* CombatCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CombatCameraLocation, CombatCameraRotation);
	if (CombatCamera)
	{
		// Set the combat camera as the view target with a smooth blend.
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			PC->SetViewTargetWithBlend(CombatCamera, CameraBlendTime);
		}
	}
}
