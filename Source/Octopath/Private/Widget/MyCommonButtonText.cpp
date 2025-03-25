#include "Widget/MyCommonButtonText.h"
#include "Engine/Engine.h"

void UMyCommonButtonText::NativeOnClicked()
{
    // Call the base implementation (if any behavior is required)
    Super::NativeOnClicked();

    // Broadcast our dynamic delegate to notify listeners that this button was clicked
    OnMyTextClicked.Broadcast(this);
}

void UMyCommonButtonText::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    // Broadcast the hover event
    OnHovered.Broadcast(this);
    UE_LOG(LogTemp, Log, TEXT("MyCommonButtonText::NativeOnMouseEnter: %s"), *GetName());
}

void UMyCommonButtonText::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    // Broadcast the unhover event
    OnUnhovered.Broadcast(this);
    UE_LOG(LogTemp, Log, TEXT("MyCommonButtonText::NativeOnMouseLeave: %s"), *GetName());
}

void UMyCommonButtonText::SetButtonText(const FText& InText)
{
    if (ButtonTextBlock)
    {
        ButtonTextBlock->SetText(InText);
    }
}