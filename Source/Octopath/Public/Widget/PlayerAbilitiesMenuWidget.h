#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Manager/SkillData.h"
#include "PlayerAbilitiesMenuWidget.generated.h"

// Forward declarations
class UScrollBox;
class UMyCommonButton;
class UMyCommonButtonText;

/**
 * UPlayerAbilitiesMenuWidget
 *
 * Widget that displays the player's abilities (skills) for selection.
 * It uses a ScrollBox to contain the ability buttons and a CommonUI button for "Back".
 */
UCLASS(Blueprintable)
class OCTOPATH_API UPlayerAbilitiesMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	/** Populates the abilities menu by creating a button for each ability */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void PopulateAbilitiesMenu();

	/** Handler for when an ability button is clicked */
	UFUNCTION()
	void OnAbilityButtonClicked(UMyCommonButtonText* ClickedButton);

	/** Handler for when the Back button is clicked */
	UFUNCTION()
	void HandleBackButtonClicked(UMyCommonButton* Button);

	UFUNCTION()
	void OnAbilityButtonHovered(UMyCommonButtonText* HoveredButton);

	UFUNCTION()
	void OnAbilityButtonUnhovered(UMyCommonButtonText* UnhoveredButton);

public:

	/** Delegate to broadcast when an ability is selected */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilitySelected, USkillData*, SelectedSkill);
	UPROPERTY(BlueprintAssignable, Category = "Abilities")
	FOnAbilitySelected OnAbilitySelected;

	/** Delegate to broadcast when the Back button is pressed */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackPressed);
	UPROPERTY(BlueprintAssignable, Category = "Abilities")
	FOnBackPressed OnBackPressed;

public:

	/** Class of the ability button widget (assignable in the editor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UMyCommonButtonText> AbilityButtonClass;

	/** Bindable widget reference for the ScrollBox holding ability buttons (set in UMG Designer) */
	UPROPERTY(meta = (BindWidget))
	UScrollBox* AbilitiesScrollBox;

	/** Bindable widget reference for the Back button (Common UI button, set in UMG Designer) */
	UPROPERTY(meta = (BindWidget))
	UMyCommonButton* BackButton;

	/** Map to associate each created ability button with its corresponding skill */
	UPROPERTY()
	TMap<UMyCommonButtonText*, USkillData*> AbilityButtonMap;

	// Class of the description widget (assignable in the editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<class USkillDescriptionWidget> SkillDescriptionWidgetClass;

	// Currently displayed description widget
	UPROPERTY()
	USkillDescriptionWidget* CurrentSkillDescriptionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Description Positioning")
	float DescriptionVerticalOffset = 10.f;

	// Fixed position for the description widget in the viewport (modifiable dans l'�diteur)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Description Positioning")
	FVector2D DescriptionWidgetFixedPosition = FVector2D(300.f, 200.f);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Description Positioning")
	FVector2D DescriptionOffset = FVector2D(0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Description Positioning")
	FAnchors DescriptionAnchors = FAnchors(0.5f, 0.f, 0.5f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities|Description Positioning")
	FVector2D DescriptionAlignment = FVector2D(0.5f, 1.f);
};
