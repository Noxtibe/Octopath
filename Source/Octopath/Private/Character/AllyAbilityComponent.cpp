#include "Character/AllyAbilityComponent.h"
#include "Manager/StatComponent.h"

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

	// Retrieve the owner's StatComponent.
	if (UStatComponent* StatComp = Owner->FindComponentByClass<UStatComponent>())
	{
		// Return the PhysicalAttack stat as damage.
		return StatComp->PhysicalAttack;
	}

	return 0.f;
}
