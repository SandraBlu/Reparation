// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/RAbilitySystemComponent.h"
#include "RGameplayTags.h"
#include "AbilitySystem/Ability/RGameplayAbility.h"
//#include "AlphaOmega/AOLogChannel.h"
#include "Interface/RPlayerInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include <Framework/RBFL.h>
#include <AbilitySystem/Data/RAbilityInfo.h>

void URAbilitySystemComponent::AbilityActorInfoInit()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URAbilitySystemComponent::EffectApplied);
}

void URAbilitySystemComponent::AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : GrantedAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		//GiveAbility(AbilitySpec);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void URAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag) const
{
	if (GetAvatarActor()->Implements<URPlayerInterface>())
	{
		if (IRPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			FGameplayEventData Payload;
			Payload.EventTag = AttributeTag;
			Payload.EventMagnitude = .5f;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

			IRPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
		}
	}
}

//void URAbilitySystemComponent::AddPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& PassiveAbilities)
//{
//	for (const TSubclassOf<UGameplayAbility> AbilityClass : PassiveAbilities)
//	{
//		//if ability is an AOGameplayAbility, look for ability input tag and add it to dynamic ability tags
//		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
//		GiveAbilityAndActivateOnce(AbilitySpec);
//	}
//}

void URAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* ASComp, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle) const
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectTags.Broadcast(TagContainer);
}

