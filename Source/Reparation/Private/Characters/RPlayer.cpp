// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RPlayer.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraComponent.h"
#include "RGameplayTags.h"
#include "Actors/RMelee.h"
#include "Actors/RWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/REquipmentComponent.h"
#include "Components/RFootstepsComponent.h"
#include "Components/Combat/PlayerCombatComp.h"
#include "Framework/RPlayerController.h"
#include "Framework/RPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/LevelUpInfo.h"
#include "Input/RInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GAS/RHUD.h"

ARPlayer::ARPlayer()
{
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	
	Gear = CreateDefaultSubobject<UREquipmentComponent>("GearComp");
	CharacterClass = ECharacterClass::Elementalist;

	LevelUpFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFXComp"));
	LevelUpFX->SetupAttachment(GetRootComponent());
	LevelUpFX->bAutoActivate = false;

	PlayerCombatComp = CreateDefaultSubobject<UPlayerCombatComp>(TEXT("PlayerCombatComp"));
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

UPawnCombatComponent* ARPlayer::GetPawnCombatComponent() const
{
	return PlayerCombatComp;
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
		//GetPawnCombatComponent()->RegisterSpawnedWeapon(GameplayTags.combatSocket_weapon, Gear->EquippedWeapon, true);
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

/*void ARPlayer::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	if (!Gear->EquippedWeapon) return;
	Gear->EquippedWeapon->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	Gear->EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(true);
	Gear->EquippedWeapon->GetWeaponMesh()->SetEnableGravity(true);
	Gear->EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Gear->EquippedWeapon->GetWeaponMesh()->AddImpulse(DeathImpulse);

	UGameplayStatics::PlaySoundAtLocation(this, DeathCry, GetActorLocation(), GetActorRotation());
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	bDead = true;
	OnDeath.Broadcast(this);
}*/

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
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void ARPlayer::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagReleased(InputTag);
}

void ARPlayer::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (Gear->EquippedWeapon == nullptr ) return;
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
		InputComp->BindAction(SwitchTarget, ETriggerEvent::Started, this, &ARPlayer::Input_SwitchTargetTriggered);
		InputComp->BindAction(SwitchTarget, ETriggerEvent::Completed, this, &ARPlayer::Input_SwitchTargetCompleted);
	}
}