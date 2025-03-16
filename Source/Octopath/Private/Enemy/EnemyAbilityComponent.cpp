#include "Enemy/EnemyAbilityComponent.h"
#include "Manager/StatComponent.h"
#include "Manager/SkillData.h"

UEnemyAbilityComponent::UEnemyAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UEnemyAbilityComponent::ExecuteDefaultAttack()
{
	UE_LOG(LogTemp, Log, TEXT("EnemyAbilityComponent::ExecuteDefaultAttack - Called"));
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyAbilityComponent::ExecuteDefaultAttack - Owner is null"));
		return 0.f;
	}
	UStatComponent* EnemyStat = Owner->FindComponentByClass<UStatComponent>();
	if (!EnemyStat)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyAbilityComponent::ExecuteDefaultAttack - StatComponent not found"));
		return 0.f;
	}
	float DamageAmount = EnemyStat->PhysicalAttack;
	UE_LOG(LogTemp, Log, TEXT("EnemyAbilityComponent::ExecuteDefaultAttack - DamageAmount calculated: %f"), DamageAmount);
	return DamageAmount;
}

float UEnemyAbilityComponent::ExecuteSkill(USkillData* Skill)
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
		Result = -HealAmount;
		break;
	}
	case EAbilityCategory::Buff:
	case EAbilityCategory::Debuff:
	{
		StatComp->ApplyStatModifier(Skill->AffectedStat, Skill->ModifierValue, Skill->ModifierType, Skill->Duration);
		StatComp->UseTechniquePoints(Skill->TechniqueCost);
		Result = Skill->ModifierValue;
		break;
	}
	default:
		break;
	}
	return Result;
}
