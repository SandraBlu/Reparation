// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/RAttributeMenuController.h"

#include "RGameplayTags.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/RAttributeInfo.h"

void URAttributeMenuController::BindCallbacksToDependencies()
{
	URAttributeSet* AS = CastChecked<URAttributeSet>(AttributeSet);
	check(AttributeInfo);
	for (auto& Pair : AS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda([this, Pair](const FOnAttributeChangeData& Data)
		{
			BroadcastAttributeInfo(Pair.Key, Pair.Value());
		});	
	}
}

void URAttributeMenuController::BroadcastInitialValues()
{
	URAttributeSet* AS = CastChecked<URAttributeSet>(AttributeSet);
	check(AttributeInfo);
	for (auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
}

void URAttributeMenuController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,
	const FGameplayAttribute& Attribute) const
{
	FAttributeData Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}

