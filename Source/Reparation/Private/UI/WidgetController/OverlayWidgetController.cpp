// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "Attributes/AOAttributeSet.h"
#include <Abilities/AOAbilitySystemComponent.h>

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


	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnMaxHealthChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnStaminaChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetMaxStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnMaxStaminaChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetEnergyAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnEnergyChange.Broadcast(Data.NewValue); });
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AOAttributeSet->GetMaxEnergyAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {OnMaxEnergyChange.Broadcast(Data.NewValue); });

	Cast<UAOAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
	[this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("message"));
				if (Tag.MatchesTag(MessageTag))
				{
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(DTMessageUI, Tag);
					WidgetMessageRow.Broadcast(*Row);
				}
			}
		}
	);
}


