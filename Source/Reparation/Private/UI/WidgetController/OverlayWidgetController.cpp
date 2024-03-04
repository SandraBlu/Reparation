// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "Attributes/AOAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const UAOAttributeSet* AOAttributeSet = CastChecked<UAOAttributeSet>(AttributeSet);
	OnHealthChange.Broadcast(AOAttributeSet->GetHealth());
	OnMaxHealthChange.Broadcast(AOAttributeSet->GetMaxHealth());
	OnStaminaChange.Broadcast(AOAttributeSet->GetStamina());
	OnMaxStaminaChange.Broadcast(AOAttributeSet->GetMaxStamina());
	OnEnergyChange.Broadcast(AOAttributeSet->GetEnergy());
	OnMaxEnergyChange.Broadcast(AOAttributeSet->GetMaxEnergy());

	
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const UAOAttributeSet* AOAttributeSet = CastChecked<UAOAttributeSet>(AttributeSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HandleHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HandleMaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetStaminaAttribute()).AddUObject(this, &UOverlayWidgetController::HandleStaminaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetMaxStaminaAttribute()).AddUObject(this, &UOverlayWidgetController::HandleMaxStaminaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetEnergyAttribute()).AddUObject(this, &UOverlayWidgetController::HandleEnergyChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetMaxEnergyAttribute()).AddUObject(this, &UOverlayWidgetController::HandleMaxEnergyChanged);
}

void UOverlayWidgetController::HandleHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChange.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::HandleMaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChange.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::HandleStaminaChanged(const FOnAttributeChangeData& Data) const
{
	OnStaminaChange.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxStaminaChange.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::HandleEnergyChanged(const FOnAttributeChangeData& Data) const
{
	OnEnergyChange.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::HandleMaxEnergyChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxEnergyChange.Broadcast(Data.NewValue);
}

