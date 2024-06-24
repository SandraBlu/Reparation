// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/RAbilityMenuController.h"

#include "RGameplayTags.h"
#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/AbilityInfo.h"

void URAbilityMenuController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
	AbilityPointsChange.Broadcast(GetRPS()->GetAbilityPts());
}

void URAbilityMenuController::BindCallbacksToDependencies()
{
	GetRASC()->AbilityStatusChangeDelegate.AddLambda([this] (const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
	{
		if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
		{
			SelectedAbility.Status = StatusTag;
			bool bEnableSpendPoints = false;
			bool bEnableEquip = false;
			ShouldEnableButtons(StatusTag, CurrentAbilityPoints, bEnableSpendPoints, bEnableEquip);
			FString Description;
			FString NextLevelDescription;
			GetRASC()->GetDescriptionByAbilityTag(AbilityTag, Description, NextLevelDescription);
			AbilitySelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
		}
		
		if (AbilityInfo)
		{
			FRAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});

	GetRPS()->OnAbilityPtsChangeDelegate.AddLambda([this] (int32 AbilityPoints)
	{
		AbilityPointsChange.Broadcast(AbilityPoints);
		CurrentAbilityPoints = AbilityPoints;

		bool bEnableSpendPoints = false;
		bool bEnableEquip = false;
		ShouldEnableButtons(SelectedAbility.Status, CurrentAbilityPoints, bEnableSpendPoints, bEnableEquip);
		FString Description;
		FString NextLevelDescription;
		GetRASC()->GetDescriptionByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
		AbilitySelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
	});
}

void URAbilityMenuController::AbilitySelected(const FGameplayTag& AbilityTag)
{
	const FRGameplayTags GameplayTags = FRGameplayTags::Get();
	const int32 AbilityPoints = GetRPS()->GetAbilityPts();
	FGameplayTag AbilityStatus;

	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.ability_none);
	const FGameplayAbilitySpec* AbilitySpec = GetRASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.ability_status_locked;
	}
	else 
	{
		AbilityStatus = GetRASC()->GetStatusFromSpec(*AbilitySpec);
	}
	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;
	bool bEnableSpendPoints = false;
	bool bEnableEquip = false;
	ShouldEnableButtons(AbilityStatus, AbilityPoints, bEnableSpendPoints, bEnableEquip);
	FString Description;
	FString NextLevelDescription;
	GetRASC()->GetDescriptionByAbilityTag(AbilityTag, Description, NextLevelDescription);
	AbilitySelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
}

void URAbilityMenuController::SpendPointButtonPressed()
{
	if (GetRASC())
	{
		GetRASC()->ServerSpendAbilityPoint(SelectedAbility.Ability);
	}
}

void URAbilityMenuController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 AbilityPoints,
                                                  bool& bEnableSpendPointButton, bool& bEnableEquipButton)
{
	const FRGameplayTags GameplayTags = FRGameplayTags::Get();

	bEnableSpendPointButton = false;
	bEnableEquipButton = false;
	if (AbilityStatus.MatchesTagExact(GameplayTags.ability_status_equipped))
	{
		bEnableEquipButton = true;
		if (AbilityPoints > 0)
		{
			bEnableSpendPointButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.ability_status_available))
	{
		if (AbilityPoints > 0)
		{
			bEnableSpendPointButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.ability_status_unlocked))
	{
		bEnableEquipButton = true;
		if (AbilityPoints > 0)
		{
			bEnableSpendPointButton = true;
		}
	}
}
