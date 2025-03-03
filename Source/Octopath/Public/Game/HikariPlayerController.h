#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HikariPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * Custom PlayerController for Octopath.
 * - In the base level, it uses GameOnly mode (no mouse cursor).
 * - During combat, calling EnableCombatInputMode() switches to GameAndUI mode,
 *   shows the mouse cursor and enables UI interaction.
 */
UCLASS()
class OCTOPATH_API AHikariPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHikariPlayerController();

protected:
	// Called when the game starts.
	virtual void BeginPlay() override;

	// Setup input bindings for Enhanced Input.
	virtual void SetupInputComponent() override;

	// Enhanced Input mapping context (assign in editor).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* CombatMappingContext;

	// Input actions for moving and looking (assign in editor).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	// Input handling functions.
	UFUNCTION()
	void HandleMove(const struct FInputActionValue& Value);

	UFUNCTION()
	void HandleLook(const FInputActionValue& Value);

public:
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
