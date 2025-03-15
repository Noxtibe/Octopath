#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AllyAbilityComponent.generated.h"

/**
 * UAllyAbilityComponent
 *
 * This component handles an allied character's abilities.
 * For now, it provides a default attack that deals damage equal to the owner's PhysicalAttack stat,
 * as defined in the owner's StatComponent.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UAllyAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAllyAbilityComponent();

public:
	/**
	 * Executes the default attack.
	 * @return The damage amount, taken from the owner's PhysicalAttack stat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ally Abilities")
	float ExecuteDefaultAttack();
};
