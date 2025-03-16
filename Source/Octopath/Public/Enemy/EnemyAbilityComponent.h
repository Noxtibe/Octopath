#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAbilityComponent.generated.h"

class USkillData;

/**
 * UEnemyAbilityComponent
 *
 * This component handles an enemy’s abilities.
 * It provides methods for executing the default attack and custom skills.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UEnemyAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAbilityComponent();

public:
	/** Executes the enemy's default attack. Returns damage based on the owner's PhysicalAttack stat. */
	UFUNCTION(BlueprintCallable, Category = "Enemy Abilities")
	float ExecuteDefaultAttack();

	/**
	 * Executes a custom skill.
	 * @param Skill - The skill to execute.
	 * @return The resulting effect value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Abilities")
	float ExecuteSkill(USkillData* Skill);

	/** Array of skills available to this enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Abilities")
	TArray<USkillData*> Skills;
};
