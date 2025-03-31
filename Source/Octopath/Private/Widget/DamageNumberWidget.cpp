#include "Widget/DamageNumberWidget.h"
#include "Components/TextBlock.h"

UDamageNumberWidget::UDamageNumberWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDamageNumberWidget::SetDamageValue(int32 DamageValue)
{
	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(DamageValue));
	}
}

void UDamageNumberWidget::PlayDamageAnimation()
{
	if (DamageAnimation)
	{
		PlayAnimation(DamageAnimation);
	}
}