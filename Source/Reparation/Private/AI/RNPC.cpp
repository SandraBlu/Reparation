// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RNPC.h"

#include "RGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Reparation/Reparation.h"
#include "UI/GAS/RUserWidget.h"

ARNPC::ARNPC()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<URAttributeSet>("AttributeSet");

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
}

int32 ARNPC::GetPLayerLevel_Implementation()
{
	return Level;
}

FVector ARNPC::GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag)
{
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketLocation(WeaponDamageSocket);
	}
	return FVector();
}

void ARNPC::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
	//set health bar widget controller
	if (URUserWidget* EnemyHealthUI = Cast<URUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		EnemyHealthUI->SetWidgetController(this);
	}
	//respond to attribute changes
	if (const URAttributeSet* RAS = Cast<URAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAS->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		});
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAS->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});
		//broadcast Initial Values
		OnHealthChanged.Broadcast(RAS->GetHealth());
		OnMaxHealthChanged.Broadcast(RAS->GetMaxHealth());
	}
}

void ARNPC::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	InitializeAttributes();
}
