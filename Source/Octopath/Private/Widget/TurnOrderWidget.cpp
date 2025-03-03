#include "Widget/TurnOrderWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"

void UTurnOrderWidget::UpdateTurnOrder(const TArray<FCombatantTurnInfo>& CombatantInfos, AActor* SelectedEnemy)
{
    // V�rifier que les conteneurs sont valides.
    if (!TurnOrderBox || !NextTurnOrderBox)
    {
        return;
    }

    // Vider les conteneurs existants.
    TurnOrderBox->ClearChildren();
    NextTurnOrderBox->ClearChildren();

    // Boucle pour remplir la barre d'ordre de tour actuelle.
    for (const FCombatantTurnInfo& Info : CombatantInfos)
    {
        UImage* IconImage = NewObject<UImage>(this);
        if (!IconImage)
        {
            continue;
        }

        // Si ce combattant est s�lectionn� et que SelectedIconTexture est d�fini, utiliser SelectedIconTexture ; sinon utiliser Info.Icon.
        UTexture2D* IconToUse = nullptr;
        if (Info.Combatant == SelectedEnemy && SelectedIconTexture)
        {
            IconToUse = SelectedIconTexture;
        }
        else
        {
            IconToUse = Info.Icon;
        }

        if (IconToUse)
        {
            IconImage->SetBrushFromTexture(IconToUse);
        }

        // Appliquer une teinte par d�faut (blanc).
        IconImage->SetColorAndOpacity(FLinearColor::White);

        // D�finir la taille souhait�e pour les ic�nes de la barre actuelle.
        IconImage->SetDesiredSizeOverride(FVector2D(64.f, 64.f));
        if (UHorizontalBoxSlot* NewSlot = TurnOrderBox->AddChildToHorizontalBox(IconImage))
        {
            NewSlot->SetPadding(FMargin(5.f));
        }
    }

    // Boucle pour remplir la barre de prochain tour.
    for (const FCombatantTurnInfo& Info : CombatantInfos)
    {
        UImage* IconImage = NewObject<UImage>(this);
        if (!IconImage)
        {
            continue;
        }

        UTexture2D* IconToUse = nullptr;
        if (Info.Combatant == SelectedEnemy && SelectedIconTexture)
        {
            IconToUse = SelectedIconTexture;
        }
        else
        {
            IconToUse = Info.Icon;
        }

        if (IconToUse)
        {
            IconImage->SetBrushFromTexture(IconToUse);
        }

        IconImage->SetColorAndOpacity(FLinearColor::White);

        // Pour la barre du prochain tour, on utilise une taille l�g�rement plus petite.
        IconImage->SetDesiredSizeOverride(FVector2D(48.f, 48.f));
        if (UHorizontalBoxSlot* NewSlot = NextTurnOrderBox->AddChildToHorizontalBox(IconImage))
        {
            NewSlot->SetPadding(FMargin(3.f));
        }
    }
}
