#include "Widget/EnemyIndicatorWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEnemyIndicatorWidget::SetEnemyName(const FText& NewName)
{
    if (EnemyNameText)
    {
        EnemyNameText->SetText(NewName);
    }
}
