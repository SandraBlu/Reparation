// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/REnemy.h"
#include "RGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "RGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Reparation/Reparation.h"
#include "UI/GAS/RUserWidget.h"

AREnemy::AREnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("ASC");
	AttributeSet = CreateDefaultSubobject<URAttributeSet>("Attributes");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("Health");
	HealthBar->SetupAttachment(GetRootComponent());
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Combat");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

int32 AREnemy::GetPLayerLevel_Implementation()
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
	return FVector();
}

void AREnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void AREnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	URAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent);
	
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
		//Call Hit React function
		AbilitySystemComponent->RegisterGameplayTagEvent(FRGameplayTags::Get().ability_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AREnemy::HitReactTagChanged);
		//Broadcast Initial Values
		OnHealthChange.Broadcast(AS->GetHealth());
		OnMaxHealthChange.Broadcast(AS->GetMaxHealth());
	}
}

void AREnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	InitializeAttributes();
}

void AREnemy::InitializeAttributes() const
{
	URAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}
