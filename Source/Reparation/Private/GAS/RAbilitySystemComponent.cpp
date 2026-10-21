// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Ability/RElementalMastery.h"
#include "GAS/Ability/RMeleeAbility.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"

void URAbilitySystemComponent::PreserveDebuffPeriod(FActiveGameplayEffectHandle Handle, float NextTickDelay)
{
	FActiveGameplayEffect* Active = const_cast<FActiveGameplayEffect*>(GetActiveGameplayEffect(Handle));
	if (!Active || !GetOwner()->HasAuthority() || NextTickDelay <= 0.f || Active->Spec.GetPeriod() <= 0.f) return;
	GetWorld()->GetTimerManager().SetTimer(Active->PeriodHandle, FTimerDelegate::CreateUObject(this, &URAbilitySystemComponent::ExecutePeriodicEffect, Handle), Active->Spec.GetPeriod(), true, NextTickDelay);
}

#include "AbilitySystemBlueprintLibrary.h"
#include "RGameplayTags.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GAS/Ability/RGameplayAbility.h"
#include "GAS/Data/AbilityInfo.h"
#include "Interfaces/RPlayerInterface.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "GAS/Data/LevelUpInfo.h"
#include "GAS/RAttributeSet.h"

void URAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URAbilitySystemComponent::ClientEffectApplied);
}

void URAbilitySystemComponent::AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass  : GrantedAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAbilityInfo* Data = GetSkillData(); Cast<ARPlayer>(GetAvatarActor()) && Data)
		{
			const FRAbilityInfo Info = Data->FindAbilityInfoForTag(GetAbilityTagFromSpec(AbilitySpec));
			if (Info.AbilityTag.IsValid() && (!Info.bSkillTreePurchasable || Info.AbilityTag == Data->AutomaticTierReward)) continue;
		}
		if (const URGameplayAbility* RAbility = Cast<URGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(RAbility->StartupInputTag);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FRGameplayTags::Get().ability_status_equipped);
			GiveAbility(AbilitySpec);
		}
	}
	bGrantedAbilitiesGiven = true;
	AbilityGivenDelegate.Broadcast();
}

void URAbilitySystemComponent::AddPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& PassiveAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass  : PassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		const UAbilityInfo* Data = GetSkillData();
		const FRAbilityInfo Info = Data ? Data->FindAbilityInfoForTag(GetAbilityTagFromSpec(AbilitySpec)) : FRAbilityInfo();
		// Gear-only effects are granted by equipment, not by the player's skill loadout startup list.
		if (Cast<ARPlayer>(GetAvatarActor()) && Info.AbilityTag.IsValid() && !Info.bSkillTreePurchasable) continue;
		if (Cast<ARPlayer>(GetAvatarActor()) && Info.AbilityType == FRGameplayTags::Get().ability_type_passive)
		{
			if (GetSpecFromAbilityTag(Info.AbilityTag)) continue;
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FRGameplayTags::Get().ability_status_unlocked);
			GiveAbility(AbilitySpec);
			const auto& T = FRGameplayTags::Get();
			for (FGameplayTag Slot : {T.InputTag_Passive01, T.InputTag_Passive02, T.InputTag_Passive03, T.InputTag_Passive04})
				if (IsLoadoutSlotUnlocked(Slot) && InputSlotIsEmpty(Slot)) { ServerEquipAbility_Implementation(Info.AbilityTag, Slot); break; }
		}
		else GiveAbilityAndActivateOnce(AbilitySpec); // Preserve non-loadout innate effects and NPC setup.
	}
}

void URAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	const FRGameplayTags GameplayTags = FRGameplayTags::Get();
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (auto& AbilitySpec : GetActivatableAbilities())
	{
		if(!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)) continue;
		if (AbilitySpec.IsActive())
		{
			if (URMeleeAbility* Melee = Cast<URMeleeAbility>(AbilitySpec.GetPrimaryInstance()))
			{
				Melee->QueueComboAttack();
				if (!IsOwnerActorAuthoritative()) ServerQueueMeleeCombo(AbilitySpec.Handle);
				continue;
			}
		}
		if (InputTag.MatchesTag(GameplayTags.InputTag_Toggle) && AbilitySpec.IsActive())
		{
			CancelAbilityHandle(AbilitySpec.Handle);
		}
		else
		{
			TryActivateAbility(AbilitySpec.Handle);
		}
	}
}

void URAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	const FRGameplayTags GameplayTags = FRGameplayTags::Get();
	
	if (!InputTag.IsValid() || !InputTag.MatchesTag(GameplayTags.InputTag_MustBeHeld))
	{
		return;
	}

	FScopedAbilityListLock ActiveScopeLoc(*this);
 	for (auto& AbilitySpec : GetActivatableAbilities())
 	{
 		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
 		{
 			CancelAbilityHandle(AbilitySpec.Handle);
 			AbilitySpecInputReleased(AbilitySpec);
			// Instanced abilities each hold their own activation info; the copy on the
			// spec is deprecated and only ever applied to non-instanced abilities.
			FPredictionKey ActivationPredictionKey;
			if (const UGameplayAbility* PrimaryInstance = AbilitySpec.GetPrimaryInstance())
			{
				ActivationPredictionKey = PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey();
			}
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, ActivationPredictionKey);
 		}
 	}
}

void URAbilitySystemComponent::ServerQueueMeleeCombo_Implementation(FGameplayAbilitySpecHandle Handle)
{
	if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle); Spec && Spec->IsActive())
		if (URMeleeAbility* Melee = Cast<URMeleeAbility>(Spec->GetPrimaryInstance())) Melee->QueueComboAttack();
}

void URAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag URAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->GetAssetTags())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("ability"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag URAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag URAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag("ability.status")))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

FGameplayTag URAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayTag URAbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

bool URAbilitySystemComponent::InputSlotIsEmpty(const FGameplayTag& InputSlot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasInputSlot(AbilitySpec, InputSlot))
		{
			return false;
		}
	}
	return true;
}

bool URAbilitySystemComponent::AbilityHasInputSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& InputSlot)
{
	return Spec.GetDynamicSpecSourceTags().HasTagExact(InputSlot);
}

bool URAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.GetDynamicSpecSourceTags().HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

FGameplayAbilitySpec* URAbilitySystemComponent::GetSpecWithInputSlot(const FGameplayTag& InputSlot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputSlot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

bool URAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	const UAbilityInfo* AbilityInfo = GetSkillData();
	if (!AbilityInfo) return false;
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FRAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;
	return AbilityType.MatchesTagExact(FRGameplayTags::Get().ability_type_passive);
}

void URAbilitySystemComponent::AssignInputSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& InputSlot)
{
	ClearSlot(&Spec);
	Spec.GetDynamicSpecSourceTags().AddTag(InputSlot);
}

void URAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag,
	bool bActivate)
{
	ActivatePassiveEffectDelegate.Broadcast(AbilityTag, bActivate);
}

