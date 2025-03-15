#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EnemyIndicatorWidget.generated.h"

// Forward declarations
class UImage;
class UTextBlock;

/**
 * UEnemyIndicatorWidget
 *
 * This widget displays a white dot indicator and the enemy's name next to it.
 * It inherits from UCommonActivatableWidget to leverage common UI functionalities.
 */
UCLASS()
class OCTOPATH_API UEnemyIndicatorWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// Public functions
	/**
	 * Sets the enemy name text.
	 *
	 * @param NewName - The new text to display as the enemy's name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Indicator")
	void SetEnemyName(const FText& NewName);

protected:
	// Protected variables
	/** The image representing the white dot indicator. */
	UPROPERTY(meta = (BindWidget))
	UImage* IndicatorImage;

	/** The text block that displays the enemy's name. */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EnemyNameText;
};
