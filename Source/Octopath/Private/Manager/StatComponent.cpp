#include "Manager/StatComponent.h"
#include "Math/UnrealMathUtility.h"


UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default entity name.
	EntityName = FText::FromString("Unnamed");

	// Set default values:
	// Default for non-boss characters:
	MaxHealth = 250.f;
	Health = MaxHealth;

	// Technique Points
	MaxTechniquePoints = 50.f;
	TechniquePoints = MaxTechniquePoints;

	// Defense Stats
	PhysicalDefense = 30.f;
	MagicalDefense = 30.f;

	// Attack Stats (default values, adjustable in the editor)
	PhysicalAttack = 0.f;
	MagicalAttack = 0.f;

	// Speed
	Speed = 80.f;

	// Boss flag defaults to false.
	bIsBoss = false;

	bIsDefending = false;
	DefenseReductionPercentage = 0.3f;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
	BasePhysicalAttack = PhysicalAttack;
	BaseMagicalAttack = MagicalAttack;
	BasePhysicalDefense = PhysicalDefense;
	BaseMagicalDefense = MagicalDefense;
	BaseSpeed = Speed;
}

void UStatComponent::ApplyDamage(float DamageAmount, bool bIsMagical)
{
    // Since damage is already calculated (including defense), simply use it.
    float EffectiveDamage = DamageAmount;

    // If the actor is defending, apply the defense reduction multiplier.
    if (bIsDefending)
    {
        EffectiveDamage *= (1.f - DefenseReductionPercentage);
    }

    Health -= EffectiveDamage;

    float HealthClampMax = bIsBoss ? MaxHealth : FMath::Min(MaxHealth, 10000.f);
    Health = FMath::Clamp(Health, 0.f, HealthClampMax);

    // Broadcast the health change event.
    OnHealthChanged.Broadcast();
}


void UStatComponent::UseTechniquePoints(float Amount)
{
	TechniquePoints -= Amount;
	TechniquePoints = FMath::Clamp(TechniquePoints, 0.f, MaxTechniquePoints);

	// Broadcast the event for Technique Points change.
	OnTechniquePointsChanged.Broadcast();
}

void UStatComponent::Heal(float Amount)
{
	Health += Amount;
	float HealthClampMax = bIsBoss ? MaxHealth : FMath::Min(MaxHealth, 10000.f);
	Health = FMath::Clamp(Health, 0.f, HealthClampMax);

	// Broadcast the event for Health change.
	OnHealthChanged.Broadcast();
}

void UStatComponent::ApplyStatModifier(ECombatStatType AffectedStat, float ModifierValue, EModifierType ModifierType, int32 DurationTurns)
{
    FActiveStatModifier NewModifier;
    NewModifier.AffectedStat = AffectedStat;
    NewModifier.ModifierValue = ModifierValue;
    NewModifier.ModifierType = ModifierType;
    NewModifier.RemainingTurns = DurationTurns;
    ActiveModifiers.Add(NewModifier);

    // Recalculate the effective stat immediately.
    RecalculateStat(AffectedStat);
}

void UStatComponent::DecrementStatModifiers()
{
    // It is best to iterate backwards when removing items.
    for (int32 i = ActiveModifiers.Num() - 1; i >= 0; i--)
    {
        ActiveModifiers[i].RemainingTurns--;
        if (ActiveModifiers[i].RemainingTurns <= 0)
        {
            ECombatStatType CombatStatType = ActiveModifiers[i].AffectedStat;
            ActiveModifiers.RemoveAt(i);
            RecalculateStat(CombatStatType);
        }
    }
}

void UStatComponent::RecalculateStat(ECombatStatType CombatStatType)
{
    float BaseValue = 0.f;
    switch (CombatStatType)
    {
    case ECombatStatType::PhysicalAttack:
        BaseValue = BasePhysicalAttack;
        break;
    case ECombatStatType::MagicalAttack:
        BaseValue = BaseMagicalAttack;
        break;
    case ECombatStatType::PhysicalDefense:
        BaseValue = BasePhysicalDefense;
        break;
    case ECombatStatType::MagicalDefense:
        BaseValue = BaseMagicalDefense;
        break;
    case ECombatStatType::Speed:
        BaseValue = BaseSpeed;
        break;
    default:
        return;
    }

    float PercentageSum = 0.f;
    float FlatSum = 0.f;

    for (const FActiveStatModifier& Modifier : ActiveModifiers)
    {
        if (Modifier.AffectedStat == CombatStatType)
        {
            if (Modifier.ModifierType == EModifierType::Percentage)
            {
                PercentageSum += Modifier.ModifierValue;
            }
            else // Flat
            {
                FlatSum += Modifier.ModifierValue;
            }
        }
    }

    float NewValue = BaseValue * (1.f + PercentageSum) + FlatSum;

    // Update the stat and broadcast event if needed.
    switch (CombatStatType)
    {
    case ECombatStatType::PhysicalAttack:
        PhysicalAttack = NewValue;
        OnPhysicalAttackChanged.Broadcast();
        break;
    case ECombatStatType::MagicalAttack:
        MagicalAttack = NewValue;
        OnMagicalAttackChanged.Broadcast();
        break;
    case ECombatStatType::PhysicalDefense:
        PhysicalDefense = NewValue;
        OnPhysicalDefenseChanged.Broadcast();
        break;
    case ECombatStatType::MagicalDefense:
        MagicalDefense = NewValue;
        OnMagicalDefenseChanged.Broadcast();
        break;
    case ECombatStatType::Speed:
        Speed = NewValue;
        OnSpeedChanged.Broadcast();
        break;
    default:
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("RecalculateStat - BaseSpeed: %f, PercentageSum: %f, FlatSum: %f, NewValue: %f"), BaseValue, PercentageSum, FlatSum, NewValue);

}
