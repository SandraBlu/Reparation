// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/ROverlayWidgetController.h"

#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/AbilityInfo.h"
#include "GAS/Data/LevelUpInfo.h"

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
	//XP Change
	ARPlayerState* RPlayerState = CastChecked<ARPlayerState>(PlayerState);
	RPlayerState->OnXPChangeDelegate.AddUObject(this, &UROverlayWidgetController::OnXPChange);
	RPlayerState->OnLevelChangeDelegate.AddLambda([this](int32 NewLevel)
	{
		OnLevelChangeDelegate.Broadcast(NewLevel);
	});
	//Attribute Change
	const URAttributeSet* Attributes = CastChecked<URAttributeSet>(AttributeSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnStaminaChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attributes->GetMaxStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxStaminaChange.Broadcast(Data.NewValue);});

	//Ability Change
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

		//Effect Message
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

void UROverlayWidgetController::OnXPChange(int32 NewXP) const
{
	const ARPlayerState* RPlayerState = CastChecked<ARPlayerState>(PlayerState);
	const ULevelUpInfo* LevelUpInfo = RPlayerState->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unable to find XPinfo, Enter in BP_PlayerState"));

	const int32 CurrentLevel = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInfo.Num();

	if (CurrentLevel <= MaxLevel && CurrentLevel >0)
	{
		const int32 LevelUpReq = LevelUpInfo->LevelUpInfo[CurrentLevel].LevelUpRequirement;
		const int32 PrevLevelUpReq = LevelUpInfo->LevelUpInfo[CurrentLevel -1].LevelUpRequirement;

		const int32 DeltaLevelReq = LevelUpReq - PrevLevelUpReq;
		const int32 XPForThisLevel = NewXP - PrevLevelUpReq;

		const float XPBarPercent = static_cast<float>(XPForThisLevel)/ static_cast<float>(DeltaLevelReq);

		OnXPPercentChangeDelegate.Broadcast(XPBarPercent);
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
