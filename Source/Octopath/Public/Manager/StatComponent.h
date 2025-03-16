#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Manager/SkillData.h"
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

 // Delegate for when Health changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChanged);
// Delegate for when MaxHealth changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMaxHealthChanged);

// Delegate for when Technique Points change.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTechniquePointsChanged);
// Delegate for when Max Technique Points change.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMaxTechniquePointsChanged);

// Delegate for when Physical Defense changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhysicalDefenseChanged);
// Delegate for when Magical Defense changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagicalDefenseChanged);

// Delegate for when Physical Attack changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhysicalAttackChanged);
// Delegate for when Magical Attack changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagicalAttackChanged);

// Delegate for when Speed changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpeedChanged);

/**
 * Structure to hold active stat modifiers (buffs or debuffs) applied to a stat.
 */
USTRUCT(BlueprintType)
struct FActiveStatModifier
{
	GENERATED_BODY()

	/** The stat that is affected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	ECombatStatType AffectedStat;

	/** The modifier value (e.g., 0.2 for +20% or -0.2 for -20%, or a flat value) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float ModifierValue;

	/** Defines if the modifier is a percentage or a flat addition/subtraction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	EModifierType ModifierType;

	/** Number of turns remaining for this modifier effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	int32 RemainingTurns;
};

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatComponent();
	virtual void BeginPlay() override;

public:
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

	// --- Functions for Buffs and Debuffs ---

	/**
	 * Applies a stat modifier (buff or debuff) for a specified number of turns.
	 *
	 * @param AffectedStat - The stat to modify.
	 * @param ModifierValue - The modification value.
	 * @param ModifierType - Whether the modifier is percentage-based or flat.
	 * @param DurationTurns - Duration of the effect in turns.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyStatModifier(ECombatStatType AffectedStat, float ModifierValue, EModifierType ModifierType, int32 DurationTurns);

	/**
	 * Decrements the duration of all active stat modifiers.
	 * This function should be called at the end of each turn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void DecrementStatModifiers();

protected:
	/**
	 * Recalculates the effective value of a given stat based on its base value and active modifiers.
	 * @param StatType - The stat type to recalculate.
	 */
	void RecalculateStat(ECombatStatType CombatStatType);

protected:
	// --- Base Stats (for recalculation) ---
	// These variables store the original stat values.
	float BasePhysicalAttack;
	float BaseMagicalAttack;
	float BasePhysicalDefense;
	float BaseMagicalDefense;
	float BaseSpeed;

	// Array to store active stat modifiers (buffs/debuffs)
	UPROPERTY()
	TArray<FActiveStatModifier> ActiveModifiers;

public:
	// --- Delegates for each stat ---
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnMaxHealthChanged OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnTechniquePointsChanged OnTechniquePointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnMaxTechniquePointsChanged OnMaxTechniquePointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnPhysicalDefenseChanged OnPhysicalDefenseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnMagicalDefenseChanged OnMagicalDefenseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnPhysicalAttackChanged OnPhysicalAttackChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnMagicalAttackChanged OnMagicalAttackChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnSpeedChanged OnSpeedChanged;

public:
	// --- General Stat Properties ---
	/** Name of the entity (used in the UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|General")
	FText EntityName;

	// --- Health Stats ---
	/** Maximum Health Points (capped to 10000 for non-boss characters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Health")
	float MaxHealth;

	/** Current Health Points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Health")
	float Health;

	// --- Technique Stats ---
	/** Maximum Technique Points, clamped between 0 and 1000 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Technique", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MaxTechniquePoints;

	/** Current Technique Points, clamped between 0 and 1000 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Technique", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float TechniquePoints;

	// --- Defense Stats ---
	/** Physical Defense, clamped between 0 and 1000 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Defense", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float PhysicalDefense;

	/** Magical Defense, clamped between 0 and 1000 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Defense", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MagicalDefense;

	// --- Attack Stats ---
	/** Physical Attack, clamped between 0 and 1000 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Attack", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float PhysicalAttack;

	/** Magical Attack, clamped between 0 and 1000 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Attack", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MagicalAttack;

	// --- Other Stats ---
	/** Speed, clamped between 0 and 1000 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Other", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float Speed;

	// --- Boss Flag ---
	/** If true, the actor is considered a boss and MaxHealth is not capped at 10000 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bIsBoss;

	// --- Additional Defense Variables ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Defense")
	bool bIsDefending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Defense")
	float DefenseReductionPercentage;
};