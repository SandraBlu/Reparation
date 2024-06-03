// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RPlayerState.h"

#include "AbilitySystem/RAbilitySystemComponent.h"
#include "AbilitySystem/RAttributeSet.h"
#include "Components/RInventoryComponent.h"
#include "Net/UnrealNetwork.h"

ARPlayerState::ARPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("ASC");
	AbilitySystemComponent ->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<URAttributeSet>("AttributeSet");
	
	PlayerInventory = CreateDefaultSubobject<URInventoryComponent>("InventoryComp");
	PlayerInventory->SetCapacity(25);
	PlayerInventory->SetWeightCapacity(60.f);

	NetUpdateFrequency = 100.f;
}

UAbilitySystemComponent* ARPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
