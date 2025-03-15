// EnemyAbilityComponent.cpp

#include "Enemy/EnemyAbilityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/StatComponent.h"
#include "GameFramework/Character.h"

UEnemyAbilityComponent::UEnemyAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UEnemyAbilityComponent::ExecuteDefaultAttack()
{
	UE_LOG(LogTemp, Log, TEXT("EnemyAbilityComponent::ExecuteDefaultAttack - Called"));

	// Get the owner (the enemy)
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyAbilityComponent::ExecuteDefaultAttack - Owner is null"));
		return 0.f;
	}

	// Retrieve the enemy's StatComponent to get the PhysicalAttack value.
	UStatComponent* EnemyStat = Owner->FindComponentByClass<UStatComponent>();
	if (!EnemyStat)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyAbilityComponent::ExecuteDefaultAttack - StatComponent not found"));
		return 0.f;
	}

	// Use the enemy's PhysicalAttack value as the damage amount.
	float DamageAmount = EnemyStat->PhysicalAttack;
	UE_LOG(LogTemp, Log, TEXT("EnemyAbilityComponent::ExecuteDefaultAttack - DamageAmount calculated: %f"), DamageAmount);

	// Return the calculated damage (do not apply it here).
	return DamageAmount;
}
