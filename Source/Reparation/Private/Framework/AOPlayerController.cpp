// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AOPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Character/AOPlayerBase.h"
#include "GameplayTagContainer.h"
#include "Input/AOInputComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/AOAbilitySystemComponent.h"

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
	if (AAOPlayerBase* PlayerPawn = GetPawn<AAOPlayerBase>())
	{
		UE_LOG(LogTemp, Warning, TEXT("Input Relay!!!"));
		PlayerPawn->OnStartInteract();
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
	UAOInputComponent* AOInputComponent = CastChecked<UAOInputComponent>(InputComponent);
	AOInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	AOInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAOPlayerController::Move);
	AOInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAOPlayerController::Look);

	//Interact
	AOInputComponent->BindAction(InterAction, ETriggerEvent::Started, this, &AAOPlayerController::StartInteract);
	AOInputComponent->BindAction(InterAction, ETriggerEvent::Completed, this, &AAOPlayerController::CompleteInteract);

	//Weapon
	AOInputComponent->BindAction(DrawWeaponAction, ETriggerEvent::Started, this, &AAOPlayerController::DrawWeapon);
	AOInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AAOPlayerController::PrimaryAttack);
	AOInputComponent->BindAction(ThrowAction, ETriggerEvent::Started, this, &AAOPlayerController::UseThrowable);
	
}

void AAOPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	//GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
}

void AAOPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if(GetASC() == nullptr) return;
	GetASC()->AbilityInputTagReleased(InputTag);
}

void AAOPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagHeld(InputTag);
}

UAOAbilitySystemComponent* AAOPlayerController::GetASC()
{
	if (!AOAbilityComp)
	{
		AOAbilityComp = Cast<UAOAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AOAbilityComp;
}
