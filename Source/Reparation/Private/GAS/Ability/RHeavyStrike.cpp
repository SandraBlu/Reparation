#include "GAS/Ability/RHeavyStrike.h"
#include "GAS/RAbilitySystemComponent.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "NativeGameplayTags.h"
#include "RGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyStrikeSkill, "ability.technique.heavyStrike");
UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyStrikeAttack, "ability.attack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyStrikeStunned, "debuff.stun");
UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyStrikeEvading, "status.evading");
UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyStrikeBlocking, "ability.block");
UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyStrikeLight, "ability.melee");
UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyStrikeAbility, "ability");

URHeavyStrike::URHeavyStrike()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    SetAssetTags(FGameplayTagContainer(HeavyStrikeSkill));
    ActivationOwnedTags.AddTag(HeavyStrikeAttack);
    ActivationBlockedTags.AddTag(HeavyStrikeAttack);
    ActivationBlockedTags.AddTag(HeavyStrikeStunned);
    ActivationBlockedTags.AddTag(HeavyStrikeEvading);
    ActivationBlockedTags.AddTag(HeavyStrikeLight);
    ActivationBlockedTags.AddTag(HeavyStrikeBlocking);
    // Match GA_Melee's single-action montage ownership while this attack plays.
    BlockAbilitiesWithTag.AddTag(HeavyStrikeAbility);
    AttackSelection = ERMeleeAttackSelection::RandomByCharacterLevel;
    bUseMeleeSpecialization = true;
}

FGameplayTag URHeavyStrike::GetRequiredSkillTag() const { return HeavyStrikeSkill; }

bool URHeavyStrike::ShouldUseThrust() const
{
    const AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar) return false;
    const FVector Velocity = Avatar->GetVelocity().GetSafeNormal2D();
    return Avatar->GetVelocity().SizeSquared2D() >= FMath::Square(FMath::Max(1.f,MinimumForwardSpeed))
        && FVector::DotProduct(Avatar->GetActorForwardVector().GetSafeNormal2D(),Velocity) >= FMath::Clamp(ForwardDirectionThreshold,0.f,1.f);
}

TArray<int32> URHeavyStrike::GetAvailableAttackIndices() const
{
    if (!bThrustAttack) return Super::GetAvailableAttackIndices();
    const auto* Player = Cast<ARPlayer>(GetAvatarActorFromActorInfo());
    const auto* PS = Player ? Player->GetPlayerState<ARPlayerState>() : nullptr;
    const int32 Level = PS ? PS->GetPlayerLevel() : 1;
    TArray<int32> Result;
    for (int32 Index=0; Index<ThrustAttacks.Num(); ++Index)
        if (IsValid(ThrustAttacks[Index].Montage) && Level >= FMath::Clamp(ThrustAttacks[Index].RequiredCharacterLevel,1,40))
            Result.Add(Attacks.Num()+Index);
    return Result;
}

FRMeleeAttack URHeavyStrike::GetCurrentAttack() const
{
    const int32 Index = CurrentAttackIndex-Attacks.Num();
    return bThrustAttack && ThrustAttacks.IsValidIndex(Index) ? ThrustAttacks[Index] : Super::GetCurrentAttack();
}

bool URHeavyStrike::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* Source, const FGameplayTagContainer* Target, FGameplayTagContainer* Relevant) const
{
    if (!Super::CanActivateAbility(Handle,Info,Source,Target,Relevant)) return false;
    auto* ASC = Info ? Cast<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get()) : nullptr;
    auto* Player = Info ? Cast<ARPlayer>(Info->AvatarActor.Get()) : nullptr;
    return ASC && Player && !Player->IsDead_Implementation() && !URAbilitySystemLibrary::IsActorBlocking(Player)
        && ASC->GetLearnedSkillRank(GetRequiredSkillTag())>0 && GetMeleeWeapon() && DamageEffectClass
        && !Super::GetAvailableAttackIndices().IsEmpty();
}

void URHeavyStrike::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Event)
{
    Super::ActivateAbility(Handle,Info,Activation,Event);
    if (!IsActive()) return;
    bThrustAttack = ShouldUseThrust();
    // Missing/level-gated thrust alternatives fall back to an authored slash.
    if (GetAvailableAttackIndices().IsEmpty()) bThrustAttack = false;
    if (!BeginMeleeAttack() || !CommitAbility(Handle,Info,Activation))
    { EndAbility(Handle,Info,Activation,true,true); return; }
    auto* Open = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FRGameplayTags::Get().Event_MeleeBegin,nullptr,false,true);
    Open->EventReceived.AddDynamic(this,&URHeavyStrike::OpenHitWindow); Open->ReadyForActivation();
    auto* Close = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FRGameplayTags::Get().Event_MeleeEnd,nullptr,false,true);
    Close->EventReceived.AddDynamic(this,&URHeavyStrike::CloseHitWindow); Close->ReadyForActivation();
    const FRMeleeAttack Attack = GetCurrentAttack();
    auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,Attack.Montage,FMath::Max(.1f,PlayRate),Attack.SectionName);
    Task->OnCompleted.AddDynamic(this,&URHeavyStrike::Completed);
    Task->OnInterrupted.AddDynamic(this,&URHeavyStrike::Interrupted);
    Task->OnCancelled.AddDynamic(this,&URHeavyStrike::Interrupted);
    Task->ReadyForActivation();
}

void URHeavyStrike::OpenHitWindow(FGameplayEventData Payload) { SetDamageWindowOpen(true); }
void URHeavyStrike::CloseHitWindow(FGameplayEventData Payload) { SetDamageWindowOpen(false); }
void URHeavyStrike::Completed() { EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false); }
void URHeavyStrike::Interrupted() { EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true); }
void URHeavyStrike::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,bool bReplicate,bool bCancelled)
{
    if (!IsEndAbilityValid(Handle,Info)) return;
    Super::EndAbility(Handle,Info,Activation,bReplicate,bCancelled);
    bThrustAttack = false;
}

void URHeavyStrike::ProcessMeleeHit(const FHitResult& Hit)
{
    AActor* Target = Hit.GetActor();
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!IsValid(Target) || !Avatar || !Avatar->HasAuthority() || !URAbilitySystemLibrary::IsNotFriend(Avatar,Target)) return;
    auto* Player = Cast<ARPlayer>(Avatar);
    if (!Player || Player->IsDead_Implementation()) return;
    FDamageEffectParams Params = MakeDamageEffectParamsFromClassDefaults(Target);
    Params.BaseDamage *= FMath::Max(0.f,DamageMultiplier);
    URAbilitySystemLibrary::ApplyDamageEffect(Params);
}
