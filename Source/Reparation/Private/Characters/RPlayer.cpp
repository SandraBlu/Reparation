// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RPlayer.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraComponent.h"
#include "RDebugHelper.h"
#include "RGameplayTags.h"
#include "Actors/RWeapon.h"
#include "Components/CapsuleComponent.h"
#include "Components/REquipmentComponent.h"
#include "Components/RFootstepsComponent.h"
#include "Framework/RPlayerController.h"
#include "Framework/RPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Locomotion/RCharacterMovementComponent.h"
#include "Locomotion/RLocomotionComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/LevelUpInfo.h"
#include "Input/RInputComponent.h"
#include "UI/GAS/RHUD.h"

ARPlayer::ARPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Speeds come from the locomotion config, not from here.
	Locomotion = CreateDefaultSubobject<URLocomotionComponent>(TEXT("LocomotionComp"));

	Gear = CreateDefaultSubobject<UREquipmentComponent>("GearComp");
	CharacterClass = ECharacterClass::Elementalist;

	LevelUpFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFXComp"));
	LevelUpFX->SetupAttachment(GetRootComponent());
	LevelUpFX->bAutoActivate = false;
	
	FootstepComponent = CreateDefaultSubobject<URFootstepsComponent>(TEXT("FootstepComp"));

	bUsingBlock = false;
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
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_handL))
	{
		return GetMesh()->GetSocketLocation(HandRSocket);
	}
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_handR))
	{
		return GetMesh()->GetSocketLocation(HandLSocket);
	}
	return FVector();
}

AActor* ARPlayer::GetCurrentEquippedWeapon_Implementation(ARWeapon* InWeapon)
{
	return Gear->EquippedWeapon;
}

void ARPlayer::Die(const FVector& DeathImpulse)
{
	if (!Gear->EquippedWeapon) return;
	Gear->EquippedWeapon->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	Gear->EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(true);
	Gear->EquippedWeapon->GetWeaponMesh()->SetEnableGravity(true);
	Gear->EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Gear->EquippedWeapon->GetWeaponMesh()->AddImpulse(DeathImpulse);

	//UGameplayStatics::PlaySoundAtLocation(this, DeathCry, GetActorLocation(), GetActorRotation());
	//GetMesh()->SetSimulatePhysics(true);
	//GetMesh()->SetEnableGravity(true);
	//GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	//GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	//GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	bDead = true;
	OnDeath.Broadcast(this);
}

void ARPlayer::AddToXP_Implementation(int32 InXP)
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	RPS->AddToXP(InXP);
}

void ARPlayer::LevelUp_Implementation()
{
	MulticastLevelUpVFX();
}

void ARPlayer::MulticastLevelUpVFX_Implementation()
{
	if (IsValid(LevelUpFX))
	{
		LevelUpFX->Activate(true);
	}
}

int32 ARPlayer::GetXP_Implementation() const
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	return RPS->GetXP();
}

int32 ARPlayer::FindLevelForXP_Implementation(int32 InXP) const
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	return RPS->LevelUpInfo->FindLevelForXP(InXP);
}

int32 ARPlayer::GetAttributePtsReward_Implementation(int32 Level) const
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	return RPS->LevelUpInfo->LevelUpInfo[Level].AttributePointReward;
}

int32 ARPlayer::GetAbilityPtsReward_Implementation(int32 Level) const
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	return RPS->LevelUpInfo->LevelUpInfo[Level].AbilityPointReward;
}

void ARPlayer::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	RPS->AddToLevel(InPlayerLevel);
	if (URAbilitySystemComponent* RASC = Cast<URAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		RASC->UpdateAbilityStatus(RPS->GetPlayerLevel());
	}
}

void ARPlayer::AddToAttributePts_Implementation(int32 InAttributePoints)
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	RPS->AddToAttributePts(InAttributePoints);
}


void ARPlayer::AddToAbilityPts_Implementation(int32 InAbilityPoints)
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	RPS->AddToAbilityPts(InAbilityPoints);
}

int32 ARPlayer::GetAttributePoints_Implementation() const
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	return RPS->GetAttributePts();
}

int32 ARPlayer::GetAbilityPoints_Implementation() const
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	return RPS->GetAbilityPts();
}

ARWeapon* ARPlayer::GetCurrentWeapon_Implementation()
{
	if (Gear->EquippedWeapon)
	{
		return Gear->EquippedWeapon;
	}
	return nullptr;
}

URFootstepsComponent* ARPlayer::GetFootstepsComp() const
{
	return FootstepComponent;
}

bool ARPlayer::GetIsUsingBlock()
{
	return bUsingBlock;
}

bool ARPlayer::SetIsUsingBlock(bool block)
{
	bUsingBlock = block;
	return bUsingBlock;
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

void ARPlayer::Input_SwitchTargetTriggered(const FInputActionValue& Value)
{
	SwitchTargetDirection = Value.Get<FVector2D>();
}

void ARPlayer::Input_SwitchTargetCompleted(const FInputActionValue& Value)
{
	FGameplayEventData EventData;
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, SwitchTargetDirection.X > 0.f? GameplayTags.Event_SwitchTarget_Right : GameplayTags.Event_SwitchTarget_Left, EventData);
	Debug::Print((TEXT("Switch Target Direction: ") + SwitchTargetDirection.ToString()));
}

