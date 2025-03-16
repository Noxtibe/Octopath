#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "MyCommonButton.generated.h"

// Declare a dynamic multicast delegate that passes the clicked button as a parameter
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyCommonButtonClicked, UMyCommonButton*, Button);

/**
 * Custom button class derived from UCommonButtonBase that broadcasts a dynamic delegate when clicked.
 */
UCLASS(Blueprintable)
class OCTOPATH_API UMyCommonButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UMyCommonButton();
	// Override the native click handler to broadcast our dynamic delegate
	virtual void NativeOnClicked() override;

	// Dynamic delegate that can be bound via AddDynamic in other classes (such as your menu widget)
	UPROPERTY(BlueprintAssignable, Category = "Common Button")
	FMyCommonButtonClicked OnMyClicked;
};
