// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RPlayer.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "RGameplayTags.h"
#include "Actors/RWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/REquipmentComponent.h"
#include "Framework/RPlayerController.h"
#include "Framework/RPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "Input/RInputComponent.h"
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
	CharacterClass = ECharacterClass::Elementalist;
}

void ARPlayer::PossessedBy(AController* NewController)
{
	//server
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
	GrantAbilities();
}

void ARPlayer::OnRep_PlayerState()
{
	//client
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

int32 ARPlayer::GetPlayerLevel_Implementation()
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	return RPS->GetPlayerLevel();
}

FVector ARPlayer::GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag)
{
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_weapon) && IsValid(Gear->EquippedWeapon))
	{
		return Gear->EquippedWeapon->GetWeaponMesh()->GetSocketLocation(Gear->EquippedWeapon->FiringSocket);
	}
	return FVector();
}

void ARPlayer::Die()
{
	if (!Gear->EquippedWeapon) return;
	Gear->EquippedWeapon->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	Gear->EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(true);
	Gear->EquippedWeapon->GetWeaponMesh()->SetEnableGravity(true);
	Gear->EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Super::Die();
}

void ARPlayer::AddToXP_Implementation(int32 InXP)
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	RPS->AddToXP(InXP);
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
	Cast<URAbilitySystemComponent>(RPS->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = RPS->GetAbilitySystemComponent();
	AttributeSet = RPS->GetAttributeSet();
	if (ARPlayerController* RPC = Cast<ARPlayerController>(GetController()))
	{
		if (ARHUD* RHUD = Cast<ARHUD>(RPC->GetHUD()))
		{
			RHUD->InitOverlay(RPC, RPS, AbilitySystemComponent, AttributeSet);
		}
	}
	InitializeAttributes();
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

URAbilitySystemComponent* ARPlayer::GetASC()
{
	if (RAbilitySystemComponent == nullptr)
	{
		RAbilitySystemComponent = CastChecked<URAbilitySystemComponent>(AbilitySystemComponent);
	}
	return RAbilitySystemComponent;
}

void ARPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (URInputComponent* InputComp = CastChecked<URInputComponent>(InputComponent))
	{
		InputComp->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	}
}
