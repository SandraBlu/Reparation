// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AttributeMenuController.h"
#include <Attributes/AOAttributeSet.h>
#include "Abilities/Data/AttributeInfo.h"

void UAttributeMenuController::BindCallbacksToDependencies()
{
	UAOAttributeSet* AS = CastChecked<UAOAttributeSet>(AttributeSet);
	for (auto& Pair : AS->AttributeTagMap)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		
		);
	}
}

void UAttributeMenuController::BroadcastInitialValues()
{
	UAOAttributeSet* AS = CastChecked<UAOAttributeSet>(AttributeSet);
	check (AttributeInfo)
	for (auto& Pair : AS->AttributeTagMap)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
}

void UAttributeMenuController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	FAttributeData Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
