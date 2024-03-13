// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Character/AOEnemy.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"
#include "Reparation/Reparation.h"
#include "Components/WidgetComponent.h"
#include "UI/AOUserWidget.h"
#include <Abilities/BFLAbilitySystem.h>
#

AAOEnemy::AAOEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAOAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UAOAttributeSet>("AttributeSet");

	Health = CreateDefaultSubobject<UWidgetComponent>("Health Bar");
	Health->SetupAttachment(GetRootComponent());

	// Enabled on mesh to react to incoming projectiles
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

int32 AAOEnemy::GetPlayerLevel()
{
	return Level;
}

void AAOEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();

	
	//Set widget controller for enemy health bar
	if (UAOUserWidget* EnemyUI = Cast<UAOUserWidget>(Health->GetUserWidgetObject()))
	{
		EnemyUI->SetWidgetController(this);
	}
	//Set up binding value changes (lambdas) for Progress bar
	if (const UAOAttributeSet* AS = Cast<UAOAttributeSet>(AttributeSet))
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
		//Set Initial values for attributes
		OnHealthChange.Broadcast(AS->GetHealth());
		OnMaxHealthChange.Broadcast(AS->GetMaxHealth());
	}
}

void AAOEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAOAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	InitializeAttributes();
}

void AAOEnemy::InitializeAttributes() const
{
	UBFLAbilitySystem::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}
