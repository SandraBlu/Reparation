// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/RAbilitySystemComponent.h"
#include "RGameplayTags.h"
#include "AbilitySystem/Ability/RGameplayAbility.h"
//#include "AlphaOmega/AOLogChannel.h"
#include "Interface/RPlayerInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
//#include <Framework/RBFL.h>

void URAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URAbilitySystemComponent::ClientEffectApplied);
}

void URAbilitySystemComponent::AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : GrantedAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const URGameplayAbility* RAbility = Cast<URGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(RAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

void URAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
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
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void URAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* ASComp, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle) const
{
	
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	for (const FGameplayTag& Tag : TagContainer)
	{
		const FString Msg = FString::Printf(TEXT("GE Tag: %s"), *Tag.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, Msg);
		//EffectTags.Broadcast(TagContainer);
	}
	
}

