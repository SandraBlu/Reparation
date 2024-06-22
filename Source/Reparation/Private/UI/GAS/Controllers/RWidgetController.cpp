// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/RWidgetController.h"

#include "Framework/RPlayerController.h"
#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/AbilityInfo.h"

void URWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void URWidgetController::BroadcastInitialValues()
{
}

void URWidgetController::BindCallbacksToDependencies()
{
}

void URWidgetController::BroadcastAbilityInfo()
{
	if (!GetRASC()->bGrantedAbilitiesGiven) return;
	
	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		FRAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(RASC->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = RASC->GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	GetRASC()->ForEachAbility(BroadcastDelegate);
}

ARPlayerController* URWidgetController::GetRPC()
{
	if (RPC == nullptr)
	{
		RPC = Cast<ARPlayerController>(PlayerController);
	}
	return RPC;
}

ARPlayerState* URWidgetController::GetRPS()
{
	if (RPS == nullptr)
	{
		RPS = Cast<ARPlayerState>(PlayerState);
	}
	return RPS;
}

URAbilitySystemComponent* URWidgetController::GetRASC()
{
	if (RASC == nullptr)
	{
		RASC = Cast<URAbilitySystemComponent>(AbilitySystemComponent);
	}
	return RASC;
}

URAttributeSet* URWidgetController::GetRAS()
{
	if (RAS == nullptr)
	{
		RAS = Cast<URAttributeSet>(AttributeSet);
	}
	return RAS;
}
