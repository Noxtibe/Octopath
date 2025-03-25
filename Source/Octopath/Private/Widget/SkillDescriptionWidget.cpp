#include "Widget/SkillDescriptionWidget.h"
#include "Components/TextBlock.h"

void USkillDescriptionWidget::SetDescription(const FText& InDescription)
{
	if (DescriptionText)
	{
		DescriptionText->SetText(InDescription);
	}
}
