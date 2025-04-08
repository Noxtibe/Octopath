#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillData.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Physical    UMETA(DisplayName = "Physical"),
    Magical     UMETA(DisplayName = "Magical")
};

UENUM(BlueprintType)
enum class ETargetMode : uint8
{
    Single      UMETA(DisplayName = "Single"),
    Multiple    UMETA(DisplayName = "Multiple"),
    All         UMETA(DisplayName = "All"),
    Random      UMETA(DisplayName = "Random")
};

UENUM(BlueprintType)
enum class ETargetType : uint8
{
    Ally        UMETA(DisplayName = "Ally"),
    Enemy       UMETA(DisplayName = "Enemy"),
    Self        UMETA(DisplayName = "Self")
};

UENUM(BlueprintType)
enum class EAbilityCategory : uint8
{
    Offensive   UMETA(DisplayName = "Offensive"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Buff        UMETA(DisplayName = "Buff"),
    Debuff      UMETA(DisplayName = "Debuff"),
    Heal        UMETA(DisplayName = "Heal"),
    Utility     UMETA(DisplayName = "Utility")
};

UENUM(BlueprintType)
enum class ECombatStatType : uint8
{
    None            UMETA(DisplayName = "None"),
    PhysicalAttack  UMETA(DisplayName = "Physical Attack"),
    MagicalAttack   UMETA(DisplayName = "Magical Attack"),
    PhysicalDefense UMETA(DisplayName = "Physical Defense"),
    MagicalDefense  UMETA(DisplayName = "Magical Defense"),
    Speed           UMETA(DisplayName = "Speed")
};

UENUM(BlueprintType)
enum class EModifierType : uint8
{
    Percentage    UMETA(DisplayName = "Percentage"),
    Flat          UMETA(DisplayName = "Flat")
};

/**
 * USkillData
 *
 * Data asset representing a skill/ability.
 * It can be configured in the editor with:
 * - Name and Description
 * - Base Damage (or effect magnitude)
 * - Technique Points cost
 * - Attack type (Physical/Magical)
 * - Targeting mode (Single, Multiple, Random)
 * - Allowed target type (Ally, Enemy, Self)
 * - Ability category (Offensive, Defensive, Buff, Debuff, Heal, etc.)
 */
UCLASS(BlueprintType)
class OCTOPATH_API USkillData : public UDataAsset
{
    GENERATED_BODY()

public :

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
    /** Name of the skill */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    FText SkillName;

    /** Description of the skill */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (MultiLine = true))
    FText Description;

    /** Base damage or effect magnitude (for offensive or healing skills) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "0.0"))
    float Damage;

    /** Technique Points cost */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "0.0"))
    float TechniqueCost;

    /** Type of attack: Physical or Magical */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    EAttackType AttackType;

    /** Targeting mode: Single, Multiple, or Random */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    ETargetMode TargetMode;

    /** Allowed target type for this skill */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    ETargetType TargetType;

    // Casting time (in seconds) for the ability.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float CastingTime = 1.0f;

    /** Category of the ability: Offensive, Defensive, Buff, Debuff, Heal, etc. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    EAbilityCategory AbilityCategory;

    // --- Buff/Debuff Specific ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Modifier")
    ECombatStatType AffectedStat;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Modifier", meta = (EditCondition = "bUseModifier", DisableEditConditionToggle = "true"))
    float ModifierValue;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Modifier", meta = (EditCondition = "bUseModifier", DisableEditConditionToggle = "true"))
    EModifierType ModifierType;

    // Duration in number of turns
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Modifier", meta = (ClampMin = "0", EditCondition = "bUseModifier", DisableEditConditionToggle = "true"))
    int32 Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bUseModifier;

    // VFX launch on the target at the end of the timeline
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Effects")
    UNiagaraSystem* ImpactVFX;

    // Offset to add to the target's location when spawning the ability FX
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Effects")
    FVector FXSpawnOffset = FVector(0.f, 0.f, 0.f);

    // VFX launch on the caster during the timeline
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Effects")
    UNiagaraSystem* CastingVFX;

    // Offset to add to the caster’s location when spawning the casting VFX.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Effects")
    FVector CastingFXOffset = FVector(0.f, 0.f, 0.f);
};
