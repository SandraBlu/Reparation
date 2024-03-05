// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/AOAbilitySystemComponent.h"

void UAOAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAOAbilitySystemComponent::EffectApplied);
}

void UAOAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* ASComp, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
