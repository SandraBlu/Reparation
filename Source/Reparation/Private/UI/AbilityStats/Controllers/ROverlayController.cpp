// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilityStats/Controllers/ROverlayController.h"

#include "AbilitySystem/RAttributeSet.h"

void UROverlayController::BroadcastInitialValues()
{
	const URAttributeSet* RAttributeSet = CastChecked<URAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(RAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(RAttributeSet->GetMaxHealth());
	OnStaminaChanged.Broadcast(RAttributeSet->GetStamina());
	OnMaxStaminaChanged.Broadcast(RAttributeSet->GetMaxStamina());
}

void UROverlayController::BindCallbacksToDependencies()
{
	const URAttributeSet* RAttributeSet = CastChecked<URAttributeSet>(AttributeSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAttributeSet->GetHealthAttribute()).AddUObject(this, &UROverlayController::HealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAttributeSet->GetMaxHealthAttribute()).AddUObject(this, &UROverlayController::MaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAttributeSet->GetStaminaAttribute()).AddUObject(this, &UROverlayController::StaminaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAttributeSet->GetMaxStaminaAttribute()).AddUObject(this, &UROverlayController::MaxStaminaChanged);
}

void UROverlayController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UROverlayController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UROverlayController::StaminaChanged(const FOnAttributeChangeData& Data) const
{
	OnStaminaChanged.Broadcast(Data.NewValue);
}

void UROverlayController::MaxStaminaChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxStaminaChanged.Broadcast(Data.NewValue);
}
