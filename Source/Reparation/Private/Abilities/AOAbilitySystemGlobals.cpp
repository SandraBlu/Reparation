// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/AOAbilitySystemGlobals.h"
#include "AOAbilityTypes.h"

FGameplayEffectContext* UAOAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FAOGameplayEffectContext();
}