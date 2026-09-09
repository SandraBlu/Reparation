// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/RAbilitySystemGlobals.h"

#include "RAbilityTypes.h"

FGameplayEffectContext* URAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FRGameplayEffectContext();
}
