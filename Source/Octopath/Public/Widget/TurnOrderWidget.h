#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Combat/CombatTurnInfo.h"
#include "TurnOrderWidget.generated.h"

class UHorizontalBox;
class UImage;

/**
 * UTurnOrderWidget
 *
 * Displays the turn order of combatants in two bars: the current turn order and the next-turn order.
 * When a combatant is selected, its icon is replaced by the selected icon (yellow).
 */
UCLASS()
class OCTOPATH_API UTurnOrderWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // Horizontal box for current turn order icons.
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* TurnOrderBox;

    // Horizontal box for next-turn order icons.
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* NextTurnOrderBox;

    /**
     * Updates both turn order displays.
     * @param CombatantInfos A sorted array of combatant info.
     * @param SelectedEnemy The currently selected enemy (to highlight its icon).
     */
    UFUNCTION(BlueprintCallable, Category = "Turn Order")
    void UpdateTurnOrder(const TArray<FCombatantTurnInfo>& CurrentTurnInfos, const TArray<FCombatantTurnInfo>& FullTurnInfos, AActor* SelectedEnemy = nullptr);


    // Texture to use when a combatant is selected (highlight texture, e.g. yellow).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Order")
    UTexture2D* SelectedIconTexture;
};
