#include "Character/AllyAbilityComponent.h"
#include "Manager/StatComponent.h"
#include "Manager/SkillData.h"
#include "Engine/Engine.h"

UAllyAbilityComponent::UAllyAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UAllyAbilityComponent::ExecuteDefaultAttack()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteDefaultAttack: Owner is null"));
		return 0.f;
	}
	if (UStatComponent* StatComp = Owner->FindComponentByClass<UStatComponent>())
	{
		return StatComp->PhysicalAttack;
	}
	return 0.f;
}

float UAllyAbilityComponent::ExecuteSkill(USkillData* Skill)
{
	if (!Skill)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteSkill: Skill is null"));
		return 0.f;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteSkill: Owner is null"));
		return 0.f;
	}

	UStatComponent* StatComp = Owner->FindComponentByClass<UStatComponent>();
	if (!StatComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteSkill: StatComponent not found"));
		return 0.f;
	}

	// Check if the owner has enough Technique Points for this skill.
	if (StatComp->TechniquePoints < Skill->TechniqueCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteSkill: Not enough Technique Points to use %s"), *Skill->SkillName.ToString());
		return 0.f;
	}

	// Deduct the TechniquePoints cost first.
	StatComp->UseTechniquePoints(Skill->TechniqueCost);

	float Result = 0.f;
	switch (Skill->AbilityCategory)
	{
	case EAbilityCategory::Offensive:
	{
		// Calculate base damage from the skill's damage and the owner's attack stat.
		float BaseDamage = Skill->Damage;
		if (Skill->AttackType == EAttackType::Physical)
		{
			BaseDamage += StatComp->PhysicalAttack;
		}
		else // Magical attack
		{
			BaseDamage += StatComp->MagicalAttack;
		}
		Result = BaseDamage;
		break;
	}
	case EAbilityCategory::Heal:
	{
		// Healing: convention is that a negative result represents healing.
		float HealAmount = Skill->Damage;
		Result = -HealAmount;
		break;
	}
	case EAbilityCategory::Buff:
	case EAbilityCategory::Debuff:
	{
		// For buff or debuff skills, apply the modifier on the affected stat.
		StatComp->ApplyStatModifier(Skill->AffectedStat, Skill->ModifierValue, Skill->ModifierType, Skill->Duration);
		Result = Skill->ModifierValue;
		break;
	}
	// Additional categories (e.g., Defensive, Utility) can be implemented here.
	default:
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteSkill: Ability category not implemented"));
		break;
	}
	}
	return Result;
}
