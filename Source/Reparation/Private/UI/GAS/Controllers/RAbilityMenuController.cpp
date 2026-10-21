// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/Controllers/RAbilityMenuController.h"

#include "RGameplayTags.h"
#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/AbilityInfo.h"
#include "GAS/Data/LevelUpInfo.h"
#include "UI/GAS/RSkillBrowser.h"
#include "UI/GAS/RSkillMenuStyle.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

TArray<FRAbilityInfo> URAbilityMenuController::GetSkillsForDiscipline(ERSkillDiscipline Discipline) const
{
	TArray<FRAbilityInfo> Result;
	if (AbilityInfo) for (const FRAbilityInfo& Entry : AbilityInfo->AbilityInfo) if (Entry.Discipline == Discipline && Entry.bSkillTreePurchasable) Result.Add(Entry);
	Result.Sort([](const FRAbilityInfo& A, const FRAbilityInfo& B) { return A.LevelRequirement < B.LevelRequirement; });
	return Result;
}
FRAbilityInfo URAbilityMenuController::GetSkillDefinition(FGameplayTag Skill) const { return AbilityInfo ? AbilityInfo->FindAbilityInfoForTag(Skill) : FRAbilityInfo(); }
int32 URAbilityMenuController::GetSkillPoints() { return GetRPS()->GetAbilityPts(); }
URSkillMenuStyle* URAbilityMenuController::GetSkillMenuStyle() const
{
	URSkillMenuStyle* Theme = SkillMenuStyle.IsNull() ? nullptr : SkillMenuStyle.LoadSynchronous();
	return Theme ? Theme : GetMutableDefault<URSkillMenuStyle>();
}
void URAbilityMenuController::OpenSkillBrowser()
{
	if (!SkillMenuCanvas) return;
	if (bWaitingForEquipSelection) AbilityDeselect(); // Cancel pending slot selection when changing views.
	if (!SkillBrowser)
	{
		UClass* BrowserClass = SkillBrowserClass.IsNull() ? nullptr : SkillBrowserClass.LoadSynchronous();
		SkillBrowser = CreateWidget<URSkillBrowser>(PlayerController, BrowserClass ? BrowserClass : URSkillBrowser::StaticClass());
		if (!SkillBrowser) return;
		SkillBrowser->Controller = this;
		SkillBrowser->MenuStyle = GetSkillMenuStyle();
		UCanvasPanelSlot* Slot = SkillMenuCanvas->AddChildToCanvas(SkillBrowser);
		Slot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		Slot->SetOffsets(FMargin(12.f));
		Slot->SetZOrder(10);
	}
	for (UWidget* Widget : LegacySkillMenuWidgets) if (Widget) Widget->SetVisibility(ESlateVisibility::Collapsed);
	SkillBrowser->SetVisibility(ESlateVisibility::Visible);
	if (LearnViewButton) LearnViewButton->SetSelected(true);
	if (LoadoutViewButton) LoadoutViewButton->SetSelected(false);
}

void URAbilityMenuController::CloseSkillBrowser()
{
	// Kept for saved Blueprint compatibility; there is no retired Loadout page to return to.
	if (!LoadoutViewButton) return;
	if (SkillBrowser) SkillBrowser->SetVisibility(ESlateVisibility::Collapsed);
	for (int32 Index = 0; Index < LegacySkillMenuWidgets.Num(); ++Index)
		if (LegacySkillMenuWidgets[Index] && LegacySkillMenuVisibilities.IsValidIndex(Index))
			LegacySkillMenuWidgets[Index]->SetVisibility(LegacySkillMenuVisibilities[Index]);
	if (LearnViewButton) LearnViewButton->SetSelected(false);
	if (LoadoutViewButton) LoadoutViewButton->SetSelected(true);
}

void URAbilityMenuController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
	CurrentAbilityPoints = GetRPS()->GetAbilityPts();
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
			AbilitySelectedDelegate.Broadcast(false, bEnableEquip, Description, FString());
		}
		
		if (AbilityInfo)
		{
			FRAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});

	GetRASC()->AbilityEquippedDelegate.AddUObject(this, &URAbilityMenuController::OnAbilityEquipped);
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
		AbilitySelectedDelegate.Broadcast(false, bEnableEquip, Description, FString());
	});
}

void URAbilityMenuController::AbilitySelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType;
		StopWaitEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
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
	AbilitySelectedDelegate.Broadcast(false, bEnableEquip, Description, FString());
}

void URAbilityMenuController::SpendPointButtonPressed()
{
	// Saved Blueprint compatibility only. Purchasing belongs to the skill tree.
}

void URAbilityMenuController::AbilityDeselect()
{
	if (bWaitingForEquipSelection)
	{
		FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.Ability = FRGameplayTags::Get().ability_none;
	SelectedAbility.Status = FRGameplayTags::Get().ability_status_locked;
	AbilitySelectedDelegate.Broadcast(false, false, FString(), FString());
}

void URAbilityMenuController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	WaitEquipDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection =true;

	const FGameplayTag SelectedStatus = GetRASC()->GetStatusFromAbilityTag(SelectedAbility.Ability);
	if (SelectedStatus.MatchesTagExact(FRGameplayTags::Get().ability_status_equipped))
	{
		SelectedSlot = GetRASC()->GetInputTagFromAbilityTag(SelectedAbility.Ability);
	}
}

void URAbilityMenuController::EquippedRowInputPressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection) return;
	//check selected ability against slot ability type
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;

	GetRASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
	
}

void URAbilityMenuController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,	const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	//Clear Out Old Slot
	bWaitingForEquipSelection = false;
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

	StopWaitEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	AbilityReassignedDelegate.Broadcast(AbilityTag);
	AbilityDeselect();
}

void URAbilityMenuController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 AbilityPoints, bool& bEnableSpendPointButton, bool& bEnableEquipButton)
{
    bEnableSpendPointButton = false; // Kept in the delegate signature for saved Blueprint compatibility.
    const FRGameplayTags& Tags = FRGameplayTags::Get();
    const FRAbilityInfo Info = GetSkillDefinition(SelectedAbility.Ability);
    bEnableEquipButton = (AbilityStatus == Tags.ability_status_equipped || AbilityStatus == Tags.ability_status_unlocked)
        && Info.AbilityType != FGameplayTag::RequestGameplayTag(TEXT("ability.type.technique"));
}
