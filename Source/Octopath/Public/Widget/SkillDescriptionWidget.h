#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "SkillDescriptionWidget.generated.h"

class UTextBlock;

/**
 * USkillDescriptionWidget
 *
 * This widget displays the description of a skill.
 * It is designed to be positioned below an ability button when hovered and selected.
 */
UCLASS(Blueprintable)
class OCTOPATH_API USkillDescriptionWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/** Sets the description text of the skill. */
	UFUNCTION(BlueprintCallable, Category = "Skill Description")
	void SetDescription(const FText& InDescription);

protected:
	/** Bindable widget reference for the text block displaying the skill description.
		Make sure this variable is bound to a TextBlock in your UMG designer. */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionText;
};
