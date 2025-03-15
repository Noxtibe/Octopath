#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatManagerComponent.generated.h"

/**
 * UCombatManagerComponent
 *
 * This component sets up the combat scene by positioning the player,
 * dynamically spawning enemies along an arc, and configuring a fixed combat camera.
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UCombatManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatManagerComponent();
	virtual void BeginPlay() override;

public:
	// Public functions
	/**
	 * Sets up the combat scene: positions the player, spawns enemies, and configures the combat camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat Setup")
	void SetupCombat();

public:
	// Public variables

	// --- Combat Setup Properties ---
	/** Fixed spawn location for the player in the combat map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup")
	FVector PlayerSpawnLocation;

	/** Fixed spawn rotation for the player so that they face the enemies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup")
	FRotator PlayerSpawnRotation;

	/** The enemy actor class to spawn during combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup")
	TSubclassOf<AActor> EnemyClass;

	/** The distance from the player at which enemies will be spawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup|Enemies")
	float EnemySpawnDistance;

	/** The total angular spread (in degrees) for enemy placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup|Enemies")
	float MaxSpreadAngle;

	// --- Combat Camera Properties ---
	/** Offset from the computed focus point where the combat camera will be placed.
		Adjust this so that the camera is high above, with the player in the bottom-right
		and enemies in the top-left. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup|Camera")
	FVector CombatCameraOffset;

	/** Blend time when switching to the combat camera view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup|Camera")
	float CameraBlendTime;

	/** If true, compute the camera focus dynamically as the midpoint between the player and the enemies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup|Camera")
	bool bUseDynamicCameraFocus;

	/** Fixed focus point to use when dynamic focus is disabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Setup|Camera")
	FVector FixedFocusPoint;

private:
	// Private functions
	/**
	 * Helper function to compute enemy spawn positions along an arc in front of the player.
	 *
	 * @param PlayerLocation   The fixed spawn location of the player.
	 * @param PlayerForward    The forward vector derived from the player's spawn rotation.
	 * @param NumEnemies       The number of enemies to spawn.
	 * @param BaseDistance     The distance from the player at which enemies will be spawned.
	 * @param TotalSpreadAngle The total angular spread (in degrees) over which enemies will be distributed.
	 * @return An array of computed world positions for enemy spawns.
	 */
	TArray<FVector> ComputeEnemySpawnPositions(const FVector& PlayerLocation, const FVector& PlayerForward, int32 NumEnemies, float BaseDistance, float TotalSpreadAngle);
};
