// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/RAbilityMenuController.h"

#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/AbilityInfo.h"

void URAbilityMenuController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
}

void URAbilityMenuController::BindCallbacksToDependencies()
{
	GetRASC()->AbilityStatusChangeDelegate.AddLambda([this] (const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag)
	{
		if (AbilityInfo)
		{
			FRAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});
}
