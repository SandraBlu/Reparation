// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Controller/RWidgetController.h"
#include "Framework/RPlayerState.h"
#include "Framework/RPlayerController.h"
#include "AbilitySystem/RAbilitySystemComponent.h"
#include "AbilitySystem/RAttributeSet.h"
//#include "AbilitySystem/Data/AbilityInfo.h"

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
	/*if (!GetRASC()->bAbilityGranted) return;

	FCheckForAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
		{

			FAOAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AOAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
			Info.InputTag = AOAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
			Info.StatusTag = AOAbilitySystemComponent->GetStatusTagFromSpec(AbilitySpec);
			AbilityInfoDelegate.Broadcast(Info);
		});
	GetAOASC()->CheckForAbility(BroadcastDelegate);*/
}

ARPlayerController* URWidgetController::GetRPC()
{
	if (RPlayerController == nullptr)
	{
		RPlayerController = Cast<ARPlayerController>(PlayerController);
	}
	return RPlayerController;
}

ARPlayerState* URWidgetController::GetRPS()
{
	if (RPlayerState == nullptr)
	{
		RPlayerState = Cast<ARPlayerState>(PlayerState);
	}
	return RPlayerState;
}

URAbilitySystemComponent* URWidgetController::GetRASC()
{
	if (RAbilitySystemComponent == nullptr)
	{
		RAbilitySystemComponent = Cast<URAbilitySystemComponent>(AbilitySystemComponent);
	}
	return RAbilitySystemComponent;
}

URAttributeSet* URWidgetController::GetRAS()
{
	if (RAttributeSet == nullptr)
	{
		RAttributeSet = Cast<URAttributeSet>(AttributeSet);
	}
	return 	RAttributeSet;
}