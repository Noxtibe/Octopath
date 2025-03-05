#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Widget/MyCommonButton.h"
#include "PlayerTurnMenuWidget.generated.h"

/**
 * A widget that appears during the player's turn to offer action choices.
 */
UCLASS(Blueprintable)
class OCTOPATH_API UPlayerTurnMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// Bindable widget references (set these in the UMG Designer)
	UPROPERTY(meta = (BindWidget))
	UMyCommonButton* AttackButton;

	UPROPERTY(meta = (BindWidget))
	UMyCommonButton* DefenseButton;

	UPROPERTY(meta = (BindWidget))
	UMyCommonButton* FleeButton;

	// Delegates to signal the selection of an action
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackSelected);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDefenseSelected);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFleeSelected);

	UPROPERTY(BlueprintAssignable, Category = "Actions")
	FOnAttackSelected OnAttackSelected;

	UPROPERTY(BlueprintAssignable, Category = "Actions")
	FOnFleeSelected OnDefenseSelected;

	UPROPERTY(BlueprintAssignable, Category = "Actions")
	FOnFleeSelected OnFleeSelected;

protected:
	// Override NativeConstruct to bind our button events
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleAttackClicked(UMyCommonButton* Button);

	UFUNCTION()
	void HandleDefenseClicked(UMyCommonButton* Button);

	UFUNCTION()
	void HandleFleeClicked(UMyCommonButton* Button);
};
