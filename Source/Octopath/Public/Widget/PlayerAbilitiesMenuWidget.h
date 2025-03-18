#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Manager/SkillData.h"
#include "PlayerAbilitiesMenuWidget.generated.h"

class UVerticalBox;
class UMyCommonButtonText;

/**
 * UPlayerAbilitiesMenuWidget
 *
 * Widget that displays the player's abilities (skills) for selection.
 */
UCLASS(Blueprintable)
class OCTOPATH_API UPlayerAbilitiesMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/** Populates the abilities menu by creating a button for each ability */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void PopulateAbilitiesMenu();

	/** Delegate to broadcast when an ability is selected */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilitySelected, USkillData*, SelectedSkill);
	UPROPERTY(BlueprintAssignable, Category = "Abilities")
	FOnAbilitySelected OnAbilitySelected;

	/** Class of the ability button widget (assignable in the editor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UMyCommonButtonText> AbilityButtonClass;

protected:
	/** Bindable widget reference for the container holding ability buttons (set in UMG Designer) */
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* AbilitiesListBox;

	/** Map to associate each created button with its corresponding skill */
	UPROPERTY()
	TMap<UMyCommonButtonText*, USkillData*> AbilityButtonMap;

	/** Handler for when an ability button is clicked */
	UFUNCTION()
	void OnAbilityButtonClicked(UMyCommonButtonText* ClickedButton);
};
