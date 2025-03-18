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

float UAllyAbilityComponent::ExecuteSkill(USkillData* Skill, const TArray<AActor*>& Targets)
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

    // Deduct the TechniquePoints cost.
    StatComp->UseTechniquePoints(Skill->TechniqueCost);

    float Result = 0.f;
    switch (Skill->AbilityCategory)
    {
    case EAbilityCategory::Offensive:
    {
        // For offensive skills, calculate damage and apply to all targets.
        float BaseDamage = Skill->Damage;
        if (Skill->AttackType == EAttackType::Physical)
        {
            BaseDamage += StatComp->PhysicalAttack;
        }
        else // Magical attack
        {
            BaseDamage += StatComp->MagicalAttack;
        }
        // Optionally, you pouvez itérer sur Targets pour appliquer les dégâts
        for (AActor* Target : Targets)
        {
            if (IsValid(Target))
            {
                if (UStatComponent* TargetStat = Target->FindComponentByClass<UStatComponent>())
                {
                    TargetStat->ApplyDamage(BaseDamage, (Skill->AttackType == EAttackType::Magical));
                }
            }
        }
        Result = BaseDamage;
        break;
    }
    case EAbilityCategory::Heal:
    {
        // For healing, the DataAsset's Damage value represents the maximum healing potential.
        // We simply call Heal(HealAmount) on the target’s StatComponent.
        // The StatComponent::Heal function clamps the health to not exceed MaxHealth.
        float HealAmount = Skill->Damage;
        for (AActor* Target : Targets)
        {
            if (IsValid(Target))
            {
                if (UStatComponent* TargetStat = Target->FindComponentByClass<UStatComponent>())
                {
                    TargetStat->Heal(HealAmount);
                    UE_LOG(LogTemp, Log, TEXT("Heal applied to %s, amount: %f"), *Target->GetName(), HealAmount);
                }
            }
        }
        Result = -HealAmount; // Convention: negative value indicates healing effect.
        break;
    }
    case EAbilityCategory::Buff:
    case EAbilityCategory::Debuff:
    {
        // For buff/debuff skills, apply the modifier to each target.
        for (AActor* Target : Targets)
        {
            if (IsValid(Target))
            {
                if (UStatComponent* TargetStat = Target->FindComponentByClass<UStatComponent>())
                {
                    TargetStat->ApplyStatModifier(Skill->AffectedStat, Skill->ModifierValue, Skill->ModifierType, Skill->Duration);
                }
            }
        }
        Result = Skill->ModifierValue;
        break;
    }
    default:
    {
        UE_LOG(LogTemp, Warning, TEXT("ExecuteSkill: Ability category not implemented"));
        break;
    }
    }
    return Result;
}