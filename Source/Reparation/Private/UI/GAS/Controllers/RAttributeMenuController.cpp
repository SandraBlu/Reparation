// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/RAttributeMenuController.h"

#include "RGameplayTags.h"
#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/RAttributeInfo.h"

void URAttributeMenuController::BindCallbacksToDependencies()
{
	
	check(AttributeInfo);
	for (auto& Pair : GetRAS()->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda([this, Pair](const FOnAttributeChangeData& Data)
		{
			BroadcastAttributeInfo(Pair.Key, Pair.Value());
		});	
	}
	GetRPS()->OnAttributePtsChangeDelegate.AddLambda([this] (int32 Points)
	{
		AttributePointsChangeDelegate.Broadcast(Points);
	});
}

void URAttributeMenuController::BroadcastInitialValues()
{
	check(AttributeInfo);
	for (auto& Pair : GetRAS()->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
	AttributePointsChangeDelegate.Broadcast(GetRPS()->GetAttributePts());
}

void URAttributeMenuController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	GetRASC()->UpgradeAttribute(AttributeTag);
}

void URAttributeMenuController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,
                                                       const FGameplayAttribute& Attribute) const
{
	FAttributeData Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}

