#include "GAS/Ability/RPlayerMeleeSkills.h"
#include "GAS/RAbilitySystemComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Characters/RPlayer.h"
#include "AI/REnemy.h"
#include "GAS/Data/RElementalResistanceProfile.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "NativeGameplayTags.h"
#include "RGameplayTags.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Actors/RWeapon.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyTechnique, "ability.technique.heavyStrike");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ChainTechnique, "ability.technique.chainAttack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(CounterSkill, "ability.technique.counterstrike");
UE_DEFINE_GAMEPLAY_TAG_STATIC(BashSkill, "ability.technique.shieldBash");
UE_DEFINE_GAMEPLAY_TAG_STATIC(CounterCooldown, "cooldown.counterstrike");
UE_DEFINE_GAMEPLAY_TAG_STATIC(BashCooldown, "cooldown.shieldBash");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TechniqueAttack, "ability.attack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TechniqueEvading, "status.evading");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TechniqueStunned, "debuff.stun");

namespace
{
    void ApplyTimedTag(UAbilitySystemComponent* Target, FGameplayTag Tag, float Seconds)
    {
        if (!Target || Seconds <= 0 || !FMath::IsFinite(Seconds)) return;
        UGameplayEffect* Effect = NewObject<UGameplayEffect>();
        Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
        Effect->DurationMagnitude = FScalableFloat(Seconds);
        FInheritedTagContainer Tags; Tags.Added.AddTag(Tag);
        Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>().SetAndApplyTargetTagChanges(Tags);
        FGameplayEffectSpec Spec(Effect,Target->MakeEffectContext(),1);
        Target->ApplyGameplayEffectSpecToSelf(Spec);
    }
}

void URAbilitySystemComponent::RecordSuccessfulBlock(AActor* Attacker)
{
    AActor* Avatar = GetAvatarActor();
    if (!IsValid(Avatar) || !Avatar->HasAuthority() || !IsValid(Attacker) || !GetWorld()
        || !URAbilitySystemLibrary::IsActorBlocking(Avatar) || !URAbilitySystemLibrary::IsNotFriend(Avatar,Attacker)
        || !URAbilitySystemLibrary::IsValidBlock(Attacker,Avatar) || GetLearnedSkillRank(CounterSkill) <= 0) return;
    CounterstrikeAttacker = Attacker;
    CounterstrikeExpires = GetWorld()->GetTimeSeconds() + FMath::Clamp(CounterstrikeWindow,.1f,5.f);
}
AActor* URAbilitySystemComponent::GetCounterstrikeTarget() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() <= CounterstrikeExpires ? CounterstrikeAttacker.Get() : nullptr;
}
bool URAbilitySystemComponent::ConsumeCounterstrike()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || !GetCounterstrikeTarget()) return false;
    CounterstrikeAttacker.Reset(); CounterstrikeExpires = -1; return true;
}

URHeavyStrikeTechnique::URHeavyStrikeTechnique() { SetAssetTags(FGameplayTagContainer(HeavyTechnique)); }
URChainAttackTechnique::URChainAttackTechnique() { SetAssetTags(FGameplayTagContainer(ChainTechnique)); }
URPlayerMeleeFollowup::URPlayerMeleeFollowup()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    ActivationOwnedTags.AddTag(TechniqueAttack);
    ActivationBlockedTags.AddTag(TechniqueStunned);
    ActivationBlockedTags.AddTag(TechniqueEvading);
}
URPlayerCounterstrike::URPlayerCounterstrike() { SetAssetTags(FGameplayTagContainer(CounterSkill)); }
URPlayerShieldBash::URPlayerShieldBash() { SetAssetTags(FGameplayTagContainer(BashSkill)); bShieldBash=true; DamageMultiplier=.65f; CooldownSeconds=6.f; }

