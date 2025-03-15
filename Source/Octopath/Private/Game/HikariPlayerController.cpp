#include "Game/HikariPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "Character/Hikari.h"
#include "Blueprint/UserWidget.h"
#include "Widget/PlayerTurnMenuWidget.h"
#include "TimerManager.h"

AHikariPlayerController::AHikariPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHikariPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add the Enhanced Input Mapping Context.
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// By default, use GameOnly mode and hide the mouse cursor.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);
}

void AHikariPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHikariPlayerController::HandleMove);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHikariPlayerController::HandleLook);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AHikariPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (AHikari* HikariPawn = Cast<AHikari>(GetPawn()))
	{
		HikariPawn->Move(Value);
	}
}

void AHikariPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (AHikari* HikariPawn = Cast<AHikari>(GetPawn()))
	{
		HikariPawn->Look(Value);
	}
}

void AHikariPlayerController::EnableCombatInputMode()
{
	UE_LOG(LogTemp, Log, TEXT("EnableCombatInputMode() called"));

	// Switch to the combat mapping context.
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// Optionally: Remove the base mapping context if you do not want gameplay actions to trigger.
		Subsystem->RemoveMappingContext(DefaultMappingContext);
		// Add the combat mapping context.
		if (CombatMappingContext)
		{
			Subsystem->AddMappingContext(CombatMappingContext, 1);
		}
	}

	// Switch to GameAndUI mode for UI input and show the mouse cursor.
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
	UE_LOG(LogTemp, Log, TEXT("Mouse cursor is now visible"));
}

void AHikariPlayerController::DisableCombatInputMode()
{
	// Remove the combat mapping context and (optionally) re-add the base mapping context.
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (CombatMappingContext)
		{
			Subsystem->RemoveMappingContext(CombatMappingContext);
		}
		// Re-add the base mapping context if necessary:
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Switch to GameOnly mode to return to normal gameplay.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);
	UE_LOG(LogTemp, Log, TEXT("DisableCombatInputMode() called: Mouse cursor is now hidden"));
}
