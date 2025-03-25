#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "MyCommonButtonText.generated.h"

// Declare a dynamic multicast delegate that passes the clicked button as a parameter
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyCommonButtonTextClicked, UMyCommonButtonText*, Button);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonHovered, UMyCommonButtonText*, Button);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonUnhovered, UMyCommonButtonText*, Button);

UCLASS(Blueprintable)
class OCTOPATH_API UMyCommonButtonText : public UCommonButtonBase
{
    GENERATED_BODY()

public:
    // Override the native click handler to broadcast our dynamic delegate
    virtual void NativeOnClicked() override;

    // Override mouse enter/leave events to broadcast hover delegates
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

public:
    /** Sets the button's displayed text */
    UFUNCTION(BlueprintCallable, Category = "Button")
    void SetButtonText(const FText& InText);

public:
    // Dynamic delegate that can be bound via AddDynamic in other classes (such as your menu widget)
    UPROPERTY(BlueprintAssignable, Category = "Common Button")
    FMyCommonButtonTextClicked OnMyTextClicked;

    UPROPERTY(BlueprintAssignable, Category = "Button|Events")
    FOnButtonHovered OnHovered;

    UPROPERTY(BlueprintAssignable, Category = "Button|Events")
    FOnButtonUnhovered OnUnhovered;

public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ButtonTextBlock;
};
