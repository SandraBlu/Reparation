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
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetHealthAttribute()).AddUObject(this, &UROverlayWidgetController::HealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetMaxHealthAttribute()).AddUObject(this, &UROverlayWidgetController::MaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetStaminaAttribute()).AddUObject(this, &UROverlayWidgetController::StaminaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetMaxStaminaAttribute()).AddUObject(this, &UROverlayWidgetController::MaxStaminaChanged);
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

void UROverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChange.Broadcast(Data.NewValue);
}

void UROverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChange.Broadcast(Data.NewValue);
}

void UROverlayWidgetController::StaminaChanged(const FOnAttributeChangeData& Data) const
{
	OnStaminaChange.Broadcast(Data.NewValue);
}

void UROverlayWidgetController::MaxStaminaChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxStaminaChange.Broadcast(Data.NewValue);
}
