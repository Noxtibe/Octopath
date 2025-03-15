#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyTrigger.generated.h"

/**
 * AEnemyTrigger
 *
 * This actor is responsible for triggering combat scenarios.
 * It uses a sphere collision component to detect when the player overlaps with it.
 * Upon overlap, it loads a specified combat level and determines the number of enemies to spawn.
 */

 // Forward declarations
class USphereComponent;

UCLASS()
class OCTOPATH_API AEnemyTrigger : public AActor
{
	GENERATED_BODY()

public:
	AEnemyTrigger();
	virtual void BeginPlay() override;

public:
	// Public functions
	/**
	 * Function called when an overlap begins.
	 * @param OverlappedComponent - The component that was overlapped.
	 * @param OtherActor - The other actor involved in the overlap.
	 * @param OtherComp - The other component involved in the overlap.
	 * @param OtherBodyIndex - The index of the other body.
	 * @param bFromSweep - Whether the overlap was from a sweep.
	 * @param SweepResult - Details about the sweep result.
	 */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:
	// Public variables
	/** Collision component used to detect overlaps with the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	USphereComponent* CollisionSphere;

	/** Name of the combat level to load when the player overlaps this trigger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName CombatLevelName;

	/** Number of enemies to spawn during combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 EnemiesCount;
};
