// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Controller/RAttributeMenuController.h"
#include "RGameplayTags.h"
#include "AbilitySystem/RAttributeSet.h"
#include "AbilitySystem/Data/RAttributeData.h"
#include "Framework/RPlayerState.h"
#include "AbilitySystem/RAbilitySystemComponent.h"

void URAttributeMenuController::BindCallbacksToDependencies()
{
	for (auto& Pair : GetRAS()->AttributeTagMap)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}

	GetRPS()->OnAttributePtsChangeDelegate.AddLambda([this](int32 Points)
		{
			AttributePtsChangeDelegate.Broadcast(Points);
		}
	);
}

void URAttributeMenuController::BroadcastInitialValues()
{
	check(AttributeInfo)
	for (auto& Pair : GetRAS()->AttributeTagMap)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
	AttributePtsChangeDelegate.Broadcast(GetRPS()->GetAttributePts());
}

void URAttributeMenuController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	GetRASC()->UpgradeAttribute(AttributeTag);
}

void URAttributeMenuController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	FAttributeData Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