FGameplayAbilitySpec* URAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for(FGameplayTag Tag : AbilitySpec.Ability.Get()->GetAssetTags())
		{
			if (Tag.MatchesTagExact(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void URAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<URPlayerInterface>())
	{
		if (IRPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void URAbilitySystemComponent::UpdateAbilityStatus(int32 Level)
{
	const UAbilityInfo* AbilityInfo = GetSkillData();
	if (!AbilityInfo) return;
	for (const FRAbilityInfo& Info : AbilityInfo->AbilityInfo)
	{
		if (!Info.AbilityTag.IsValid() || !Info.Ability || !Info.bSkillTreePurchasable)continue;
		if (Level < Info.LevelRequirement) continue;
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FRGameplayTags::Get().ability_status_available);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec);
			ClientUpdateAbilityStatus(Info.AbilityTag, FRGameplayTags::Get().ability_status_available, 1);
		}
	}
	RefreshTierReward();
}

void URAbilitySystemComponent::ServerSpendAbilityPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (!GetSkillUpgradeIssue(AbilityTag).IsEmpty()) return;
	ARPlayer* Player = Cast<ARPlayer>(GetAvatarActor());
	ARPlayerState* PS = Player ? Player->GetPlayerState<ARPlayerState>() : nullptr;
	if (!PS || PS->GetAbilityPts() <= 0) return;
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FRGameplayTags GameplayTags = FRGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		const bool bAvailable = Status.MatchesTagExact(GameplayTags.ability_status_available);
		if (!bAvailable && !Status.MatchesTagExact(GameplayTags.ability_status_equipped) && !Status.MatchesTagExact(GameplayTags.ability_status_unlocked)) return;
		const UAbilityInfo* Info = GetSkillData();
		const FRAbilityInfo Entry = Info ? Info->FindAbilityInfoForTag(AbilityTag) : FRAbilityInfo();
		if (Entry.LevelRequirement > PS->GetPlayerLevel()) return;
		const int32 ProgressionCap = PS->LevelUpInfo ? PS->LevelUpInfo->GetMaxLevel() : FMath::Max(1, PS->GetPlayerLevel());
		const int32 RankCap = Entry.MaximumAbilityLevel > 0 ? FMath::Min(Entry.MaximumAbilityLevel, ProgressionCap) : ProgressionCap;
		if (!bAvailable && AbilitySpec->Level >= RankCap) return;
		const int32 Cost = FMath::Clamp(Entry.SkillPointCost, 1, 5);
		FRSkillPurchaseRecord* Record = SkillPurchases.FindByPredicate([AbilityTag](const FRSkillPurchaseRecord& R) { return R.Skill == AbilityTag; });
		if (!Record)
		{
			FRSkillPurchaseRecord NewRecord; NewRecord.Skill = AbilityTag; NewRecord.BaselineRank = GetLearnedSkillRank(AbilityTag);
			Record = &SkillPurchases.Add_GetRef(NewRecord);
		}
		Record->Points += Cost;
		if (Entry.bCountsTowardsCoreInvestment) Record->CorePoints += Cost;
		PS->AddToAbilityPts(-Cost);
		if (Entry.bCountsTowardsCoreInvestment)
			CoreSkillPointsSpent = static_cast<int32>(FMath::Min<int64>(MAX_int32, static_cast<int64>(CoreSkillPointsSpent) + Cost));
		if (Status.MatchesTagExact(GameplayTags.ability_status_available))
		{
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.ability_status_available);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.ability_status_unlocked);
			Status = GameplayTags.ability_status_unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.ability_status_equipped) || Status.MatchesTagExact(GameplayTags.ability_status_unlocked))
		{
			AbilitySpec->Level += 1;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
		UpdateAbilityStatus(PS->GetPlayerLevel());
	}
}

const UAbilityInfo* URAbilitySystemComponent::GetSkillData() const { return SkillDefinitions ? SkillDefinitions.Get() : URAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor()); }

bool URAbilitySystemComponent::IsSkillQuestEligible(FName UnlockId) const
{
	if (UnlockId.IsNone()) return false;
	const UAbilityInfo* Data = GetSkillData();
	const ARPlayer* Player = Cast<ARPlayer>(GetAvatarActor());
	const ARPlayerState* PS = Player ? Player->GetPlayerState<ARPlayerState>() : nullptr;
	if (!Data || !PS) return false;
	for (const FRSkillQuestUnlock& Unlock : Data->QuestUnlocks)
		if (Unlock.Id == UnlockId) return PS->GetPlayerLevel() >= Unlock.OfferAtLevel || CoreSkillPointsSpent >= Unlock.OfferAtCorePointsSpent;
	return false;
}

void URAbilitySystemComponent::GrantProgressionUnlock(FName UnlockId)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || UnlockId.IsNone()) return;
	const UAbilityInfo* Data = GetSkillData();
	if (Data && Data->QuestUnlocks.ContainsByPredicate([UnlockId](const FRSkillQuestUnlock& Unlock) { return Unlock.Id == UnlockId; }))
		ProgressionUnlocks.AddUnique(UnlockId);
}

