#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AllyAbilityComponent.generated.h"

class USkillData;

/**
 * UAllyAbilityComponent
 *
 * This component handles an allied character's abilities.
 * It provides methods for executing both the default attack and custom skills.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UAllyAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAllyAbilityComponent();

public:
	/** Executes the default attack. Returns damage based on the owner's PhysicalAttack stat. */
	UFUNCTION(BlueprintCallable, Category = "Ally Abilities")
	float ExecuteDefaultAttack();

	/**
	 * Executes a custom skill.
	 * For offensive/healing skills, returns the calculated damage/healing.
	 * For buffs/debuffs, applies the modifier to the affected stat.
	 *
	 * @param Skill - The skill to execute.
	 * @return The resulting effect value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ally Abilities")
	float ExecuteSkill(USkillData* Skill);

	/** Array of skills available to this allied character. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ally Abilities")
	TArray<USkillData*> Skills;
};
