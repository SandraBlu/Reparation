// Fill out your copyright notice in the Description page of Project Settings.
#include "GAS/Ability/RMeleeAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Actors/RWeapon.h"
#include "Animation/AnimMontage.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Interfaces/RPlayerInterface.h"

URMeleeAbility::URMeleeAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

UPawnCombatComponent* URMeleeAbility::GetCombatComponentFromActorInfo()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	return IsValid(Avatar) ? Avatar->FindComponentByClass<UPawnCombatComponent>() : nullptr;
}

ARWeapon* URMeleeAbility::GetMeleeWeapon() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!IsValid(Avatar) || !Avatar->Implements<URPlayerInterface>()) return nullptr;
	ARWeapon* Weapon = IRPlayerInterface::Execute_GetCurrentWeapon(Avatar);
	if (!IsValid(Weapon) || !Weapon->IsEquipped ||
		(Weapon->CombatType != ECombatType::ECT_LightSword && Weapon->CombatType != ECombatType::ECT_Staff))
	{
		return nullptr;
	}
	return Weapon;
}

bool URMeleeAbility::GetMeleeTraceLocations(FVector& Start, FVector& End) const
{
	Start = End = FVector::ZeroVector;
	ARWeapon* Weapon = GetMeleeWeapon();
	USkeletalMeshComponent* Mesh = Weapon ? Weapon->GetWeaponMesh() : nullptr;
	if (!IsValid(Mesh) || !Mesh->DoesSocketExist(Weapon->MeleeStartSocket) ||
		!Mesh->DoesSocketExist(Weapon->MeleeEndSocket)) return false;
	Start = Mesh->GetSocketLocation(Weapon->MeleeStartSocket);
	End = Mesh->GetSocketLocation(Weapon->MeleeEndSocket);
	return true;
}

int32 URMeleeAbility::GetUnlockedAttackCount() const
{
	int32 Count = 0;
	for (int32 Index = 0; Index < Attacks.Num(); ++Index)
	{
		const int32 RequiredLevel = Index == 0 ? 1 : FMath::Max(2, Attacks[Index].RequiredAbilityLevel);
		if (!IsValid(Attacks[Index].Montage) || GetAbilityLevel() < RequiredLevel) break;
		++Count;
	}
	return Count;
}

bool URMeleeAbility::BeginMeleeAttack()
{
	if (!IsActive() || CurrentAttackIndex != INDEX_NONE) return false;
	ResetMeleeState();
	ARWeapon* Weapon = GetMeleeWeapon();
	if (!Weapon || GetUnlockedAttackCount() == 0) return false;
	ActiveMeleeWeapon = Weapon;
	CurrentAttackIndex = 0;
	return true;
}

bool URMeleeAbility::QueueComboAttack()
{
	if (!IsActive() || CurrentAttackIndex == INDEX_NONE || !bComboWindowOpen ||
		!IsValid(ActiveMeleeWeapon) || GetMeleeWeapon() != ActiveMeleeWeapon ||
		CurrentAttackIndex + 1 >= GetUnlockedAttackCount()) return false;
	bComboInputQueued = true;
	return true;
}

void URMeleeAbility::SetComboWindowOpen(bool bOpen)
{
	bComboWindowOpen = bOpen && IsActive() && CurrentAttackIndex != INDEX_NONE;
}

bool URMeleeAbility::AdvanceCombo()
{
	if (!IsActive() || CurrentAttackIndex == INDEX_NONE || !bComboInputQueued ||
		!IsValid(ActiveMeleeWeapon) || GetMeleeWeapon() != ActiveMeleeWeapon ||
		CurrentAttackIndex + 1 >= GetUnlockedAttackCount()) return false;
	SetDamageWindowOpen(false);
	++CurrentAttackIndex;
	bComboInputQueued = bComboWindowOpen = bDamageWindowOpen = false;
	HitActors.Reset();
	return true;
}

FRMeleeAttack URMeleeAbility::GetCurrentAttack() const
{
	return Attacks.IsValidIndex(CurrentAttackIndex) ? Attacks[CurrentAttackIndex] : FRMeleeAttack();
}

void URMeleeAbility::SetDamageWindowOpen(bool bOpen)
{
	UWorld* World = GetWorld();
	const bool bCanOpen = bOpen && World && IsActive() && CurrentAttackIndex != INDEX_NONE &&
		IsValid(ActiveMeleeWeapon) && GetMeleeWeapon() == ActiveMeleeWeapon;
	if (!bCanOpen)
	{
		bDamageWindowOpen = false;
		if (World) World->GetTimerManager().ClearTimer(MeleeTraceTimer);
		return;
	}
	// Repeated begin events must not restart the timer or reset the swing's hit history.
	if (bDamageWindowOpen && World->GetTimerManager().IsTimerActive(MeleeTraceTimer)) return;
	bDamageWindowOpen = true;
	const float Interval = FMath::IsFinite(TraceInterval) ? FMath::Max(0.001f, TraceInterval) : 0.01f;
	World->GetTimerManager().SetTimer(MeleeTraceTimer, this,
		&URMeleeAbility::TraceCurrentAttack, Interval, true);
}

void URMeleeAbility::TraceCurrentAttack()
{
	if (!IsActive() || !bDamageWindowOpen || !IsValid(ActiveMeleeWeapon) ||
		GetMeleeWeapon() != ActiveMeleeWeapon)
	{
		SetDamageWindowOpen(false);
		return;
	}
	const TArray<FHitResult> Hits = URAbilitySystemLibrary::TraceMeleeWeapon(
		GetAvatarActorFromActorInfo(), TraceRadius, bDrawDebugTrace);
	for (const FHitResult& Hit : Hits)
	{
		if (RegisterMeleeHit(Hit.GetActor()))
		{
			OnMeleeHit(Hit);
		}
	}
}

bool URMeleeAbility::ApplyMeleeHit(AActor* TargetActor)
{
	// Retain the explicit simple-damage helper for existing Blueprint callers.
	if (!DamageEffectClass || !GetAbilitySystemComponentFromActorInfo() || !RegisterMeleeHit(TargetActor))
	{
		return false;
	}
	CauseDamage(TargetActor);
	return true;
}

bool URMeleeAbility::RegisterMeleeHit(AActor* TargetActor)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!IsActive() || !bDamageWindowOpen || !IsValid(Avatar) || !Avatar->HasAuthority() ||
		!IsValid(ActiveMeleeWeapon) || GetMeleeWeapon() != ActiveMeleeWeapon ||
		!IsValid(TargetActor) || TargetActor == Avatar || TargetActor == ActiveMeleeWeapon ||
		HitActors.Contains(TWeakObjectPtr<AActor>(TargetActor)) ||
		!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor)) return false;
	HitActors.Add(TWeakObjectPtr<AActor>(TargetActor));
	return true;
}

void URMeleeAbility::ResetMeleeState()
{
	SetDamageWindowOpen(false);
	CurrentAttackIndex = INDEX_NONE;
	bComboWindowOpen = bComboInputQueued = bDamageWindowOpen = false;
	ActiveMeleeWeapon = nullptr;
	HitActors.Reset();
}

void URMeleeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo)) return;
	ResetMeleeState();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
