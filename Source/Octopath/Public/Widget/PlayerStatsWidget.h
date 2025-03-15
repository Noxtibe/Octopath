#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PlayerStatsWidget.generated.h"

// Forward declarations
class UTextBlock;
class UProgressBar;
class UStatComponent;

/**
 * UPlayerStatsWidget
 *
 * This widget displays the player's stats (name, health, and technique points) and updates its display
 * only when needed, using functions triggered by events.
 */
UCLASS()
class OCTOPATH_API UPlayerStatsWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// Public Variables
	/** Bindable widget variable: Player name text (assign in Blueprint) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	/** Bindable widget variable: Health text (assign in Blueprint) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	/** Bindable widget variable: Health progress bar (assign in Blueprint) */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	/** Bindable widget variable: Technique points text (assign in Blueprint) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TechniqueText;

	/** Bindable widget variable: Technique points progress bar (assign in Blueprint) */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* TechniqueBar;

protected:
	// Protected Functions
	virtual void NativeConstruct() override;

	/** Retrieves all the necessary references (only once) */
	void GetReferences();

	/** Updates the player's name display */
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void UpdatePlayerName();

	/** Updates the player's health display */
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void UpdateHealth();

	/** Updates the player's technique points display */
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void UpdateTechniquePoints();

private:
	// Private Variables
	/** Cached reference to the player's stat component */
	UPROPERTY()
	UStatComponent* PlayerStatComp;
};
