// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AOPlayerState.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"

AAOPlayerState::AAOPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAOAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAOAttributeSet>("AttributeSet");
	NetUpdateFrequency = 100.f;
}

UAbilitySystemComponent* AAOPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
