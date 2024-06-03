// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitySystem/Controller/RWidgetController.h"

void URWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void URWidgetController::BroadcastInitialValues()
{
	
}