int32 URAbilitySystemComponent::GetLearnedSkillRank(const FGameplayTag& AbilityTag)
{
	if (!AbilityTag.IsValid()) return 0;
	const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag);
	if (!Spec) return 0;
	const FGameplayTag Status = GetStatusFromSpec(*Spec);
	const FRGameplayTags& Tags = FRGameplayTags::Get();
	return Status == Tags.ability_status_unlocked || Status == Tags.ability_status_equipped ? Spec->Level : 0;
}

FText URAbilitySystemComponent::GetSkillUpgradeIssue(const FGameplayTag& AbilityTag)
{
	const ARPlayer* Player = Cast<ARPlayer>(GetAvatarActor());
	const ARPlayerState* PS = Player ? Player->GetPlayerState<ARPlayerState>() : nullptr;
	if (!PS) return FText::FromString(TEXT("Player progression is unavailable."));
	const UAbilityInfo* Data = GetSkillData();
	const FRAbilityInfo Entry = Data ? Data->FindAbilityInfoForTag(AbilityTag) : FRAbilityInfo();
	if (Data && (!Entry.AbilityTag.IsValid() || !Entry.Ability)) return FText::FromString(TEXT("This technique is not available yet."));
	if (!Entry.bSkillTreePurchasable) return FText::FromString(Data && AbilityTag == Data->AutomaticTierReward
		? TEXT("Granted automatically after completing the required tier; no purchase needed.") : TEXT("This effect is not purchased through the skill tree."));
	if (PS->GetPlayerLevel() < Entry.LevelRequirement) return FText::FromString(FString::Printf(TEXT("Requires character level %d."), Entry.LevelRequirement));
	if (!IsBranchTierUnlocked(Entry.Discipline, Entry.BranchTier, Entry.PreviousTierSkillsRequired))
		return FText::FromString(FString::Printf(TEXT("Learn %d skill(s) in the preceding branch tier first."), FMath::Max(1, Entry.PreviousTierSkillsRequired)));
	if (CoreSkillPointsSpent < Entry.MinimumCorePointsSpent)
		return FText::FromString(FString::Printf(TEXT("Invest %d more point(s) across Stealth, Melee or Archer."), Entry.MinimumCorePointsSpent - CoreSkillPointsSpent));
	if (!Entry.RequiredProgressionUnlock.IsNone() && !ProgressionUnlocks.Contains(Entry.RequiredProgressionUnlock))
	{
		FString Name = Entry.RequiredProgressionUnlock.ToString();
		if (Data) for (const FRSkillQuestUnlock& Unlock : Data->QuestUnlocks)
			if (Unlock.Id == Entry.RequiredProgressionUnlock) Name = Unlock.DisplayName.ToString();
		return FText::FromString(FString::Printf(TEXT("Complete %s. %s"), *Name,
			IsSkillQuestEligible(Entry.RequiredProgressionUnlock) ? TEXT("Quest eligible; quest content is not authored yet.") : TEXT("Quest eligibility requires more levels or branch investment.")));
	}
	for (const FRSkillPrerequisite& Required : Entry.Prerequisites)
	{
		if (GetLearnedSkillRank(Required.AbilityTag) < FMath::Max(1, Required.RequiredRank))
		{
			const FRAbilityInfo Prerequisite = Data->FindAbilityInfoForTag(Required.AbilityTag);
			const FString Name = Prerequisite.DisplayName.IsEmpty() ? Required.AbilityTag.ToString() : Prerequisite.DisplayName.ToString();
			return FText::FromString(FString::Printf(TEXT("Requires %s rank %d."), *Name, Required.RequiredRank));
		}
	}
	const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag);
	if (!Spec) return FText::FromString(TEXT("This technique is not available yet."));
	const FGameplayTag Status = GetStatusFromSpec(*Spec);
	const FRGameplayTags& Tags = FRGameplayTags::Get();
	if (Status != Tags.ability_status_available && Status != Tags.ability_status_unlocked && Status != Tags.ability_status_equipped)
		return FText::FromString(TEXT("This technique is locked."));
	const int32 Cap = PS->LevelUpInfo ? PS->LevelUpInfo->GetMaxLevel() : FMath::Max(1, PS->GetPlayerLevel());
	const int32 RankCap = Entry.MaximumAbilityLevel > 0 ? FMath::Min(Cap, Entry.MaximumAbilityLevel) : Cap;
	if (GetLearnedSkillRank(AbilityTag) >= RankCap) return FText::FromString(TEXT("Maximum rank reached."));
	if (PS->GetAbilityPts() < FMath::Clamp(Entry.SkillPointCost, 1, 5)) return FText::FromString(FString::Printf(TEXT("Requires %d skill point(s)."), FMath::Clamp(Entry.SkillPointCost, 1, 5)));
	return FText::GetEmpty();
}

