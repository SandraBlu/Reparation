#include "GAS/Ability/RCompanionInterrupt.h"
#include "GAS/Ability/RDamageAbility.h"
#include "AI/RCompanion.h"
#include "AI/REnemy.h"
#include "GAS/Data/RElementalResistanceProfile.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "RGameplayTags.h"
#include "AIController.h"
URCompanionInterrupt::URCompanionInterrupt() { InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly; }
bool URCompanionInterrupt::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
    ARCompanion* Companion = ActorInfo ? Cast<ARCompanion>(ActorInfo->AvatarActor.Get()) : nullptr;
    AREnemy* Enemy = Companion ? Cast<AREnemy>(Companion->GetCombatTarget_Implementation()) : nullptr;
    if (!Enemy || Enemy->IsDead_Implementation() || Companion->GetDistanceTo(Enemy) > 250.f || (Enemy->ElementalResistanceProfile && Enemy->ElementalResistanceProfile->bImmuneToControl)) return false;
    const AAIController* AI = Cast<AAIController>(Companion->GetController());
    if (!AI || !AI->LineOfSightTo(Enemy)) return false;
    for (const FGameplayAbilitySpec& Spec : Enemy->GetAbilitySystemComponent()->GetActivatableAbilities())
        if (Spec.IsActive() && Spec.Ability && Spec.Ability->IsA<URDamageAbility>()) return true;
    return false;
}
void URCompanionInterrupt::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    ARCompanion* Companion = Cast<ARCompanion>(ActorInfo->AvatarActor.Get());
    AREnemy* Enemy = Companion ? Cast<AREnemy>(Companion->GetCombatTarget_Implementation()) : nullptr;
    if (!Enemy || !CommitAbility(Handle, ActorInfo, ActivationInfo)) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }
    UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
    TArray<FGameplayAbilitySpecHandle> Attacks;
    for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()) if (Spec.IsActive() && Spec.Ability && Spec.Ability->IsA<URDamageAbility>()) Attacks.Add(Spec.Handle);
    for (const FGameplayAbilitySpecHandle Attack : Attacks) ASC->CancelAbilityHandle(Attack);
    UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage());
    Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
    Effect->DurationMagnitude = FScalableFloat(0.75f);
    FInheritedTagContainer Tags;
    Tags.Added.AddTag(FRGameplayTags::Get().Debuff_Stun);
    Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>().SetAndApplyTargetTagChanges(Tags);
    FGameplayEffectSpec Spec(Effect, ActorInfo->AbilitySystemComponent->MakeEffectContext(), 1.f);
    ASC->ApplyGameplayEffectSpecToSelf(Spec);
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
