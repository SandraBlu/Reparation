// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Character/AOEnemy.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"

AAOEnemy::AAOEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAOAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UAOAttributeSet>("AttributeSet");

	// Enabled on mesh to react to incoming projectiles
	GetMesh()->SetGenerateOverlapEvents(true);
}

int32 AAOEnemy::GetPlayerLevel()
{
	return Level;
}

void AAOEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
}

void AAOEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAOAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}
