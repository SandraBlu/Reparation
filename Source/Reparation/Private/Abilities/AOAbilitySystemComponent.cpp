// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/AOAbilitySystemComponent.h"
#include "AOGameplayTags.h"
#include "Abilities/AOGameplayAbility.h"

void UAOAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAOAbilitySystemComponent::EffectApplied);
	
}

void UAOAbilitySystemComponent::AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : GrantedAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAOGameplayAbility* AOAbility = Cast<UAOGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AOAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

void UAOAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
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

void UAOAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
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

void UAOAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* ASComp, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
