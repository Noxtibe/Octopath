#include "Widget/MyCommonButton.h"

UMyCommonButton::UMyCommonButton()
{
	// You can initialize properties here if needed.
}

void UMyCommonButton::NativeOnClicked()
{
	// Call the base implementation (if any behavior is required)
	Super::NativeOnClicked();

	// Broadcast our dynamic delegate to notify listeners that this button was clicked
	OnMyClicked.Broadcast(this);
}
