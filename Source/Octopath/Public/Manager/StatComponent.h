#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"

/**
 * UStatComponent
 *
 * This component manages various stats such as Health, Technique Points, Physical/Magical Defense,
 * Physical/Magical Attack, and Speed.
 *
 * - Health (MaxHealth and Health) is clamped between 0 and 10000 for non-boss characters.
 *   If bIsBoss is true, MaxHealth is not capped at 10000 and can be set to a higher value.
 * - Technique Points, Defense, Speed, and both Attack stats are clamped between 0 and 1000.
 *
 * Attach this component to any actor (player or enemy) to provide consistent stat management.
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Constructor
	UStatComponent();

protected:
	virtual void BeginPlay() override;

public:
	// --- Health Stats ---
	// Maximum Health Points.
	// For non-boss characters, this is clamped to a maximum of 10000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Health")
	float MaxHealth;

	// Current Health Points.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Health")
	float Health;

	// --- Technique Stats ---
	// Maximum Technique Points (mana), clamped between 0 and 1000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Technique", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MaxTechniquePoints;

	// Current Technique Points (mana), clamped between 0 and 1000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Technique", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float TechniquePoints;

	// --- Defense Stats ---
	// Physical Defense, clamped between 0 and 1000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Defense", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float PhysicalDefense;

	// Magical Defense, clamped between 0 and 1000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Defense", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MagicalDefense;

	// --- Attack Stats ---
	// Physical Attack, clamped between 0 and 1000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Attack", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float PhysicalAttack;

	// Magical Attack, clamped between 0 and 1000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Attack", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MagicalAttack;

	// --- Other Stats ---
	// Speed value, clamped between 0 and 1000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Other", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float Speed;

	// --- Boss Flag ---
	// If true, the actor is considered a boss and MaxHealth is not capped at 10000.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bIsBoss;

	// --- Functions to Modify Stats ---

	/**
	 * Applies damage to the actor, taking into account defense values.
	 *
	 * @param DamageAmount - The base damage to apply.
	 * @param bIsMagical - If true, use MagicalDefense; otherwise, use PhysicalDefense.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyDamage(float DamageAmount, bool bIsMagical = false);

	/**
	 * Consumes a specified amount of technique points.
	 *
	 * @param Amount - The amount of technique points to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void UseTechniquePoints(float Amount);

	/**
	 * Heals the actor by a specified amount, without exceeding MaxHealth.
	 *
	 * @param Amount - The amount of health to restore.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void Heal(float Amount);
};
