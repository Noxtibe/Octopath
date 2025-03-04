#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PlayerStatsWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UStatComponent;

/**
 * UPlayerStatsWidget
 *
 * This widget displays the player's stats (name, health, techniques) and updates its display
 * only when needed, using the UpdateStats() function triggered by events.
 */
UCLASS()
class OCTOPATH_API UPlayerStatsWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    // Bindable widget variables - assign these in your Blueprint.
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TechniqueText;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* TechniqueBar;

protected:
    // Called when the widget is constructed.
    virtual void NativeConstruct() override;

    // Retrieves all the necessary references (only once).
    void GetReferences();

    UFUNCTION(BlueprintCallable, Category = "Player Stats")
    void UpdatePlayerName();

    UFUNCTION(BlueprintCallable, Category = "Player Stats")
    void UpdateHealth();

    UFUNCTION(BlueprintCallable, Category = "Player Stats")
    void UpdateTechniquePoints();

private:
    // Cached reference to the player's stat component.
    UPROPERTY()
    UStatComponent* PlayerStatComp;
};
