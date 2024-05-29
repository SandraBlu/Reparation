// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilityStats/Controllers/ROverlayController.h"

#include "AbilitySystem/RAbilitySystemComponent.h"
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
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAttributeSet->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAttributeSet->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAttributeSet->GetStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnStaminaChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAttributeSet->GetMaxStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxStaminaChanged.Broadcast(Data.NewValue);});

	Cast<URAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda([this](const FGameplayTagContainer& AssetTags)
	{
		for (const FGameplayTag& Tag : AssetTags)
		{
			FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("message"));
			if (Tag.MatchesTag(MessageTag))
			{
				const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
				MessageRowDelegate.Broadcast(*Row);	
			}
		}
	});
}
