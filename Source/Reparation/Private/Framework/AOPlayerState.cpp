// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AOPlayerState.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"
#include <Net/UnrealNetwork.h>

AAOPlayerState::AAOPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAOAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAOAttributeSet>("AttributeSet");
	NetUpdateFrequency = 100.f;
}

void AAOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAOPlayerState, Level);
}

UAbilitySystemComponent* AAOPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAOPlayerState::OnRep_Level(int32 OldLevel)
{

}