void ARPlayer::InitAbilityActorInfo()
{
	ARPlayerState* RPS = GetPlayerState<ARPlayerState>();
	check(RPS);
	RPS->GetAbilitySystemComponent()->InitAbilityActorInfo(RPS, this);
	Cast<URAbilitySystemComponent>(RPS->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = RPS->GetAbilitySystemComponent();
	AttributeSet = RPS->GetAttributeSet();
	OnASCRegistered.Broadcast(AbilitySystemComponent);
	
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
	if (Gear->EquippedWeapon == nullptr ) return;
	if (GetASC() == nullptr) return;
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void ARPlayer::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagReleased(InputTag);
}

/*void ARPlayer::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (Gear->EquippedWeapon == nullptr ) return;
 	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagHeld(InputTag);	
}*/

URAbilitySystemComponent* ARPlayer::GetASC()
{
	if (RAbilitySystemComponent == nullptr)
	{
		RAbilitySystemComponent = CastChecked<URAbilitySystemComponent>(AbilitySystemComponent);
	}
	return RAbilitySystemComponent;
}

void ARPlayer::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (Locomotion)
	{
		Locomotion->SetMovementInput(Axis);
	}

	// Climbing maps input onto the wall plane, which the locomotion component
	// does itself. Camera relative input here is purely horizontal, so projecting
	// it onto a vertical wall can never produce any ascent.
	if (Locomotion && Locomotion->IsClimbing())
	{
		return;
	}

	if (!Controller)
	{
		return;
	}

	// Move relative to where the camera looks, ignoring pitch.
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FRotationMatrix YawMatrix(YawRotation);

	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::Y), Axis.X);
}

void ARPlayer::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ARPlayer::Input_JumpStarted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->TryJump();
	}
}

void ARPlayer::Input_JumpCompleted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->StopJumping();
	}
}

void ARPlayer::Input_SprintStarted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetSprintHeld(true);
	}
}

void ARPlayer::Input_SprintCompleted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetSprintHeld(false);
	}
}

void ARPlayer::Input_WalkStarted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetWalkHeld(true);
	}
}

void ARPlayer::Input_WalkCompleted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetWalkHeld(false);
	}
}

void ARPlayer::Input_CrouchToggled(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->ToggleCrouch();
	}
}

void ARPlayer::Input_SwimVertical(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetSwimVerticalInput(Value.Get<float>());
	}
}

void ARPlayer::Input_SwimVerticalCompleted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetSwimVerticalInput(0.f);
	}
}

void ARPlayer::Input_GlideStarted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetGlideHeld(true);
	}
}

void ARPlayer::Input_GlideCompleted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetGlideHeld(false);
	}
}

void ARPlayer::Input_ClimbStarted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetClimbHeld(true);
	}
}

void ARPlayer::Input_ClimbCompleted(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->SetClimbHeld(false);
	}
}

void ARPlayer::Input_FlyToggled(const FInputActionValue& Value)
{
	if (Locomotion)
	{
		Locomotion->ToggleFlight();
	}
}

void ARPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (URInputComponent* InputComp = CastChecked<URInputComponent>(InputComponent))
	{
		InputComp->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased);
		InputComp->BindAction(SwitchTarget, ETriggerEvent::Started, this, &ARPlayer::Input_SwitchTargetTriggered);
		InputComp->BindAction(SwitchTarget, ETriggerEvent::Completed, this, &ARPlayer::Input_SwitchTargetCompleted);

		// Locomotion. Each action is optional so a partially configured input
		// asset still lets the rest of the character work.
		if (MoveAction)
		{
			InputComp->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARPlayer::Input_Move);
			InputComp->BindAction(MoveAction, ETriggerEvent::Completed, this, &ARPlayer::Input_Move);
		}
		if (LookAction)
		{
			InputComp->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARPlayer::Input_Look);
		}
		if (JumpAction)
		{
			InputComp->BindAction(JumpAction, ETriggerEvent::Started, this, &ARPlayer::Input_JumpStarted);
			InputComp->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARPlayer::Input_JumpCompleted);
		}
		if (SprintAction)
		{
			InputComp->BindAction(SprintAction, ETriggerEvent::Started, this, &ARPlayer::Input_SprintStarted);
			InputComp->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARPlayer::Input_SprintCompleted);
		}
		if (WalkAction)
		{
			InputComp->BindAction(WalkAction, ETriggerEvent::Started, this, &ARPlayer::Input_WalkStarted);
			InputComp->BindAction(WalkAction, ETriggerEvent::Completed, this, &ARPlayer::Input_WalkCompleted);
		}
		if (CrouchAction)
		{
			InputComp->BindAction(CrouchAction, ETriggerEvent::Started, this, &ARPlayer::Input_CrouchToggled);
		}
		if (SwimVerticalAction)
		{
			InputComp->BindAction(SwimVerticalAction, ETriggerEvent::Triggered, this, &ARPlayer::Input_SwimVertical);
			InputComp->BindAction(SwimVerticalAction, ETriggerEvent::Completed, this, &ARPlayer::Input_SwimVerticalCompleted);
		}
		if (GlideAction)
		{
			InputComp->BindAction(GlideAction, ETriggerEvent::Started, this, &ARPlayer::Input_GlideStarted);
			InputComp->BindAction(GlideAction, ETriggerEvent::Completed, this, &ARPlayer::Input_GlideCompleted);
		}
		if (ClimbAction)
		{
			InputComp->BindAction(ClimbAction, ETriggerEvent::Started, this, &ARPlayer::Input_ClimbStarted);
			InputComp->BindAction(ClimbAction, ETriggerEvent::Completed, this, &ARPlayer::Input_ClimbCompleted);
		}
		if (FlyAction)
		{
			InputComp->BindAction(FlyAction, ETriggerEvent::Started, this, &ARPlayer::Input_FlyToggled);
		}
	}
}