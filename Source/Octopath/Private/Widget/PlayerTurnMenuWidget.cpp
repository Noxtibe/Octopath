#include "Widget/PlayerTurnMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UPlayerTurnMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind the OnMyClicked delegate from our custom buttons to our handler functions
	if (AttackButton)
	{
		AttackButton->OnMyClicked.AddDynamic(this, &UPlayerTurnMenuWidget::HandleAttackClicked);
	}
	if (DefenseButton)
	{
		DefenseButton->OnMyClicked.AddDynamic(this, &UPlayerTurnMenuWidget::HandleDefenseClicked);
	}
	if (FleeButton)
	{
		FleeButton->OnMyClicked.AddDynamic(this, &UPlayerTurnMenuWidget::HandleFleeClicked);
	}
}

void UPlayerTurnMenuWidget::HandleAttackClicked(UMyCommonButton* Button)
{
	// Broadcast that the Attack option was selected
	OnAttackSelected.Broadcast();
}

void UPlayerTurnMenuWidget::HandleDefenseClicked(UMyCommonButton* Button)
{
	// Broadcast that the Defense option was selected
	OnDefenseSelected.Broadcast();
}

void UPlayerTurnMenuWidget::HandleFleeClicked(UMyCommonButton* Button)
{
	// Broadcast that the Flee option was selected
	OnFleeSelected.Broadcast();
}
