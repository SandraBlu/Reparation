// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/ROverlayWidgetController.h"

#include "GAS/RAttributeSet.h"

void UROverlayWidgetController::BroadcastInitialValues()
{
	const URAttributeSet* Attributes = CastChecked<URAttributeSet>(AttributeSet);
	OnHealthChange.Broadcast(Attributes->GetHealth());
	OnMaxHealthChange.Broadcast(Attributes->GetMaxHealth());
}
