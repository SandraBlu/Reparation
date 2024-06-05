// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/RAbilitySystemComponent.h"

#include "RGameplayTags.h"

void URAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URAbilitySystemComponent::EffectApplied);
}

void URAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
                                             const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTags.Broadcast(TagContainer);
}
