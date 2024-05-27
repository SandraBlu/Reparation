// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Controller/RHUDController.h"
#include "AbilitySystem/RAttributeSet.h"
#include "AbilitySystem/RAbilitySystemComponent.h"
#include "RGameplayTags.h"

void URHUDController::BroadcastInitialValues()
{
	OnHealthChange.Broadcast(GetRAS()->GetHealth());
	OnMaxHealthChange.Broadcast(GetRAS()->GetMaxHealth());
	OnStaminaChange.Broadcast(GetRAS()->GetStamina());
	OnMaxStaminaChange.Broadcast(GetRAS()->GetMaxStamina());
	OnEnergyChange.Broadcast(GetRAS()->GetEnergy());
	OnMaxEnergyChange.Broadcast(GetRAS()->GetMaxEnergy());
}

void URHUDController::BindCallbacksToDependencies()
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnHealthChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnMaxHealthChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnStaminaChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetMaxStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnMaxStaminaChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetEnergyAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnEnergyChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetMaxEnergyAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnMaxEnergyChange.Broadcast(Data.NewValue); });
	
	GetRASC()->EffectTags.AddLambda([this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("message"));
				if(Tag.MatchesTag(MessageTag))
				{
					const FMessageTableRow* Row = GetDataTableRowByTag<FMessageTableRow>(DTEffectMessages, Tag);
					MessageRowDelegate.Broadcast(*Row);	
				}
			}
		}
	);
}
