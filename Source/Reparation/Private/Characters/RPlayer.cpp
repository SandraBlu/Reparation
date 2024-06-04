// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RPlayer.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Actors/RWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/REquipmentComponent.h"
#include "Framework/RPlayerController.h"
#include "Framework/RPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/GAS/RHUD.h"

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

void ARPlayer::PossessedBy(AController* NewController)
{
	//server
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void ARPlayer::OnRep_PlayerState()
{
	//client
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
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

void ARPlayer::InitAbilityActorInfo()
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	RPS->GetAbilitySystemComponent()->InitAbilityActorInfo(RPS, this);
	AbilitySystemComponent = RPS->GetAbilitySystemComponent();
	AttributeSet = RPS->GetAttributeSet();
	if (ARPlayerController* RPC = Cast<ARPlayerController>(GetController()))
	{
		if (ARHUD* RHUD = Cast<ARHUD>(RPC->GetHUD()))
		{
			RHUD->InitOverlay(RPC, RPS, AbilitySystemComponent, AttributeSet);
		}
	}
	
}

void ARPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
