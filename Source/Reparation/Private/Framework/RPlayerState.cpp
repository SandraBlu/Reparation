// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RPlayerState.h"

#include "Components/RInventoryComponent.h"
#include "Framework/RGameMode.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "RTypes/REnumTypes.h"

ARPlayerState::ARPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<URAttributeSet>("AttributeSet");

	PlayerInventory = CreateDefaultSubobject<URInventoryComponent>("InventoryComp");
	PlayerInventory->SetCapacity(25);
	PlayerInventory->SetWeightCapacity(60.f);
	
	NetUpdateFrequency = 100.f;
}

void ARPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARPlayerState, Level);
	DOREPLIFETIME(ARPlayerState, XP);
	DOREPLIFETIME(ARPlayerState, AttributePoints);
	DOREPLIFETIME(ARPlayerState, AbilityPoints);
}

UAbilitySystemComponent* ARPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ARPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChangeDelegate.Broadcast(XP);
}

void ARPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangeDelegate.Broadcast(Level);
}

void ARPlayerState::AddToAttributePts(int32 InAttributePts)
{
	AttributePoints += InAttributePts;
	OnAttributePtsChangeDelegate.Broadcast(AttributePoints);
}

void ARPlayerState::AddToAbilityPts(int32 InAbilityPts)
{
	AbilityPoints += InAbilityPts;
	OnAbilityPtsChangeDelegate.Broadcast(AbilityPoints);
}

void ARPlayerState::SetXP(int32 InXP)
{
	XP = InXP;
	OnXPChangeDelegate.Broadcast(XP);
}

void ARPlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangeDelegate.Broadcast(Level);
}

void ARPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangeDelegate.Broadcast(Level);
}

void ARPlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChangeDelegate.Broadcast(XP);
}

void ARPlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	OnAttributePtsChangeDelegate.Broadcast(AttributePoints);
}

void ARPlayerState::OnRep_AbilityPoints(int32 OldAbilityPoints)
{
	OnAbilityPtsChangeDelegate.Broadcast(AbilityPoints);
}
