#include "GAS/Ability/RCompanionAttackAbility.h"
#include "AI/RCompanion.h"
#include "AI/AIC_Companion.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "TimerManager.h"

URCompanionAttackAbility::URCompanionAttackAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool URCompanionAttackAbility::IsConfiguredFor(const ARCompanion* Companion) const
{
	return GetConfigurationIssue(Companion).IsEmpty();
}

FText URCompanionAttackAbility::GetConfigurationIssue(const ARCompanion* Companion) const
{
	if (!Companion) return FText::FromString(TEXT("No companion selected."));
	if (Companion->bRequiresEquippedWeapon && !Companion->HasAssignedWeapon()) return FText::FromString(TEXT("No assigned weapon with a valid hand socket."));
	if (!DamageEffectClass) return FText::FromString(TEXT("Attack Ability is missing its Damage Effect."));
	if (!DamageType.IsValid()) return FText::FromString(TEXT("Attack Ability is missing its Damage Type."));
	if (!FMath::IsFinite(AttackRange) || AttackRange <= 0.f) return FText::FromString(TEXT("Attack Ability needs an Attack Range above zero."));
	if (!FMath::IsFinite(HitTimeFraction)) return FText::FromString(TEXT("Attack Ability has an invalid Hit Time Fraction."));
	if (Damage.GetValueAtLevel(Companion->CompanionLevel) <= 0.f) return FText::FromString(TEXT("Attack Ability needs damage above zero."));
	if (!ChooseMontageFor(Companion)) return FText::FromString(TEXT("Attack Ability has no valid Attack Montage."));
	return FText::GetEmpty();
}

UAnimMontage* URCompanionAttackAbility::ChooseMontageFor(const ARCompanion* Companion) const
{
	if (IsValid(AttackMontage) && AttackMontage->GetPlayLength() > 0.f) return AttackMontage;
	TArray<FTaggedMontage> Montages = IRCombatInterface::Execute_GetAttackMontages(const_cast<ARCompanion*>(Companion));
	if (Montages.IsEmpty()) Montages = const_cast<ARCompanion*>(Companion)->GetAttackMontages_Implementation();
	Montages.RemoveAll([](const FTaggedMontage& Entry) { return !IsValid(Entry.Montage) || Entry.Montage->GetPlayLength() <= 0.f; });
	return Montages.IsEmpty() ? nullptr : Montages[FMath::RandHelper(Montages.Num())].Montage;
}

bool URCompanionAttackAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const ARCompanion* Companion = ActorInfo ? Cast<ARCompanion>(ActorInfo->AvatarActor.Get()) : nullptr;
	return Companion && IsConfiguredFor(Companion) && Companion->CanAttackTarget(Companion->GetCombatTarget_Implementation())
		&& Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void URCompanionAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ARCompanion* Companion = Cast<ARCompanion>(GetAvatarActorFromActorInfo());
	if (Companion && Companion->bRequiresEquippedWeapon && !Companion->EquipAssignedWeapons()) { InterruptAttack(); return; }
	if (!Companion || !CommitAbility(Handle, ActorInfo, ActivationInfo)) { InterruptAttack(); return; }
	AttackTarget = IRCombatInterface::Execute_GetCombatTarget(Companion);
	UAnimMontage* Montage = ChooseMontageFor(Companion);
	if (!Montage) { InterruptAttack(); return; }
	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	Task->OnCompleted.AddDynamic(this, &ThisClass::CompleteAttack);
	Task->OnInterrupted.AddDynamic(this, &ThisClass::InterruptAttack);
	Task->OnCancelled.AddDynamic(this, &ThisClass::InterruptAttack);
	Task->ReadyForActivation();
	if (!IsActive()) return;
	GetWorld()->GetTimerManager().SetTimer(HitTimer, this, &ThisClass::ApplyHit,
		Montage->GetPlayLength() * FMath::Clamp(HitTimeFraction, 0.01f, 0.99f), false);
	GetWorld()->GetTimerManager().SetTimer(TimeoutTimer, this, &ThisClass::InterruptAttack, Montage->GetPlayLength() + 1.f, false);
}

void URCompanionAttackAbility::ApplyHit()
{
	ARCompanion* Companion = Cast<ARCompanion>(GetAvatarActorFromActorInfo());
	AActor* Target = AttackTarget.Get();
	AAIC_Companion* AI = Companion ? Cast<AAIC_Companion>(Companion->GetController()) : nullptr;
	if (!IsActive() || !Companion || !Companion->HasAuthority() || !Companion->CanAttackTarget(Target) || !AI
		|| FVector::Dist(Companion->GetActorLocation(), Target->GetActorLocation()) > AttackRange || !AI->LineOfSightTo(Target)) return;
	URAbilitySystemLibrary::ApplyDamageEffect(MakeDamageEffectParamsFromClassDefaults(Target));
}

void URCompanionAttackAbility::CompleteAttack() { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); }
void URCompanionAttackAbility::InterruptAttack() { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true); }

void URCompanionAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo)) return;
	GetWorld()->GetTimerManager().ClearTimer(HitTimer);
	GetWorld()->GetTimerManager().ClearTimer(TimeoutTimer);
	AttackTarget.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
