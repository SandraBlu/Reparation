// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RPlayer.h"

#include "EnhancedInputSubsystems.h"
#include "Actors/RWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/REquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARPlayer::ARPlayer()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCam"));
	FollowCam->SetupAttachment(CameraBoom);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	
	Gear = CreateDefaultSubobject<UREquipmentComponent>("GearComp");
}

void ARPlayer::SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled)
{
	if (Gear->EquippedWeapon && Gear->EquippedWeapon->GetWeaponBox())
	{
		Gear->EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		Gear->EquippedWeapon->IgnoreActors.Empty();
	}
}

void ARPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PlayerMappingContext, 0);
		}
	}
}

void ARPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