bool URPlayerMeleeFollowup::CanHitFollowupTarget(AActor* Target) const
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    auto* Enemy = Cast<AREnemy>(Target);
    if (!IsValid(Avatar) || !IsValid(Enemy) || Enemy->IsDead_Implementation() || !URAbilitySystemLibrary::IsNotFriend(Avatar,Enemy)) return false;
    const FVector Delta = Target->GetActorLocation()-Avatar->GetActorLocation();
    if (Delta.SizeSquared() > FMath::Square(Reach) || FVector::DotProduct(Avatar->GetActorForwardVector(),Delta.GetSafeNormal()) < .35f) return false;
    FHitResult Hit; FCollisionQueryParams Query(SCENE_QUERY_STAT(MeleeFollowupSight),false,Avatar);
    Query.AddIgnoredActor(GetMeleeWeapon());
    return !GetWorld()->LineTraceSingleByChannel(Hit,Avatar->GetActorLocation(),Target->GetActorLocation(),ECC_Visibility,Query) || Hit.GetActor()==Target;
}
bool URPlayerMeleeFollowup::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* Source,const FGameplayTagContainer* Target,FGameplayTagContainer* Relevant) const
{
    if (!Super::CanActivateAbility(Handle,Info,Source,Target,Relevant)) return false;
    auto* ASC = Info ? Cast<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get()) : nullptr;
    auto* Player = Info ? Cast<ARPlayer>(Info->AvatarActor.Get()) : nullptr;
    if (!ASC || !Player || Player->IsDead_Implementation() || ASC->GetLearnedSkillRank(bShieldBash ? BashSkill : CounterSkill)<=0
        || ASC->HasMatchingGameplayTag(bShieldBash ? BashCooldown : CounterCooldown) || Attacks.IsEmpty() || !Attacks[0].Montage || !GetMeleeWeapon()) return false;
    return bShieldBash ? URAbilitySystemLibrary::IsActorBlocking(Player) : CanHitFollowupTarget(ASC->GetCounterstrikeTarget());
}
void URPlayerMeleeFollowup::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event)
{
    auto* ASC = CastChecked<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get());
    CounterTarget = ASC->GetCounterstrikeTarget(); bCounterHit=false;
    if (!CommitAbility(Handle,Info,Activation) || (!bShieldBash && !ASC->ConsumeCounterstrike())) { EndAbility(Handle,Info,Activation,true,true); return; }
    if (!bShieldBash)
    {
        FGameplayTagContainer Block(FRGameplayTags::Get().ability_block); ASC->CancelAbilities(&Block);
        if (auto* Player=Cast<ARPlayer>(Info->AvatarActor.Get())) Player->SetIsUsingBlock(false);
    }
    if (!BeginMeleeAttack()) { EndAbility(Handle,Info,Activation,true,true); return; }
    ApplyTimedTag(ASC,bShieldBash ? BashCooldown : CounterCooldown,CooldownSeconds);
    if (bShieldBash)
    {
        // Guard's montage is reused for the first magic-shield bash; dedicated thrust animation remains editable.
        GetWorld()->GetTimerManager().SetTimer(BashImpactTimer,this,&URPlayerMeleeFollowup::ImpactBash,FMath::Max(.01f,BashImpactTime),false);
        GetWorld()->GetTimerManager().SetTimer(BashRecoveryTimer,this,&URPlayerMeleeFollowup::Completed,FMath::Max(BashImpactTime+.05f,BashRecoveryTime),false);
    }
    else
    {
        auto* Open=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FRGameplayTags::Get().Event_MeleeBegin,nullptr,false,true);
        Open->EventReceived.AddDynamic(this,&URPlayerMeleeFollowup::OpenHitWindow); Open->ReadyForActivation();
        auto* Close=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FRGameplayTags::Get().Event_MeleeEnd,nullptr,false,true);
        Close->EventReceived.AddDynamic(this,&URPlayerMeleeFollowup::CloseHitWindow); Close->ReadyForActivation();
    }
    auto* Montage=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,GetCurrentAttack().Montage,1.f,GetCurrentAttack().SectionName);
    Montage->OnCompleted.AddDynamic(this,&URPlayerMeleeFollowup::Completed);
    Montage->OnInterrupted.AddDynamic(this,&URPlayerMeleeFollowup::Interrupted);
    Montage->OnCancelled.AddDynamic(this,&URPlayerMeleeFollowup::Interrupted);
    Montage->ReadyForActivation();
}
void URPlayerMeleeFollowup::OpenHitWindow(FGameplayEventData Payload) { SetDamageWindowOpen(true); }
void URPlayerMeleeFollowup::CloseHitWindow(FGameplayEventData Payload) { SetDamageWindowOpen(false); }
void URPlayerMeleeFollowup::Completed() { EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false); }
void URPlayerMeleeFollowup::Interrupted() { EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true); }
void URPlayerMeleeFollowup::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,const FGameplayAbilityActivationInfo Activation,bool bReplicate,bool bCancelled)
{
    if (!IsEndAbilityValid(Handle,Info)) return;
    if (GetWorld()) { GetWorld()->GetTimerManager().ClearTimer(BashImpactTimer); GetWorld()->GetTimerManager().ClearTimer(BashRecoveryTimer); }
    CounterTarget.Reset();
    Super::EndAbility(Handle,Info,Activation,bReplicate,bCancelled);
}
void URPlayerMeleeFollowup::ProcessMeleeHit(const FHitResult& Hit)
{
    if (!bShieldBash && !bCounterHit && Hit.GetActor()==CounterTarget.Get() && CanHitFollowupTarget(Hit.GetActor()))
    { bCounterHit=true; DamageTarget(Hit.GetActor()); }
}
void URPlayerMeleeFollowup::ImpactBash()
{
    if (!IsActive() || !GetAvatarActorFromActorInfo()->HasAuthority()) return;
    AActor* Avatar=GetAvatarActorFromActorInfo();
    FCollisionObjectQueryParams Objects; Objects.AddObjectTypesToQuery(ECC_Pawn);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(ShieldBash),false,Avatar);
    TArray<FOverlapResult> Hits;
    GetWorld()->OverlapMultiByObjectType(Hits,Avatar->GetActorLocation(),FQuat::Identity,Objects,FCollisionShape::MakeSphere(Reach),Query);
    AActor* Best=nullptr; float Distance=MAX_flt;
    for (const FOverlapResult& Hit : Hits) if (CanHitFollowupTarget(Hit.GetActor()) && Avatar->GetDistanceTo(Hit.GetActor())<Distance)
    { Best=Hit.GetActor(); Distance=Avatar->GetDistanceTo(Best); }
    if (Best) DamageTarget(Best);
}
void URPlayerMeleeFollowup::DamageTarget(AActor* Target)
{
    if (!IsValid(Target) || !GetAvatarActorFromActorInfo()->HasAuthority()) return;
    if (auto* Player=Cast<ARPlayer>(GetAvatarActorFromActorInfo()); Player && Player->IsDead_Implementation()) return;
    FDamageEffectParams Params=MakeDamageEffectParamsFromClassDefaults(Target);
    Params.BaseDamage *= FMath::Max(0.f,DamageMultiplier);
    auto* Enemy=Cast<AREnemy>(Target);
    const bool bImmune = Enemy && Enemy->ElementalResistanceProfile && Enemy->ElementalResistanceProfile->bImmuneToControl;
    if (bImmune) { Params.KnockbackChance=0; Params.KnockbackForce=FVector::ZeroVector; }
    URAbilitySystemLibrary::ApplyDamageEffect(Params);
    if (bShieldBash && Enemy && !Enemy->IsDead_Implementation() && !bImmune)
    {
        auto* TargetASC=Enemy->GetAbilitySystemComponent();
        TArray<FGameplayAbilitySpecHandle> ActiveAttacks;
        for (const auto& Spec : TargetASC->GetActivatableAbilities()) if (Spec.IsActive() && Spec.Ability && Spec.Ability->IsA<URDamageAbility>()) ActiveAttacks.Add(Spec.Handle);
        for (const auto H : ActiveAttacks) TargetASC->CancelAbilityHandle(H);
        ApplyTimedTag(TargetASC,FRGameplayTags::Get().Debuff_Stun,StunSeconds);
    }
}
