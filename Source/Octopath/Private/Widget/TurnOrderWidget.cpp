#include "Widget/TurnOrderWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"

void UTurnOrderWidget::UpdateTurnOrder(const TArray<FCombatantTurnInfo>& CurrentTurnInfos, const TArray<FCombatantTurnInfo>& FullTurnInfos, AActor* SelectedEnemy)
{
    if (!TurnOrderBox || !NextTurnOrderBox)
    {
        return;
    }

    // Vider les conteneurs existants.
    TurnOrderBox->ClearChildren();
    NextTurnOrderBox->ClearChildren();

    // Remplir la barre du round en cours (grande taille) avec uniquement les combattants qui n'ont pas encore joué.
    for (const FCombatantTurnInfo& Info : CurrentTurnInfos)
    {
        UImage* IconImage = NewObject<UImage>(this);
        if (!IconImage)
            continue;

        UTexture2D* IconToUse = (Info.Combatant == SelectedEnemy && SelectedIconTexture) ? SelectedIconTexture : Info.Icon;
        if (IconToUse)
        {
            IconImage->SetBrushFromTexture(IconToUse);
        }
        IconImage->SetColorAndOpacity(FLinearColor::White);
        IconImage->SetDesiredSizeOverride(FVector2D(64.f, 64.f));
        if (UHorizontalBoxSlot* NewSlot = TurnOrderBox->AddChildToHorizontalBox(IconImage))
        {
            NewSlot->SetPadding(FMargin(5.f));
        }
    }

    // Remplir la barre du prochain tour (taille réduite) avec la liste complète.
    for (const FCombatantTurnInfo& Info : FullTurnInfos)
    {
        UImage* IconImage = NewObject<UImage>(this);
        if (!IconImage)
            continue;

        UTexture2D* IconToUse = (Info.Combatant == SelectedEnemy && SelectedIconTexture) ? SelectedIconTexture : Info.Icon;
        if (IconToUse)
        {
            IconImage->SetBrushFromTexture(IconToUse);
        }
        IconImage->SetColorAndOpacity(FLinearColor::White);
        IconImage->SetDesiredSizeOverride(FVector2D(48.f, 48.f));
        if (UHorizontalBoxSlot* NewSlot = NextTurnOrderBox->AddChildToHorizontalBox(IconImage))
        {
            NewSlot->SetPadding(FMargin(3.f));
        }
    }
}