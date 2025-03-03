#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Hikari.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class OCTOPATH_API AHikari : public ACharacter
{
	GENERATED_BODY()

public:
	AHikari();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Camera components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// Flag for combat logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsPlayerControlled;

	// Combat action functions
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Attack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Heal();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Flee();

	// Functions to handle player controller commands
	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
};
