#include "Character/AllyAbilityComponent.h"
#include "Manager/StatComponent.h"
#include "Manager/SkillData.h"

UAllyAbilityComponent::UAllyAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UAllyAbilityComponent::ExecuteDefaultAttack()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
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
		return 0.f;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0.f;
	}

	UStatComponent* StatComp = Owner->FindComponentByClass<UStatComponent>();
	if (!StatComp)
	{
		return 0.f;
	}

	float Result = 0.f;
	switch (Skill->AbilityCategory)
	{
	case EAbilityCategory::Offensive:
	{
		float BaseDamage = Skill->Damage;
		if (Skill->AttackType == EAttackType::Physical)
		{
			BaseDamage += StatComp->PhysicalAttack;
		}
		else
		{
			BaseDamage += StatComp->MagicalAttack;
		}
		StatComp->UseTechniquePoints(Skill->TechniqueCost);
		Result = BaseDamage;
		break;
	}
	case EAbilityCategory::Heal:
	{
		float HealAmount = Skill->Damage;
		StatComp->UseTechniquePoints(Skill->TechniqueCost);
		// Convention: negative result indicates healing.
		Result = -HealAmount;
		break;
	}
	case EAbilityCategory::Buff:
	case EAbilityCategory::Debuff:
	{
		// Here, we delegate the buff/debuff logic to the StatComponent.
		StatComp->ApplyStatModifier(Skill->AffectedStat, Skill->ModifierValue, Skill->ModifierType, Skill->Duration);
		StatComp->UseTechniquePoints(Skill->TechniqueCost);
		Result = Skill->ModifierValue;
		break;
	}
	// You can add other cases (Defensive, Utility, etc.) as needed.
	default:
		break;
	}
	return Result;
}
