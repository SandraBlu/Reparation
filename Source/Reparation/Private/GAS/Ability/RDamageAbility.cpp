// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RDamageAbility.h"
#include "GameFramework/Character.h"
#include "Components/RCompanionPartyComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AI/REnemy.h"
#include "AI/RCombatCoordinator.h"
#include "Engine/World.h"
#include "GAS/Ability/RFallDamageAbility.h"
#include "GAS/Ability/RElementalMastery.h"
#include "GAS/RAbilitySystemComponent.h"
#include "Framework/RAbilitySystemLibrary.h"

bool URDamageAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	if (IsA<URFallDamageAbility>()) return true;
	AREnemy* Enemy = ActorInfo ? Cast<AREnemy>(ActorInfo->AvatarActor.Get()) : nullptr;
	return !Enemy || Enemy->GetWorld()->GetSubsystem<URCombatCoordinator>()->CanStartAttack(Enemy, Enemy->GetCombatTarget_Implementation());
}

void URDamageAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AREnemy* Enemy = ActorInfo && !IsA<URFallDamageAbility>() ? Cast<AREnemy>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (Enemy && !Enemy->GetWorld()->GetSubsystem<URCombatCoordinator>()->AcquireAttack(Enemy, Enemy->GetCombatTarget_Implementation(), this))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URDamageAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AREnemy* Enemy = ActorInfo ? Cast<AREnemy>(ActorInfo->AvatarActor.Get()) : nullptr)
		Enemy->GetWorld()->GetSubsystem<URCombatCoordinator>()->ReleaseAttack(Enemy, this);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URDamageAbility::CauseDamage(AActor* TargetActor)
{
	if (bUseMeleeSpecialization)
	{
		if (IsValid(TargetActor) && GetAvatarActorFromActorInfo()->HasAuthority()) URAbilitySystemLibrary::ApplyDamageEffect(MakeDamageEffectParamsFromClassDefaults(TargetActor));
		return;
	}
	if (!bSilentAttack && IsValid(TargetActor) && TargetActor->ActorHasTag(TEXT("enemy")))
		if (URCompanionPartyComponent* Party = GetAvatarActorFromActorInfo()->FindComponentByClass<URCompanionPartyComponent>()) Party->NotifyNormalCombat(TargetActor);
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
	const float ScaledDamage = Damage.GetValueAtLevel(GetDamageScalingLevel());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, DamageType, ScaledDamage);
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

FDamageEffectParams URDamageAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor,
	FVector InRadialDamageOrigin, bool bOverrideKnockbackDirection, FVector KnockbackOverride,
	bool bOverrideDeathImpulse, FVector DeathImpulseOverride, bool bOverridePitch, float PitchOverride) const
{
	FDamageEffectParams Params;
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	Params.BaseDamage = Damage.GetValueAtLevel(GetDamageScalingLevel());
	Params.bSilentAttack = bSilentAttack;
	Params.AbilityLevel = GetAbilityLevel();
	Params.DamageType = DamageType;
	Params.DebuffChance = DebuffChance;
	Params.DebuffDamage = DebuffDamage;
	Params.DebuffDuration = DebuffDuration;
	Params.DebuffFrequency = DebuffFrequency;
	if (bUseMeleeSpecialization)
	{
		if (URAbilitySystemComponent* ASC = Cast<URAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
		{
			const int32 Rank = ASC->GetLearnedSkillRank(URElementalMastery::GetSkillForDamage(ASC->SelectedMeleeElement));
			if (Rank > 0)
			{
				Params.DamageType = ASC->SelectedMeleeElement;
				Params.DebuffChance = 30.f;
				Params.DebuffDamage = 5.f * (1.f + 0.2f * (FMath::Clamp(Rank, 1, 5) - 1));
				Params.DebuffDuration = 4.f;
				Params.DebuffFrequency = 1.f;
			}
		}
	}
	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	Params.KnockbackMagnitude = KnockbackMagnitude;
	Params.KnockbackChance = KnockbackChance;

	if (IsValid(TargetActor))
	{
		FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
        
		if (bOverridePitch)
		{
			Rotation.Pitch = PitchOverride;
		}
		const FVector ToTarget = Rotation.Vector();
		if (!bOverrideKnockbackDirection)
		{
			Params.KnockbackForce = ToTarget * KnockbackMagnitude;
		}
		if (!bOverrideDeathImpulse)
		{
			Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;
		}
	}
	//Override Knockback Direction
	if (bOverrideKnockbackDirection)
	{
		KnockbackOverride.Normalize();
		Params.KnockbackForce = KnockbackOverride * KnockbackMagnitude;
		if (bOverridePitch)
		{
			FRotator KnockbackRotation = KnockbackOverride.Rotation();
			KnockbackRotation.Pitch = PitchOverride;
			Params.KnockbackForce = KnockbackRotation.Vector() * KnockbackMagnitude;
		}
	}
	//Override DeathImpulse Direction
	if (bOverrideDeathImpulse)
	{
		DeathImpulseOverride.Normalize();
		Params.DeathImpulse = DeathImpulseOverride * DeathImpulseMagnitude;
		if (bOverridePitch)
		{
			FRotator DeathImpulseRotation = DeathImpulseOverride.Rotation();
			DeathImpulseRotation.Pitch = PitchOverride;
			Params.DeathImpulse = DeathImpulseRotation.Vector() * DeathImpulseMagnitude;
		}
	}
	if (bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialDamageOrigin = InRadialDamageOrigin;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
	}
	return Params;
}

FTaggedMontage URDamageAbility::GetRandomTaggedMontage(const TArray<FTaggedMontage>& TaggedMontages) const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	ACharacter* Target = Avatar && Avatar->Implements<URCombatInterface>()
		? Cast<ACharacter>(IRCombatInterface::Execute_GetCombatTarget(Avatar)) : nullptr;
	return SelectMontageForTarget(TaggedMontages, Target);
}

FTaggedMontage URDamageAbility::SelectMontageForTarget(const TArray<FTaggedMontage>& Montages, const ACharacter* Target)
{
	const EAttackTargetStance Stance = Target
		? (Target->bIsCrouched ? EAttackTargetStance::Crouching : EAttackTargetStance::Standing)
		: EAttackTargetStance::Any;
	TArray<const FTaggedMontage*> Exact;
	TArray<const FTaggedMontage*> General;
	for (const FTaggedMontage& Entry : Montages)
	{
		if (!IsValid(Entry.Montage)) continue;
		if (Entry.TargetStance == EAttackTargetStance::Any) General.Add(&Entry);
		else if (Target && Entry.TargetStance == Stance) Exact.Add(&Entry);
	}
	const TArray<const FTaggedMontage*>& Choices = Exact.IsEmpty() ? General : Exact;
	return Choices.IsEmpty() ? FTaggedMontage() : *Choices[FMath::RandRange(0, Choices.Num() - 1)];
}
