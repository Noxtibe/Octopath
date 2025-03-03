#include "Enemy/EnemyAbilityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/StatComponent.h"
#include "GameFramework/Character.h"

UEnemyAbilityComponent::UEnemyAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyAbilityComponent::ExecuteDefaultAttack()
{
	// Get the owner (the enemy)
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Retrieve the enemy's StatComponent to get the PhysicalAttack value.
	UStatComponent* EnemyStat = Owner->FindComponentByClass<UStatComponent>();
	if (!EnemyStat)
	{
		return;
	}

	// Use the enemy's PhysicalAttack value as the damage amount.
	float DamageAmount = EnemyStat->PhysicalAttack;

	// Get the player character (assumed to be the target).
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		return;
	}

	// Retrieve the player's StatComponent.
	UStatComponent* PlayerStat = PlayerCharacter->FindComponentByClass<UStatComponent>();
	if (PlayerStat)
	{
		// Apply damage to the player (false indicates physical damage).
		PlayerStat->ApplyDamage(DamageAmount, false);
	}
}
