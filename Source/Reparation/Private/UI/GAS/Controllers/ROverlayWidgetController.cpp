// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/ROverlayWidgetController.h"

#include "RGameplayTags.h"
#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/AbilityInfo.h"
#include "GAS/Data/LevelUpInfo.h"

void UROverlayWidgetController::BroadcastInitialValues()
{
	
	OnHealthChange.Broadcast(GetRAS()->GetHealth());
	OnMaxHealthChange.Broadcast(GetRAS()->GetMaxHealth());
	OnStaminaChange.Broadcast(GetRAS()->GetStamina());
	OnMaxStaminaChange.Broadcast(GetRAS()->GetMaxStamina());
}

void UROverlayWidgetController::BindCallbacksToDependencies()
{
	//XP Change
	GetRPS()->OnXPChangeDelegate.AddUObject(this, &UROverlayWidgetController::OnXPChange);
	GetRPS()->OnLevelChangeDelegate.AddLambda([this](int32 NewLevel)
	{
		OnLevelChangeDelegate.Broadcast(NewLevel);
	});
	//Attribute Change
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnStaminaChange.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRAS()->GetMaxStaminaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxStaminaChange.Broadcast(Data.NewValue);});

	//Ability Change
	if (GetRASC())
	{
		GetRASC()->AbilityEquippedDelegate.AddUObject(this, &UROverlayWidgetController::OnAbilityEquipped);
		if (GetRASC()->bGrantedAbilitiesGiven)
		{
			BroadcastAbilityInfo();
		}
		else
		{
			GetRASC()->AbilityGivenDelegate.AddUObject(this, &UROverlayWidgetController::BroadcastAbilityInfo);
		}

		//Effect Message
    	GetRASC()->EffectAssetTags.AddLambda(
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

void UROverlayWidgetController::OnXPChange(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetRPS()->LevelUpInfo;
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

void UROverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot) const
{
	//Clear Out Old Slot
	const FRGameplayTags Tag = FRGameplayTags::Get();
	FRAbilityInfo PrevSlotInfo;
	PrevSlotInfo.StatusTag = Tag.ability_status_unlocked;
	PrevSlotInfo.InputTag = PrevSlot;
	PrevSlotInfo.AbilityTag =Tag.ability_none;
	AbilityInfoDelegate.Broadcast(PrevSlotInfo);
	
	//Fill new slot
	FRAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}
