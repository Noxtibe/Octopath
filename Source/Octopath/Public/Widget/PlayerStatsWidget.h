#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PlayerStatsWidget.generated.h"

class UTextBlock;
class UProgressBar;

/**
 * PlayerStatsWidget
 *
 * This widget displays the player's stats:
 * - Player name.
 * - Health as numeric values (e.g., "500/500") and a green health bar.
 * - Mana as numeric values and a blue mana bar.
 *
 * All logic for updating the UI is handled in C++ via NativeTick.
 */
UCLASS()
class OCTOPATH_API UPlayerStatsWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    // Bindable widget variables - assign these in your Blueprint.

    // Text block to display the player's name.
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    // Text block to display the player's health (e.g., "500/500").
    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    // Progress bar for the player's health (should be colored green in the designer).
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    // Text block to display the player's mana (e.g., "100/100").
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ManaText;

    // Progress bar for the player's mana (should be colored blue in the designer).
    UPROPERTY(meta = (BindWidget))
    UProgressBar* ManaBar;

protected:
    // Called every frame to update the widget.
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
