// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/RAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RGameplayTags.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GAS/Ability/RGameplayAbility.h"
#include "GAS/Data/AbilityInfo.h"
#include "Interfaces/RPlayerInterface.h"

void URAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URAbilitySystemComponent::ClientEffectApplied);
}

void URAbilitySystemComponent::AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass  : GrantedAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const URGameplayAbility* RAbility = Cast<URGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(RAbility->StartupInputTag);
			AbilitySpec.DynamicAbilityTags.AddTag(FRGameplayTags::Get().ability_status_equipped);
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
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void URAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (auto& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void URAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
 
 	for (auto& AbilitySpec : GetActivatableAbilities())
 	{
 		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
 		{
 			AbilitySpecInputReleased(AbilitySpec);
 		}
 	}
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
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
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
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
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
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag("ability.status")))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

FGameplayAbilitySpec* URAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for(FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
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
	UAbilityInfo* AbilityInfo = URAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FRAbilityInfo& Info : AbilityInfo->AbilityInfo)
	{
		if (!Info.AbilityTag.IsValid())continue;
		if (Level < Info.LevelRequirement) continue;
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.DynamicAbilityTags.AddTag(FRGameplayTags::Get().ability_status_available);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec);
			ClientUpdateAbilityStatus(Info.AbilityTag, FRGameplayTags::Get().ability_status_available, 1);
		}
	}
}

void URAbilitySystemComponent::ServerSpendAbilityPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (GetAvatarActor()->Implements<URPlayerInterface>())
		{
			IRPlayerInterface::Execute_AddToAbilityPts(GetAvatarActor(), -1);
		}
		const FRGameplayTags GameplayTags = FRGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.ability_status_available))
		{
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.ability_status_available);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.ability_status_unlocked);
			Status = GameplayTags.ability_status_unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.ability_status_equipped) || Status.MatchesTagExact(GameplayTags.ability_status_unlocked))
		{
			AbilitySpec->Level += 1;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void URAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);
	if (GetAvatarActor()->Implements<URPlayerInterface>())
	{
		IRPlayerInterface::Execute_AddToAttributePts(GetAvatarActor(), -1);
	}
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
