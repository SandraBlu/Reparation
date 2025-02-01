// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RPlayerGameplayAbility.h"
#include "TargetLockAbility.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UTargetLockAbility : public URPlayerGameplayAbility
{
	GENERATED_BODY()

protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	
};
