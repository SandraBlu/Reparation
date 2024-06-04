// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/ROverlayWidgetController.h"

#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"

void UROverlayWidgetController::BroadcastInitialValues()
{
	const URAttributeSet* Attributes = CastChecked<URAttributeSet>(AttributeSet);
	OnHealthChange.Broadcast(Attributes->GetHealth());
	OnMaxHealthChange.Broadcast(Attributes->GetMaxHealth());
	OnStaminaChange.Broadcast(Attributes->GetStamina());
	OnMaxStaminaChange.Broadcast(Attributes->GetMaxStamina());
}

void UROverlayWidgetController::BindCallbacksToDependencies()
{
	const URAttributeSet* Attributes = CastChecked<URAttributeSet>(AttributeSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnStaminaChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetMaxStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxStaminaChange.Broadcast(Data.NewValue);});
	Cast<URAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("message"));
				if (Tag.MatchesTag(MessageTag))
				{
					const FUIMessageRow* Row = GetDataTableRowByTag<FUIMessageRow>(MessageWidgetDataTable, Tag);
					MessageWidgetDelegate.Broadcast(*Row);
				}
			}
		}
	);
}