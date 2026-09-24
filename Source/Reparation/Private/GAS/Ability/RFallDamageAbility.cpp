// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RFallDamageAbility.h"

#include "AbilitySystemComponent.h"
#include "Framework/RAbilitySystemLibrary.h"

void URFallDamageAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// event.landed carries the impact speed as its magnitude. Without the event
	// there is nothing to scale against, so this only runs as a trigger.
	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	const float ImpactSpeed = TriggerEventData->EventMagnitude;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const bool bSuppressed = SuppressedByTag.IsValid() && ASC && ASC->HasMatchingGameplayTag(SuppressedByTag);

	if (ImpactSpeed <= SafeImpactSpeed || bSuppressed)
	{

		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// The inherited Damage is a scalable float, and GetValueAtLevel simply
	// evaluates its curve at whatever is passed in. Feeding it the impact speed
	// rather than an ability level turns CT_FallDamage into a damage by impact
	// curve, and an unset curve leaves the flat value as a sane fallback.
	const float FallDamage = Damage.GetValueAtLevel(ImpactSpeed);

	if (FallDamage <= 0.f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Source and target are both the faller. Unusual for this pipeline, which
	// normally has an attacker, but it means resistances and death are handled
	// by the same execution rather than by a parallel path.
	AActor* Faller = GetAvatarActorFromActorInfo();
	FDamageEffectParams Params = MakeDamageEffectParamsFromClassDefaults(Faller);
	Params.BaseDamage = FallDamage;

	// Configuration failures here are silent otherwise: an unset effect class or
	// damage type produces a spec the execution reads as zero, which looks
	// exactly like the ability never running.
	if (!Params.DamageGameplayEffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%hs: [%s] has no DamageEffectClass set, so no damage is applied."),
			__FUNCTION__, *GetNameSafe(this));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!Params.DamageType.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%hs: [%s] has no DamageType set. The execution reads damage by that tag, so it resolves to nothing."),
			__FUNCTION__, *GetNameSafe(this));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	URAbilitySystemLibrary::ApplyDamageEffect(Params);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
