// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilityStats/Controllers/RAttributeWidgetController.h"
#include "RGameplayTags.h"
#include "AbilitySystem/RAttributeSet.h"
#include "AbilitySystem/Data/AttributeData.h"

void URAttributeWidgetController::BroadcastInitialValues()
{
	URAttributeSet* AS = CastChecked<URAttributeSet>(AttributeSet);

	check(AttributeData);

	FRAttributeInfo Info = AttributeData->FindAttributeInfoForTag(FRGameplayTags::Get().Attributes_Primary_Agility);
	Info.AttributeValue = AS->GetAgility();
	AttributeInfoDelegate.Broadcast(Info);
}

void URAttributeWidgetController::BindCallbacksToDependencies()
{
	
}
