// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RPlayerState.h"

#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Net/UnrealNetwork.h"

ARPlayerState::ARPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<URAttributeSet>("AttributeSet");
	NetUpdateFrequency = 100.f;
}

void ARPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARPlayerState, Level);
}

UAbilitySystemComponent* ARPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ARPlayerState::OnRep_Level(int32 OldLevel)
{
}
