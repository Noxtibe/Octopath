#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OctopathGameInstance.generated.h"

/**
 * UOctopathGameInstance
 *
 * This custom Game Instance class for Octopath stores global game data.
 * For example, it holds the number of enemies to spawn during combat.
 */
UCLASS()
class OCTOPATH_API UOctopathGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Constructors and override functions
	UOctopathGameInstance();

public:
	// Public functions
	/**
	 * Resets game data to default values.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game Data")
	void ResetGameData();

public:
	// Public variables
	/** Number of enemies to spawn in the combat phase (set by the EnemyTrigger) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 NumberOfEnemies;
};
