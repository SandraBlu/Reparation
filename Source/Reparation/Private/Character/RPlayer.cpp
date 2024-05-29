// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "Framework/RPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RAbilitySystemComponent.h"
#include "Framework/RPlayerController.h"
#include "GameplayTagContainer.h"
#include "Components/REquipmentComponent.h"
#include "Input/RInputComponent.h"
#include "UI/AbilityStats/RHUD.h"

#define LOCTEXT_NAMESPACE "AOCharacter"

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
	//GrantAbilities();
}

void ARPlayer::OnRep_PlayerState()
{
	//client
	InitAbilityActorInfo();
}

void ARPlayer::InitAbilityActorInfo()
{
	ARPlayerState* RPlayerState = GetPlayerState<ARPlayerState>();
	check(RPlayerState)
	RPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(RPlayerState, this);
	Cast<URAbilitySystemComponent>(RPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = RPlayerState->GetAbilitySystemComponent();
	AttributeSet = RPlayerState->GetAttributeSet();
	if (ARPlayerController* AOPC = Cast<ARPlayerController>(GetController()))
	{
 		if (ARHUD* Hud = Cast<ARHUD>(AOPC->GetHUD()))
 		{
 			Hud->InitOverlay(AOPC, RPlayerState, AbilitySystemComponent, AttributeSet);
 		}
	}
	InitializeAttributes();
}
	int32 ARPlayer::GetCharacterLevel_Implementation()
 {
 	const ARPlayerState* RPlayerState = GetPlayerState<ARPlayerState>();
 	check(RPlayerState)
 	return RPlayerState->GetCharacterLevel();
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

void ARPlayer::AbilityInputTagPressed(FGameplayTag InputTag)
{
	//GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
}

void ARPlayer::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagReleased(InputTag);
}

void ARPlayer::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagHeld(InputTag);
}

void ARPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (URInputComponent* RInputComp = CastChecked<URInputComponent>(InputComponent))
	{
		RInputComp->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
		//RInputComp->BindAction(DrawWeapon, ETriggerEvent::Triggered, this, &ARPlayer::DrawWeapon);
	}
	
}

#undef LOCTEXT_NAMESPACE