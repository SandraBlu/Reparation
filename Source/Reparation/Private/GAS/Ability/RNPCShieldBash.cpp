#include "GAS/Ability/RNPCShieldBash.h"
#include "AI/REnemy.h"
#include "Characters/RCharacterBase.h"
#include "Stealth/RStealthComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "RGameplayTags.h"
#include "NativeGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(NPCShieldBash,"ability.npc.shieldBash");
UE_DEFINE_GAMEPLAY_TAG_STATIC(NPCShieldBashCooldown,"cooldown.npc.shieldBash");
UE_DEFINE_GAMEPLAY_TAG_STATIC(NPCShieldAttack,"ability.attack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(NPCShieldStun,"debuff.stun");
UE_DEFINE_GAMEPLAY_TAG_STATIC(NPCShieldStagger,"debuff.stagger");

URNPCShieldBash::URNPCShieldBash()
{
    NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;
    SetAssetTags(FGameplayTagContainer(NPCShieldBash));
    ActivationOwnedTags.AddTag(NPCShieldAttack);
    ActivationBlockedTags.AddTag(NPCShieldAttack);
    ActivationBlockedTags.AddTag(NPCShieldBashCooldown);
    ActivationBlockedTags.AddTag(NPCShieldStun);
    ActivationBlockedTags.AddTag(NPCShieldStagger);
    bTraceFromCharacterMesh=true;
    TraceStartSocket=TraceEndSocket="hand_l";
}
bool URNPCShieldBash::CanHitTarget(AActor* Target) const
{
    auto* Enemy=Cast<AREnemy>(GetAvatarActorFromActorInfo());
    const auto* Character=Cast<ARCharacterBase>(Target);
    if (!Enemy || !IsValid(Target) || Enemy->IsDead_Implementation() || !URAbilitySystemLibrary::IsNotFriend(Enemy,Target)
        || (Character && (Character->IsDead_Implementation() || Character->Stealth->IsConcealed()))) return false;
    const FVector Delta=Target->GetActorLocation()-Enemy->GetActorLocation();
    if (Delta.SizeSquared()>FMath::Square(Reach) || FVector::DotProduct(Enemy->GetActorForwardVector(),Delta.GetSafeNormal())<.35f) return false;
    FHitResult Hit; FCollisionQueryParams Query(SCENE_QUERY_STAT(NPCShieldBashSight),false,Enemy);
    return !GetWorld()->LineTraceSingleByChannel(Hit,Enemy->GetActorLocation(),Target->GetActorLocation(),ECC_Visibility,Query) || Hit.GetActor()==Target;
}
bool URNPCShieldBash::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* Source,const FGameplayTagContainer* Target,FGameplayTagContainer* Relevant) const
{
    const auto* Enemy=Info ? Cast<AREnemy>(Info->AvatarActor.Get()) : nullptr;
    return Enemy && !Attacks.IsEmpty() && Attacks[0].Montage && CanHitTarget(Enemy->GetCombatTarget_Implementation())
        && Super::CanActivateAbility(Handle,Info,Source,Target,Relevant);
}
void URNPCShieldBash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event)
{
    Super::ActivateAbility(Handle,Info,Activation,Event); // Existing attack reservations and cancellation ownership.
    if (!IsActive()) return;
    auto* Enemy=Cast<AREnemy>(Info->AvatarActor.Get());
    if (!Enemy || !CommitAbility(Handle,Info,Activation) || Attacks.IsEmpty() || !Attacks[0].Montage)
    { EndAbility(Handle,Info,Activation,true,true); return; }
    BashTarget=Enemy->GetCombatTarget_Implementation();
    bBashHit=false;
    if (!BeginMeleeAttack()) { EndAbility(Handle,Info,Activation,true,true); return; }
    auto* Open=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FRGameplayTags::Get().Event_MeleeBegin,nullptr,false,true);
    Open->EventReceived.AddDynamic(this,&URNPCShieldBash::OpenHitWindow); Open->ReadyForActivation();
    auto* Close=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FRGameplayTags::Get().Event_MeleeEnd,nullptr,false,true);
    Close->EventReceived.AddDynamic(this,&URNPCShieldBash::CloseHitWindow); Close->ReadyForActivation();
    auto* Task=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,Attacks[0].Montage,1.f,Attacks[0].SectionName,true,1.f,0.f,true);
    Task->OnCompleted.AddDynamic(this,&URNPCShieldBash::Completed);
    Task->OnInterrupted.AddDynamic(this,&URNPCShieldBash::Interrupted);
    Task->OnCancelled.AddDynamic(this,&URNPCShieldBash::Interrupted);
    Task->ReadyForActivation();
}
void URNPCShieldBash::OpenHitWindow(FGameplayEventData Payload) { SetDamageWindowOpen(true); }
void URNPCShieldBash::CloseHitWindow(FGameplayEventData Payload) { SetDamageWindowOpen(false); }
void URNPCShieldBash::ProcessMeleeHit(const FHitResult& Hit)
{
    if (bBashHit || Hit.GetActor()!=BashTarget.Get() || !IsActive() || !GetAvatarActorFromActorInfo()->HasAuthority() || !CanHitTarget(BashTarget.Get())) return;
    bBashHit=true;
    auto Params=MakeDamageEffectParamsFromClassDefaults(BashTarget.Get());
    Params.BaseDamage*=FMath::Max(0.f,DamageMultiplier);
    URAbilitySystemLibrary::ApplyDamageEffect(Params);
}
void URNPCShieldBash::Completed() { EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false); }
void URNPCShieldBash::Interrupted() { EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true); }
void URNPCShieldBash::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,bool bReplicate,bool bCancelled)
{
    if (!IsEndAbilityValid(Handle,Info)) return;
    BashTarget.Reset();
    Super::EndAbility(Handle,Info,Activation,bReplicate,bCancelled);
}
