// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AOPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Character/AOPlayerBase.h"

AAOPlayerController::AAOPlayerController()
{

}

void AAOPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(PlayerMappingContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(Subsystem)
	{
		Subsystem->AddMappingContext(PlayerMappingContext, 0);
	}
}

class AAOPlayerBase* AAOPlayerController::GetPawnOwner() const
{
	return ControlledPawn;
}

void AAOPlayerController::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* PawnOwner = GetPawn<APawn>())
	{
		if (!IgnoreMoveInput)
		{
			PawnOwner->AddMovementInput(ForwardDirection, MovementVector.Y);
			PawnOwner->AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void AAOPlayerController::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (APawn* PawnOwner = GetPawn<APawn>())
	{
		PawnOwner->AddControllerYawInput(LookAxisValue.X);
		PawnOwner->AddControllerPitchInput(LookAxisValue.Y);
	}
}

void AAOPlayerController::StartInteract(const FInputActionValue& Value)
{
	if (GetPawnOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("Input Relay!!!"));
		ControlledPawn->OnStartInteract();
	}
}

void AAOPlayerController::CompleteInteract(const FInputActionValue& Value)
{
	if (AAOPlayerBase* PlayerPawn = GetPawn<AAOPlayerBase>())
	{
		PlayerPawn->OnCompleteInteract();
	}
}

void AAOPlayerController::DrawWeapon(const FInputActionValue& Value)
{
	if (AAOPlayerBase* PlayerPawn = GetPawn<AAOPlayerBase>())
	{
		PlayerPawn->DrawWeapon();
	}
}

void AAOPlayerController::PrimaryAttack(const FInputActionValue& Value)
{
	if (AAOPlayerBase* PlayerPawn = GetPawn<AAOPlayerBase>())
	{
		PlayerPawn->EnterAttack();
	}
}

void AAOPlayerController::UseThrowable(const FInputActionValue& Value)
{
	if (AAOPlayerBase* PlayerPawn = GetPawn<AAOPlayerBase>())
	{
		PlayerPawn->UseThrowable();
	}
}

void AAOPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAOPlayerController::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAOPlayerController::Look);

	//Interact
	EnhancedInputComponent->BindAction(InterAction, ETriggerEvent::Started, this, &AAOPlayerController::StartInteract);
	EnhancedInputComponent->BindAction(InterAction, ETriggerEvent::Completed, this, &AAOPlayerController::CompleteInteract);

	//Weapon
	EnhancedInputComponent->BindAction(DrawWeaponAction, ETriggerEvent::Started, this, &AAOPlayerController::DrawWeapon);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AAOPlayerController::PrimaryAttack);
	EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Started, this, &AAOPlayerController::UseThrowable);
	
}
