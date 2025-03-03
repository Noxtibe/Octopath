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
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatComponent::ApplyDamage(float DamageAmount, bool bIsMagical)
{
	// Choose the appropriate defense value.
	float Defense = bIsMagical ? MagicalDefense : PhysicalDefense;

	// Calculate effective damage; ensure at least 1 damage is applied.
	float EffectiveDamage = FMath::Max(DamageAmount - Defense, 1.f);

	Health -= EffectiveDamage;

	// Determine the maximum health clamp based on whether the actor is a boss.
	float HealthClampMax = bIsBoss ? MaxHealth : FMath::Min(MaxHealth, 10000.f);
	Health = FMath::Clamp(Health, 0.f, HealthClampMax);
}

void UStatComponent::UseTechniquePoints(float Amount)
{
	TechniquePoints -= Amount;
	TechniquePoints = FMath::Clamp(TechniquePoints, 0.f, MaxTechniquePoints);
}

void UStatComponent::Heal(float Amount)
{
	Health += Amount;
	float HealthClampMax = bIsBoss ? MaxHealth : FMath::Min(MaxHealth, 10000.f);
	Health = FMath::Clamp(Health, 0.f, HealthClampMax);
}
