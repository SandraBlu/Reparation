// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilityStats/Controllers/RAttributeWidgetController.h"
#include "AbilitySystem/RAttributeSet.h"
#include "AbilitySystem/Data/AttributeData.h"

void URAttributeWidgetController::BindCallbacksToDependencies()
{
	URAttributeSet* AS = CastChecked<URAttributeSet>(AttributeSet);
	check(AttributeData);
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

void URAttributeWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	FRAttributeInfo Info = AttributeData->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}

void URAttributeWidgetController::BroadcastInitialValues()
{
	URAttributeSet* AS = CastChecked<URAttributeSet>(AttributeSet);
	check(AttributeData);

	for (auto& Pair : AS->AttributeTagMap)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
}


