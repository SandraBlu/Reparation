// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/ROverlayWidgetController.h"

#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/AbilityInfo.h"

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

	if (URAbilitySystemComponent* RASC = Cast<URAbilitySystemComponent>(AbilitySystemComponent))
	{
		if (RASC->bGrantedAbilitiesGiven)
		{
			OnInitializeGrantedAbilities(RASC);
		}
		else
		{
			RASC->AbilityGivenDelegate.AddUObject(this, &UROverlayWidgetController::OnInitializeGrantedAbilities);
		}
		
    	RASC->EffectAssetTags.AddLambda(
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
	
}

void UROverlayWidgetController::OnInitializeGrantedAbilities(URAbilitySystemComponent* RAbilitySystemComp)
{
	if (!RAbilitySystemComp->bGrantedAbilitiesGiven) return;
	
	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this, RAbilitySystemComp](const FGameplayAbilitySpec& AbilitySpec)
	{
		FRAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(RAbilitySystemComp->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = RAbilitySystemComp->GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	RAbilitySystemComp->ForEachAbility(BroadcastDelegate);
}
