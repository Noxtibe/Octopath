#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Widget/MyCommonButton.h"
#include "PlayerTurnMenuWidget.generated.h"

/**
 * UPlayerTurnMenuWidget
 *
 * A widget that appears during the player's turn to offer action choices.
 */
UCLASS(Blueprintable)
class OCTOPATH_API UPlayerTurnMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// Public Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackSelected);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilitiesSelected);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDefenseSelected);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFleeSelected);

	UPROPERTY(BlueprintAssignable, Category = "Actions")
	FOnAttackSelected OnAttackSelected;

	UPROPERTY(BlueprintAssignable, Category = "Actions")
	FOnAbilitiesSelected OnAbilitiesSelected;

	UPROPERTY(BlueprintAssignable, Category = "Actions")
	FOnDefenseSelected OnDefenseSelected;

	UPROPERTY(BlueprintAssignable, Category = "Actions")
	FOnFleeSelected OnFleeSelected;

public:
	// Public Variables
	/** Bindable widget reference for the Attack button (set in UMG Designer) */
	UPROPERTY(meta = (BindWidget))
	UMyCommonButton* AttackButton;

	/** Bindable widget reference for the Abilities button (set in UMG Designer) */
	UPROPERTY(meta = (BindWidget))
	UMyCommonButton* AbilitiesButton;

	/** Bindable widget reference for the Defense button (set in UMG Designer) */
	UPROPERTY(meta = (BindWidget))
	UMyCommonButton* DefenseButton;

	/** Bindable widget reference for the Flee button (set in UMG Designer) */
	UPROPERTY(meta = (BindWidget))
	UMyCommonButton* FleeButton;

protected:
	// Protected Functions
	/** Overrides the widget construction to bind button events */
	virtual void NativeConstruct() override;

	/** Handles the event when the Abilities button is clicked */
	UFUNCTION()
	void HandleAttackClicked(UMyCommonButton* Button);

	/** Handles the event when the Attack button is clicked */
	UFUNCTION()
	void HandleAbilitiesClicked(UMyCommonButton* Button);

	/** Handles the event when the Defense button is clicked */
	UFUNCTION()
	void HandleDefenseClicked(UMyCommonButton* Button);

	/** Handles the event when the Flee button is clicked */
	UFUNCTION()
	void HandleFleeClicked(UMyCommonButton* Button);
};
