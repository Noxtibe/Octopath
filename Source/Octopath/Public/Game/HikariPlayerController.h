#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HikariPlayerController.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;

/**
 * AHikariPlayerController
 *
 * Custom PlayerController for Octopath.
 * - In the base level, it uses GameOnly mode (no mouse cursor).
 * - During combat, calling EnableCombatInputMode() switches to GameAndUI mode,
 *   shows the mouse cursor, and enables UI interaction.
 */
UCLASS()
class OCTOPATH_API AHikariPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHikariPlayerController();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	// Protected variables
	/** Enhanced Input mapping context (assign in editor). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	/** Enhanced Input mapping context for combat (assign in editor). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* CombatMappingContext;

	/** Input action for moving (assign in editor). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	/** Input action for looking (assign in editor). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	// Protected functions
	/** Handles movement input. */
	UFUNCTION()
	void HandleMove(const struct FInputActionValue& Value);

	/** Handles look/rotation input. */
	UFUNCTION()
	void HandleLook(const FInputActionValue& Value);

public:
	// Public functions
	/**
	 * Enables combat input mode: switches to GameAndUI mode and shows the mouse cursor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void EnableCombatInputMode();

	/**
	 * Disables combat input mode: switches to GameOnly mode and hides the mouse cursor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DisableCombatInputMode();
};