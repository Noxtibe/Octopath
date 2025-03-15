#include "Character/Hikari.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

AHikari::AHikari()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsPlayerControlled = true;

	// Create and configure the SpringArm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeRotation(FRotator(0.0f, 0.f, 0.f));

	// Create and configure the Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = true;

	// Do not force controller rotation on the Pawn
	bUseControllerRotationYaw = false;
	// Automatically orient the character to movement direction
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// Set the rotation rate (speed of turning)
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
}

void AHikari::BeginPlay()
{
	Super::BeginPlay();
}

void AHikari::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHikari::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Although the PlayerController handles Enhanced Input,
	// this function is still necessary for the API interface.
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AHikari::Move(const FInputActionValue& Value)
{
	// The input value is a Vector2D (X for sideways, Y for forward/backward)
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		// Get the controller's rotation (keep only the Yaw)
		FRotator Rotation = Controller->GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);

		// Calculate forward and right direction vectors
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHikari::Look(const FInputActionValue& Value)
{
	// The input value is a Vector2D (X for yaw, Y for pitch)
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
