// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RGameplayAbility.h"
#include "RMeleeGameplayAbility.generated.h"

class UPawnCombatComponent;

UENUM(BlueprintType)
enum class ERAbilityActivationPolicy : uint8
{
	OnTriggered,
	OnGiven
};

/**
 * 
 */
UCLASS()
class REPARATION_API URMeleeGameplayAbility : public URGameplayAbility
{
	GENERATED_BODY()

protected:
	//~ Begin UGameplayAbility Interface.
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface

	UPROPERTY(EditDefaultsOnly, Category = "WarriorAbility")
	ERAbilityActivationPolicy AbilityActivationPolicy = ERAbilityActivationPolicy::OnTriggered;

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	UPawnCombatComponent* GetPawnCombatComponentFromActorInfo() const;

};
