// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitySystem/Controller/ROverlayController.h"

#include "AbilitySystem/RAttributeSet.h"

void UROverlayController::BroadcastInitialValues()
{
	const URAttributeSet* RAttributeSet = CastChecked<URAttributeSet>(AttributeSet);
	OnHealthChanged.Broadcast(RAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(RAttributeSet->GetMaxHealth());
}
