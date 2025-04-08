#include "Manager/SkillData.h"

#if WITH_EDITOR
void USkillData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    // Si la propriété modifiée est AffectedStat, alors on met à jour bUseModifier
    if (PropertyName == GET_MEMBER_NAME_CHECKED(USkillData, AffectedStat))
    {
        // Si AffectedStat est None, on désactive automatiquement le modificateur
        bUseModifier = (AffectedStat != ECombatStatType::None);
    }
}
#endif