void URAbilitySystemComponent::BuildSkillDescriptions(const FGameplayTag& AbilityTag, FString& Current, FString& Next)
{
	const UAbilityInfo* Data = GetSkillData();
	if (!Data) return;
	const FRAbilityInfo Entry = Data->FindAbilityInfoForTag(AbilityTag);
	if (!Entry.AbilityTag.IsValid()) return;
	const int32 Rank = GetLearnedSkillRank(AbilityTag);
	const ARPlayer* Player = Cast<ARPlayer>(GetAvatarActor());
	const ARPlayerState* PS = Player ? Player->GetPlayerState<ARPlayerState>() : nullptr;
	const int32 Cap = PS && PS->LevelUpInfo ? PS->LevelUpInfo->GetMaxLevel() : 40;
	const int32 RankCap = Entry.MaximumAbilityLevel > 0 ? FMath::Min(Cap, Entry.MaximumAbilityLevel) : Cap;
	if (!Entry.DisplayName.IsEmpty()) Current = Entry.DisplayName.ToString() + FString::Printf(TEXT(" — Rank %d/%d\n"), Rank, RankCap) + Current;
	if (!Entry.Summary.IsEmpty()) Current += TEXT("\n") + Entry.Summary.ToString();
	if (Entry.RankDescriptions.IsValidIndex(Rank - 1)) Current += TEXT("\n") + Entry.RankDescriptions[Rank - 1].ToString();
	if (Rank >= RankCap) Next = TEXT("Maximum rank reached.");
	else
	{
		if (Entry.RankDescriptions.IsValidIndex(Rank)) Next = Entry.RankDescriptions[Rank].ToString();
		Next += FString::Printf(TEXT("\nCost: %d skill point(s)."), FMath::Clamp(Entry.SkillPointCost, 1, 5));
		const FText Issue = GetSkillUpgradeIssue(AbilityTag);
		if (!Issue.IsEmpty()) Next += TEXT("\n") + Issue.ToString();
	}
}

void URAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	ARPlayer* Player = Cast<ARPlayer>(GetAvatarActor());
	ARPlayerState* PS = Player ? Player->GetPlayerState<ARPlayerState>() : nullptr;
	const URAttributeSet* Attributes = PS ? Cast<URAttributeSet>(PS->GetAttributeSet()) : nullptr;
	const FGameplayTag Primary = FGameplayTag::RequestGameplayTag(TEXT("Attributes.Primary"), false);
	if (!PS || PS->GetAttributePts() <= 0 || !Attributes || !Attributes->TagsToAttributes.Contains(AttributeTag) || !AttributeTag.MatchesTag(Primary)) return;
	PS->AddToAttributePts(-1);
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);
}

void URAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& Slot)
{
	if (!GetEquipIssue(AbilityTag, Slot).IsEmpty()) return;
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FRGameplayTags& Tag = FRGameplayTags::Get();
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		const bool bStatusValid = Status == Tag.ability_status_equipped || Status == Tag.ability_status_unlocked;
		if (bStatusValid)
		{
		//HandleActivation/Deactivation for passive abilities
			if (!InputSlotIsEmpty(Slot))
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithInputSlot(Slot);
				if (SpecWithSlot)
				{
					//if same ability return early
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, Tag.ability_status_equipped, Slot, PrevSlot);
						return;
					}
					if (IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbilityDelegate.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
						CancelAbilityHandle(SpecWithSlot->Handle);
					}
					ClearSlot(SpecWithSlot);
					SpecWithSlot->GetDynamicSpecSourceTags().RemoveTag(Tag.ability_status_equipped);
					SpecWithSlot->GetDynamicSpecSourceTags().AddTag(Tag.ability_status_unlocked);
					MarkAbilitySpecDirty(*SpecWithSlot);
					ClientUpdateAbilityStatus(GetAbilityTagFromSpec(*SpecWithSlot), Tag.ability_status_unlocked, SpecWithSlot->Level);
				}
			}
			//new ability
			if (!AbilityHasAnySlot(*AbilitySpec))
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
			}
			AssignInputSlotToAbility(*AbilitySpec, Slot);
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(Tag.ability_status_unlocked);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(Tag.ability_status_equipped);
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		ClientEquipAbility(AbilityTag, Tag.ability_status_equipped, Slot, PrevSlot);
	}
}

int32 URAbilitySystemComponent::GetPassiveSlotUnlockLevel(int32 Index) const
{
	const UAbilityInfo* Data = GetSkillData();
	return Index >= 0 && Index < 4 && Data && Data->PassiveSlotUnlockLevels.IsValidIndex(Index)
		? FMath::Max(1, Data->PassiveSlotUnlockLevels[Index]) : MAX_int32;
}

bool URAbilitySystemComponent::IsLoadoutSlotUnlocked(FGameplayTag Slot) const
{
	const auto& Tags = FRGameplayTags::Get();
	if (Slot == Tags.InputTag_LMB || Slot == Tags.InputTag_RMB || Slot == Tags.InputTag_1 || Slot == Tags.InputTag_2) return true;
	const TArray<FGameplayTag> Passive{Tags.InputTag_Passive01, Tags.InputTag_Passive02, Tags.InputTag_Passive03, Tags.InputTag_Passive04};
	const int32 Index = Passive.IndexOfByKey(Slot);
	const ARPlayerState* PS = Cast<ARPlayerState>(GetOwnerActor());
	if (!PS) if (const ARPlayer* Player = Cast<ARPlayer>(GetAvatarActor())) PS = Player->GetPlayerState<ARPlayerState>();
	return Index != INDEX_NONE && PS && PS->GetPlayerLevel() >= GetPassiveSlotUnlockLevel(Index);
}

FText URAbilitySystemComponent::GetEquipIssue(FGameplayTag AbilityTag, FGameplayTag Slot)
{
	if (GetLearnedSkillRank(AbilityTag) <= 0) return FText::FromString(TEXT("Learn this skill first."));
	const UAbilityInfo* Data = GetSkillData();
	const FRAbilityInfo Info = Data ? Data->FindAbilityInfoForTag(AbilityTag) : FRAbilityInfo();
	const auto& Tags = FRGameplayTags::Get();
	if (!Info.bSkillTreePurchasable) return FText::FromString(TEXT("This effect does not use a skill loadout slot."));
	const bool bPassive = Info.AbilityType == Tags.ability_type_passive;
	if (!bPassive && Info.AbilityType != Tags.ability_type_dynamic) return FText::FromString(TEXT("This technique does not use a loadout slot."));
	const bool bPassiveSlot = Slot == Tags.InputTag_Passive01 || Slot == Tags.InputTag_Passive02 || Slot == Tags.InputTag_Passive03 || Slot == Tags.InputTag_Passive04;
	if (bPassive != bPassiveSlot) return FText::FromString(TEXT("Choose a matching active or passive slot."));
	if (!IsLoadoutSlotUnlocked(Slot)) return FText::FromString(TEXT("This slot is locked or unavailable."));
	return FText();
}

