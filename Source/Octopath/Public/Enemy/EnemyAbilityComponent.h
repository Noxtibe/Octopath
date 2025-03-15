#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAbilityComponent.generated.h"

/**
 * UEnemyAbilityComponent
 *
 * Component that handles an enemy’s abilities.
 * For now, it only supports a default attack that deals fixed damage to the player.
 * This component can be extended in the future with additional abilities, cooldowns, etc.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UEnemyAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAbilityComponent();

	// Executes the enemy's default attack (deals fixed damage to the player).
	UFUNCTION(BlueprintCallable, Category = "Enemy Abilities")
	float ExecuteDefaultAttack();
};
