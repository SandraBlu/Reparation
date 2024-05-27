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

	AttributeSet = CreateDefaultSubobject<URAttributeSet>("Attributes");

	PlayerInventory = CreateDefaultSubobject<URInventoryComponent>("InventoryComp");
	PlayerInventory->SetCapacity(25);
	PlayerInventory->SetWeightCapacity(60.f);

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

//void ARPlayerState::AddToXP(int32 InXP)
//{
//	XP += InXP;
//	OnXPChangeDelegate.Broadcast(XP);
//}
//
//void ARPlayerState::SetXP(int32 InXP)
//{
//	XP = InXP;
//	OnXPChangeDelegate.Broadcast(XP);
//}

void ARPlayerState::OnRep_Level(int32 OldLevel)
{

}

//void ARPlayerState::AddToLevel(int32 InLevel)
//{
//	Level += InLevel;
//	OnLevelChangeDelegate.Broadcast(Level);
//}

//void ARPlayerState::SetLevel(int32 InLevel)
//{
//	Level = InLevel;
//	OnLevelChangeDelegate.Broadcast(Level);
//}

//void ARPlayerState::AddToAttributePts(int32 InAttributePts)
//{
//	AttributePts += InAttributePts;
//	OnAttributePtsChangeDelegate.Broadcast(AttributePts);
//}

//void ARPlayerState::AddToAbilityPts(int32 InAbilityPts)
//{
//	AbilityPts += InAbilityPts;
//	OnAbilityPtsChangeDelegate.Broadcast(AbilityPts);
//}