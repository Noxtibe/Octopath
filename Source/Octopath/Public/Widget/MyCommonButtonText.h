#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "MyCommonButtonText.generated.h"

// Declare a dynamic multicast delegate that passes the clicked button as a parameter
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyCommonButtonTextClicked, UMyCommonButtonText*, Button);

UCLASS(Blueprintable)
class OCTOPATH_API UMyCommonButtonText : public UCommonButtonBase
{
    GENERATED_BODY()

public:
    // Override the native click handler to broadcast our dynamic delegate
    virtual void NativeOnClicked() override;

public:
    /** Sets the button's displayed text */
    UFUNCTION(BlueprintCallable, Category = "Button")
    void SetButtonText(const FText& InText);

public:
    // Dynamic delegate that can be bound via AddDynamic in other classes (such as your menu widget)
    UPROPERTY(BlueprintAssignable, Category = "Common Button")
    FMyCommonButtonTextClicked OnMyTextClicked;

public:
    // BindWidget: Associez ce TextBlock dans votre Blueprint dérivé de UMyCommonButtonText.
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ButtonTextBlock;
};
