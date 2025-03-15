#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Hikari.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;

/**
 * AHikari
 *
 * This class represents the main character controlled by the player.
 * It handles camera management, movement input, and gameplay logic in both exploration and combat scenarios.
 */
UCLASS()
class OCTOPATH_API AHikari : public ACharacter
{
	GENERATED_BODY()

public:
	// Constructors and override functions
	AHikari();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Public functions
	/** Handles movement input */
	void Move(const struct FInputActionValue& Value);
	/** Handles look/rotation input */
	void Look(const struct FInputActionValue& Value);

public:
	// Public variables
	// --- Camera Components ---
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	/** Follow camera for the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// --- Combat Logic ---
	/** Flag indicating whether the character is controlled by the player (used in combat logic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsPlayerControlled;
};
