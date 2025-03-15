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
}

void UStatComponent::ApplyDamage(float DamageAmount, bool bIsMagical)
{
	float Defense = bIsMagical ? MagicalDefense : PhysicalDefense;
	float EffectiveDamage = FMath::Max(DamageAmount - Defense, 1.f);

	// If defending, reduce the damage without resetting the defense bonus.
	if (bIsDefending)
	{
		EffectiveDamage *= (1.f - DefenseReductionPercentage);
		// Do NOT reset bIsDefending here so that the bonus remains active for the entire round.
	}

	Health -= EffectiveDamage;

	float HealthClampMax = bIsBoss ? MaxHealth : FMath::Min(MaxHealth, 10000.f);
	Health = FMath::Clamp(Health, 0.f, HealthClampMax);

	// Broadcast the event for Health change.
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
