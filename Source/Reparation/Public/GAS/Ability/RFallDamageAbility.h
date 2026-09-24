// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RDamageAbility.h"
#include "RFallDamageAbility.generated.h"

/**
 * Hurts the character for landing too hard.
 *
 * Triggered by event.landed, whose magnitude is the impact speed. Derives from
 * URDamageAbility so the damage goes through the same effect, the same damage
 * type and the same execution as a sword blow: falling should be reduced by
 * armour and resistances rather than bypassing them.
 */
UCLASS()
class REPARATION_API URFallDamageAbility : public URDamageAbility
{
	GENERATED_BODY()

public:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	/**
	 * Landings gentler than this cost nothing. Above JumpZVelocity on purpose:
	 * a standing jump lands at almost exactly that speed, so a threshold of 500
	 * would put every jump on the boundary.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Fall Damage", meta = (ClampMin = "0.0"))
	float SafeImpactSpeed = 600.f;

	/**
	 * Carrying this cancels the damage outright, which is what learning to
	 * skydive buys. Kept on the ability rather than as an ignore tag on the
	 * effect, because that effect is shared with combat and a skydiver should
	 * not also shrug off swords.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Fall Damage")
	FGameplayTag SuppressedByTag;
};
