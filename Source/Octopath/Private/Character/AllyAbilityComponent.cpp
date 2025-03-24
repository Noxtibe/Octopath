#include "Character/AllyAbilityComponent.h"
#include "Manager/StatComponent.h"
#include "Manager/SkillData.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogAllyAbilityComponent, Log, All);

UAllyAbilityComponent::UAllyAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UAllyAbilityComponent::ExecuteDefaultAttack()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogAllyAbilityComponent, Warning, TEXT("Owner is null"));
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
        UE_LOG(LogAllyAbilityComponent, Warning, TEXT("Skill is null"));
        return 0.f;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogAllyAbilityComponent, Warning, TEXT("Owner is null"));
        return 0.f;
    }

    UStatComponent* StatComp = Owner->FindComponentByClass<UStatComponent>();
    if (!StatComp)
    {
        UE_LOG(LogAllyAbilityComponent, Warning, TEXT("StatComponent not found"));
        return 0.f;
    }

    // Check if the caster has enough Technique Points to use the skill.
    if (StatComp->TechniquePoints < Skill->TechniqueCost)
    {
        UE_LOG(LogAllyAbilityComponent, Warning, TEXT("Not enough Technique Points to use %s"), *Skill->SkillName.ToString());
        return 0.f;
    }

    // Deduct the TechniquePoints cost.
    StatComp->UseTechniquePoints(Skill->TechniqueCost);

    float TotalEffect = 0.f;

    if (Skill->AbilityCategory == EAbilityCategory::Offensive)
    {
        // Offensive abilities: apply damage calculation to each target.
        for (AActor* Target : Targets)
        {
            if (!IsValid(Target))
            {
                continue;
            }
            UStatComponent* TargetStat = Target->FindComponentByClass<UStatComponent>();
            if (!TargetStat)
            {
                continue;
            }

            float AttackValue = 0.f;
            float DefenceValue = 0.f;
            if (Skill->AttackType == EAttackType::Physical)
            {
                // Attack value = base skill damage + caster's physical attack.
                AttackValue = Skill->Damage + StatComp->PhysicalAttack;
                // Defence value = target's physical defense.
                DefenceValue = TargetStat->PhysicalDefense;
            }
            else // Magical attack
            {
                // Attack value = base skill damage + caster's magical attack.
                AttackValue = Skill->Damage + StatComp->MagicalAttack;
                // Defence value = target's magical defense.
                DefenceValue = TargetStat->MagicalDefense;
            }

            // Generate a random multiplier using modifiable min and max values.
            float RandomMultiplier = FMath::FRandRange(RandomMultiplierMin, RandomMultiplierMax);
            // Calculate damage using the formula:
            // Damage = (AttackValue - (DefenceValue * DamageDefenceRatio / DamageDefenceDivisor)) * RandomMultiplier.
            float DamageDealt = (AttackValue - (DefenceValue * DamageDefenceRatio / DamageDefenceDivisor)) * RandomMultiplier;
            DamageDealt = FMath::Max(DamageDealt, 1.f); // Ensure at least 1 damage is dealt.

            UE_LOG(LogAllyAbilityComponent, Log, TEXT("Damage Calculation Formula: (%.2f - (%.2f * %.2f / %.2f)) * %.2f = %.2f"),
                AttackValue, DefenceValue, DamageDefenceRatio, DamageDefenceDivisor, RandomMultiplier, DamageDealt);

            // Apply damage to the target. The second parameter indicates if damage is magical.
            TargetStat->ApplyDamage(DamageDealt, (Skill->AttackType == EAttackType::Magical));
            UE_LOG(LogAllyAbilityComponent, Log, TEXT("Applied %f damage to target %s"), DamageDealt, *Target->GetName());
            TotalEffect += DamageDealt;
        }
    }
    else if (Skill->AbilityCategory == EAbilityCategory::Heal)
    {
        // Heal abilities: treat the damage value as the amount to heal.
        for (AActor* Target : Targets)
        {
            if (!IsValid(Target))
            {
                continue;
            }
            float HealAmount = Skill->Damage;
            UStatComponent* TargetStat = Target->FindComponentByClass<UStatComponent>();
            if (TargetStat)
            {
                TargetStat->Heal(HealAmount);
                UE_LOG(LogAllyAbilityComponent, Log, TEXT("Healed target %s for %f"), *Target->GetName(), HealAmount);
                // Convention: negative result represents healing.
                TotalEffect -= HealAmount;
            }
        }
    }
    else if (Skill->AbilityCategory == EAbilityCategory::Buff ||
        Skill->AbilityCategory == EAbilityCategory::Debuff)
         {
        // For buffs and debuffs, delegate to StatComponent.
        StatComp->ApplyStatModifier(Skill->AffectedStat, Skill->ModifierValue, Skill->ModifierType, Skill->Duration);
        UE_LOG(LogAllyAbilityComponent, Log, TEXT("Applied modifier %f to stat"), Skill->ModifierValue);
        TotalEffect = Skill->ModifierValue;
         }
    else
    {
        UE_LOG(LogAllyAbilityComponent, Warning, TEXT("Ability category not implemented"));
    }

    return TotalEffect;
}
