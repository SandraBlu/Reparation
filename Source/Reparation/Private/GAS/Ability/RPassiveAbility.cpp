// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RPassiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/RAbilitySystemComponent.h"

void URPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (URAbilitySystemComponent* RASC = Cast<URAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		RASC->DeactivatePassiveAbilityDelegate.AddUObject(this, &URPassiveAbility::ReceiveDeactivate);
	}
}

void URPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}