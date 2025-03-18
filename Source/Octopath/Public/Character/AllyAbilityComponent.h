#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Manager/SkillData.h"
#include "AllyAbilityComponent.generated.h"

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
	// Constructor
	UAllyAbilityComponent();

public:
	/**
	 * Executes the default attack.
	 * Returns damage based on the owner's PhysicalAttack stat.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ally Abilities")
	float ExecuteDefaultAttack();

	/**
	 * Executes a custom skill.
	 *
	 * For offensive or healing skills, it calculates and returns the damage/healing value.
	 * For buff/debuff skills, it applies the modifier to the affected stat.
	 *
	 * This function deducts the TechniqueCost from the owner's StatComponent before execution.
	 *
	 * @param Skill - The skill data asset to execute.
	 * @return The resulting effect value (e.g., damage, healing, or modifier value).
	 */
	UFUNCTION(BlueprintCallable, Category = "Ally Abilities")
	float ExecuteSkill(USkillData* Skill);

	/** Array of skills available to this allied character */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ally Abilities")
	TArray<USkillData*> Skills;
};
