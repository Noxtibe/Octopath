#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatTurnInfo.h"
#include "TurnBasedCombatComponent.generated.h"

// Forward declarations
class UTurnOrderWidget;
class UPlayerTurnMenuWidget;
class UPlayerStatsWidget;
class UUserWidget;
class UCanvasPanel;

/**
 * UTurnBasedCombatComponent
 *
 * Manages turn-based combat by assembling combatants (player and enemies),
 * sorting them by their "Speed" stat, updating the turn order UI, and handling turn transitions.
 * When it is the player's turn, an action menu is shown.
 * In Attack mode, target selection mode is activated automatically when hovering over an enemy.
 * The enemy indicator widget is displayed and its position is adjustable via editor-exposed offsets.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCTOPATH_API UTurnBasedCombatComponent : public UActorComponent
{
    GENERATED_BODY()

    // --------------------- Public Functions ---------------------
public:
    // Constructor
    UTurnBasedCombatComponent();

    // Overrides
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Starts the combat phase.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombat();

    // Proceeds to the next turn.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void NextTurn();

    // Ends the current round and reloads combat or returns to the base level.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndRound();

    // Updates the turn order UI.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateTurnOrderHUD();

    // Called when the player presses Attack.
    UFUNCTION()
    void OnPlayerAttack();

    // Called when the player chooses to flee.
    UFUNCTION()
    void OnPlayerFlee();

    // Sets the currently selected enemy and displays the enemy indicator widget.
    UFUNCTION(BlueprintCallable, Category = "Combat|Targeting")
    void SetSelectedEnemy(AActor* NewSelectedEnemy);

    // Updates the enemy indicator widget position on the screen.
    UFUNCTION(BlueprintCallable, Category = "Combat|Targeting")
    void UpdateEnemyIndicatorPosition();

    // Called when it is an enemy's turn; enemy performs its action.
    UFUNCTION(BlueprintCallable, Category = "Combat|Enemy")
    void OnEnemyTurn();

    // (No events defined)

// --------------------- Private Functions & Variables ---------------------
private:
    // Array of combatants (player and enemies)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> Combatants;

    // Current turn index.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    int32 CurrentTurnIndex;

    // Class of the Turn Order widget (derived from UTurnOrderWidget).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UTurnOrderWidget> TurnOrderWidgetClass;

    // Instance of the Turn Order widget.
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    UTurnOrderWidget* TurnOrderWidget;

    // Class of the player's action menu widget (derived from UPlayerTurnMenuWidget).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UPlayerTurnMenuWidget> PlayerTurnMenuWidgetClass;

    // Instance of the player's action menu widget.
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    UPlayerTurnMenuWidget* PlayerTurnMenuWidget;

    // Class of the player's stats widget (derived from UPlayerStatsWidget).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UPlayerStatsWidget> PlayerStatsWidgetClass;

    // Instance of the player's stats widget.
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    UPlayerStatsWidget* PlayerStatsWidget;

    // Original map name (used for fleeing).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FName OriginalMapName;

    // Currently selected enemy for target selection.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting", meta = (AllowPrivateAccess = "true"))
    AActor* SelectedEnemy;

    // Whether target selection mode is active.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting", meta = (AllowPrivateAccess = "true"))
    bool bIsSelectingTarget;

    // Whether the target is locked.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting", meta = (AllowPrivateAccess = "true"))
    bool bTargetLocked;

    // Class of the enemy indicator widget (e.g., a widget displaying a white dot).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> EnemyIndicatorWidgetClass;

    // Instance of the enemy indicator widget.
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    UUserWidget* CurrentEnemyIndicatorWidget;

    // Variable to detect left mouse button transitions.
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    bool bWasLeftMouseDown;

    // Instead of textures, use materials for the turn order icons.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TurnOrder", meta = (AllowPrivateAccess = "true"))
    UTexture2D* PlayerIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TurnOrder", meta = (AllowPrivateAccess = "true"))
    UTexture2D* EnemyIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TurnOrder", meta = (AllowPrivateAccess = "true"))
    UTexture2D* SelectedIconTexture;

    // Flag to indicate if the player fled.
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    bool bPlayerFled;

    // Map to store original materials for selected enemies.
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TMap<AActor*, UMaterialInterface*> OriginalMaterials;

    // Helper functions to apply/remove feedback.
    void ApplyFeedbackToEnemy(AActor* Enemy);
    void RemoveFeedbackFromEnemy(AActor* Enemy);

    // --------------------- Public Variables ---------------------
public:
    // Vertical offset in world coordinates to determine the base point above the enemy.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting")
    float IndicatorWorldVerticalOffset = 150.0f;

    // Additional offset in screen coordinates for fine adjustment of the widget position.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting")
    FVector2D IndicatorScreenOffset = FVector2D(0.0f, 0.0f);

    // Material to apply as feedback on the selected enemy.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback")
    UMaterialInterface* SelectedEnemyLightFunctionMaterial;
};
