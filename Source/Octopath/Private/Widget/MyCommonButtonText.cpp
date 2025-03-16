#include "Widget/MyCommonButtonText.h"

void UMyCommonButtonText::NativeOnClicked()
{
	// Call the base implementation (if any behavior is required)
	Super::NativeOnClicked();

	// Broadcast our dynamic delegate to notify listeners that this button was clicked
	OnMyTextClicked.Broadcast(this);
}

void UMyCommonButtonText::SetButtonText(const FText& InText)
{
    if (ButtonTextBlock)
    {
        ButtonTextBlock->SetText(InText);
    }
}