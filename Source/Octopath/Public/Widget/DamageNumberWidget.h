#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DamageNumberWidget.generated.h"

/**
 * UDamageNumberWidget
 *
 * This widget displays a damage number above an entity.
 * It contains a text block to show the damage value and can play an appearance animation.
 * You can extend or customize this widget via Blueprint to adjust the animation and style.
 */
UCLASS()
class OCTOPATH_API UDamageNumberWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UDamageNumberWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Sets the damage value displayed by the widget.
	 *
	 * @param DamageValue - The damage value to display.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage Number")
	void SetDamageValue(int32 DamageValue);

	/**
	 * Plays the appearance animation for the damage number.
	 * (This animation can be defined in Blueprint and bound using BindWidgetAnim.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage Number")
	void PlayDamageAnimation();

protected:
	// Called when the widget is constructed.
	virtual void NativeConstruct() override;

	/** TextBlock bound from UMG for displaying the damage number */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DamageText;

	/** Optional widget animation for the damage number (bind in UMG Designer) */
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* DamageAnimation;
};