void URAbilitySystemComponent::ServerUnequipAbility_Implementation(FGameplayTag AbilityTag)
{
	FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag);
	if (!Spec || GetLearnedSkillRank(AbilityTag) <= 0) return;
	const FGameplayTag Prev = GetInputTagFromSpec(*Spec);
	if (!Prev.IsValid()) return;
	if (IsPassiveAbility(*Spec))
	{
		DeactivatePassiveAbilityDelegate.Broadcast(AbilityTag);
		CancelAbilityHandle(Spec->Handle);
		MulticastActivatePassiveEffect(AbilityTag, false);
	}
	ClearSlot(Spec);
	const auto& Tags = FRGameplayTags::Get();
	Spec->GetDynamicSpecSourceTags().RemoveTag(Tags.ability_status_equipped);
	Spec->GetDynamicSpecSourceTags().AddTag(Tags.ability_status_unlocked);
	MarkAbilitySpecDirty(*Spec);
	ClientEquipAbility(AbilityTag, Tags.ability_status_unlocked, FGameplayTag(), Prev);
}

void URAbilitySystemComponent::RefreshPassiveSlotLimits()
{
	if (!IsOwnerActorAuthoritative()) return;
	TArray<FGameplayTag> Remove;
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
		if (IsPassiveAbility(Spec) && AbilityHasAnySlot(Spec) && !IsLoadoutSlotUnlocked(GetInputTagFromSpec(Spec))) Remove.Add(GetAbilityTagFromSpec(Spec));
	for (FGameplayTag Skill : Remove) ServerUnequipAbility_Implementation(Skill);
}

void URAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(URAbilitySystemComponent, SelectedMeleeElement);
	DOREPLIFETIME(URAbilitySystemComponent, CoreSkillPointsSpent);
	DOREPLIFETIME(URAbilitySystemComponent, SkillPurchases);
	DOREPLIFETIME(URAbilitySystemComponent, ProgressionUnlocks);
}

void URAbilitySystemComponent::ServerSelectMeleeElement_Implementation(FGameplayTag DamageType)
{
	if (!DamageType.IsValid() || DamageType == FRGameplayTags::Get().Damage_Physical) { SelectedMeleeElement = FGameplayTag(); return; }
	const FGameplayTag Skill = URElementalMastery::GetSkillForDamage(DamageType);
	if (Skill.IsValid() && GetLearnedSkillRank(Skill) > 0) SelectedMeleeElement = DamageType;
}

void URAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	AbilityEquippedDelegate.Broadcast(AbilityTag, Status, Slot, PrevSlot);
}

bool URAbilitySystemComponent::GetDescriptionByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,
                                                          FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (URGameplayAbility* RAbility = Cast<URGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = RAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = RAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			if (OutDescription.Contains(TEXT("LoremIpsum"))) { OutDescription.Reset(); OutNextLevelDescription.Reset(); }
			if (RAbility->IsA<URElementalMastery>()) { OutDescription.Reset(); OutNextLevelDescription.Reset(); }
			BuildSkillDescriptions(AbilityTag, OutDescription, OutNextLevelDescription);
			return true;
		}
	}
	const UAbilityInfo* AbilityInfo = URAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FRGameplayTags::Get().ability_none))
	{
		OutDescription = FString();
	}
	else
	{
		OutDescription = URGameplayAbility::GetLockedDescription(AbilityInfo ? AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement : 1);
	}
	OutNextLevelDescription = FString();
	BuildSkillDescriptions(AbilityTag, OutDescription, OutNextLevelDescription);
	return false;
}

void URAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(Slot);
}

void URAbilitySystemComponent::ClearAbilityFromSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

bool URAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	for (FGameplayTag Tag : Spec->GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTagExact(Slot))
		{
			return true;
		}
	}
	return false;
}

void URAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	if (!bGrantedAbilitiesGiven)
	{
		bGrantedAbilitiesGiven = true;
		AbilityGivenDelegate.Broadcast();
	}
}

void URAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChangeDelegate.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void URAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                                  const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTags.Broadcast(TagContainer);
}
