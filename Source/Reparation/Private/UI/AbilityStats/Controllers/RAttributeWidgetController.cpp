// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilityStats/Controllers/RAttributeWidgetController.h"
#include "RGameplayTags.h"
#include "AbilitySystem/RAttributeSet.h"
#include "AbilitySystem/Data/AttributeData.h"

void URAttributeWidgetController::BroadcastInitialValues()
{
	URAttributeSet* AS = CastChecked<URAttributeSet>(AttributeSet);

	check(AttributeData);

	for (auto& Pair : AS->AttributeTagMap)
	{
		FRAttributeInfo Info = AttributeData->FindAttributeInfoForTag(Pair.Key);
		Info.AttributeValue = Pair.Value().GetNumericValue(AS);
		AttributeInfoDelegate.Broadcast(Info);
	}
}

void URAttributeWidgetController::BindCallbacksToDependencies()
{
	
}
