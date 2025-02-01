// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RGameplayAbility.h"

#include "AI/REnemy.h"
#include "Characters/RPlayer.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"

FString URGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>"),
	L"Default Ability Name - LoremIpsum ", Level);
}

FString URGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more damage. </>"), Level);
}

FString URGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Ability Locked Until Level: %d</>"), Level);
}

float URGameplayAbility::GetEnergyCost(float InLevel) const
{
	float EnergyCost = 0.f;
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			if (Mod.Attribute == URAttributeSet::GetEnergyAttribute())
			{
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, EnergyCost);
				break;
			}
		}
	}
	return EnergyCost;
}

float URGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}

URAbilitySystemComponent* URGameplayAbility::GetRAbilitySystemComponentFromActorInfo() const
{
	return Cast<URAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent);
}

ARPlayer* URGameplayAbility::GetPlayerFromActorInfo()
{
	if (!CachedPlayer.IsValid())
	{
		CachedPlayer = Cast<ARPlayer>(CurrentActorInfo->AvatarActor);
	}
   
	return CachedPlayer.IsValid()? CachedPlayer.Get() : nullptr;
}

class AREnemy* URGameplayAbility::GetEnemyCharacterFromActorInfo()
{
	if (!CachedEnemy.IsValid())
	{
		CachedEnemy = Cast<AREnemy>(CurrentActorInfo->AvatarActor);
	}
	return CachedEnemy.IsValid()? CachedEnemy.Get() : nullptr;
}
