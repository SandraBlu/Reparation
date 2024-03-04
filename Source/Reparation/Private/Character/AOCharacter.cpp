// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AOCharacter.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"


AAOCharacter::AAOCharacter()
{
	
}

UAbilitySystemComponent* AAOCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
//void AAOCharacter::InitializeAttributes()
//{
//	if (AbilitySystemComponent && GrantedEffect)
//	{
//		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
//		EffectContext.AddSourceObject(this);
//		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GrantedEffect, 1, EffectContext);
//
//		if (SpecHandle.IsValid())
//			FActiveGameplayEffectHandle GEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
//	}
//}
//void AAOCharacter::GrantAbilities()
//{
//	if (AbilitySystemComponent)
//		for (TSubclassOf<UGameplayAbility>& StartupAbility : GrantedAbilities)
//			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartupAbility.GetDefaultObject(), 1, 0));
//}