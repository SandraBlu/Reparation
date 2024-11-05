// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/REnemy.h"
#include "RGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AI/RAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/PawnSensingComponent.h"
#include "Reparation/Reparation.h"
#include "UI/GAS/RUserWidget.h"

AREnemy::AREnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("ASC");
	AttributeSet = CreateDefaultSubobject<URAttributeSet>("Attributes");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("Health");
	HealthBar->SetupAttachment(GetRootComponent());
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComp");

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Combat");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void AREnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		URAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}
	
	if (URUserWidget* EnemyHealthUI = Cast<URUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		EnemyHealthUI->SetWidgetController(this);
	}

	if (const URAttributeSet* AS = Cast<URAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChange.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChange.Broadcast(Data.NewValue);
			}
		);
		//Call Hit React/Stunned function
		AbilitySystemComponent->RegisterGameplayTagEvent(FRGameplayTags::Get().ability_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AREnemy::HitReactTagChanged);
		//Broadcast Initial Values
		OnHealthChange.Broadcast(AS->GetHealth());
		OnMaxHealthChange.Broadcast(AS->GetMaxHealth());
	}
}

void AREnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	AIC = Cast<ARAIController>(NewController);
	AIC->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	AIC->RunBehaviorTree(BehaviorTree);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), false);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

int32 AREnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

FVector AREnemy::GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag)
{
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketLocation(WeaponDamageSocket);
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

void AREnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);
	MulticastHandleDeath(DeathImpulse);
	if (AIC) AIC->GetBlackboardComponent()->SetValueAsBool(FName("IsDead"), true);
	DissolveMesh();
}

void AREnemy::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	Weapon->AddImpulse(DeathImpulse);
	
	UGameplayStatics::PlaySoundAtLocation(this, DeathCry, GetActorLocation(), GetActorRotation());
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	bDead = true;
	OnDeath.Broadcast(this);
}

void AREnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AREnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AREnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (AIC && AIC->GetBlackboardComponent())
	{
		AIC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void AREnemy::SetToStunned(bool bIsStunned) const
{
	if (bStunned)
  	{
 		bIsStunned = true;
  		if (AIC && AIC->GetBlackboardComponent())
  		{
  			AIC->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bStunned);
  		}
  	}
 	bIsStunned = false;
}

void AREnemy::SetToEndStun()
{
	if (AIC && AIC->GetBlackboardComponent())
	{
		AIC->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), false);
	}
}

void AREnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	if (HasAuthority())
	{
		InitializeAttributes();
	}
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

void AREnemy::InitializeAttributes() const
{
	URAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}
