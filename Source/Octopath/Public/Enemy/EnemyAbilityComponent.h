#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAbilityComponent.generated.h"

/**
 * UEnemyAbilityComponent
 *
 * This component handles an enemy’s abilities.
 * Currently, it only supports a default attack that deals fixed damage to the player.
 * This component can be extended in the future with additional abilities, cooldowns, etc.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UEnemyAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Constructors and override functions
	UEnemyAbilityComponent();

public:
	// Public functions
	/**
	 * Executes the enemy's default attack.
	 * @return The calculated damage value to be applied to the player.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Abilities")
	float ExecuteDefaultAttack();
};
