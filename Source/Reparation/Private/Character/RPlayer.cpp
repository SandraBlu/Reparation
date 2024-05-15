// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "Framework/RPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Framework/RPlayerController.h"

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

	LevelUpFX = CreateDefaultSubobject<UNiagaraComponent>("LevelUpComp");
	LevelUpFX->SetupAttachment(GetRootComponent());
	LevelUpFX->bAutoActivate = false;
}

void ARPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
	GrantAbilities();
}

void ARPlayer::InitAbilityActorInfo()
{
	ARPlayerState* RPlayerState = GetPlayerState<ARPlayerState>();
	check(RPlayerState)
		RPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(RPlayerState, this);
	//Cast<UAOAbilityComp>(AOPlayerState->GetAbilitySystemComponent())->AbilityActorInfoInit();
	AbilitySystemComponent = RPlayerState->GetAbilitySystemComponent();
	//AttributeSet = RPlayerState->GetAttributeSet();
	if (ARPlayerController* AOPC = Cast<ARPlayerController>(GetController()))
	{
// 		if (AAOHUD* Hud = Cast<AAOHUD>(AOPC->GetHUD()))
// 		{
// 			Hud->InitOverlay(AOPC, RPlayerState, AbilitySystemComponent, AttributeSet);
// 		}
	}
	InitializeAttributes();
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

void ARPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

#undef LOCTEXT_NAMESPACE