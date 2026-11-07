#include "GAS/Ability/RPlayerMeleeSkills.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
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
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/REquipmentComponent.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarParryDebug(TEXT("r.Combat.ParryDebug"),1,
    TEXT("Log confirmed block eligibility and enemy parry playback. Set to 0 to disable."));

UE_DEFINE_GAMEPLAY_TAG_STATIC(HeavyTechnique, "ability.technique.heavyStrike");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ChainTechnique, "ability.melee.combo2");
UE_DEFINE_GAMEPLAY_TAG_STATIC(CounterSkill, "ability.technique.counterstrike");
UE_DEFINE_GAMEPLAY_TAG_STATIC(KickSkill, "ability.technique.kick");
UE_DEFINE_GAMEPLAY_TAG_STATIC(CounterCooldown, "cooldown.counterstrike");
UE_DEFINE_GAMEPLAY_TAG_STATIC(KickCooldown, "cooldown.kick");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TechniqueAttack, "ability.attack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TechniqueEvading, "status.evading");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TechniqueStunned, "debuff.stun");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ParrySkill, "ability.technique.parry");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ParryCooldownTag, "cooldown.parry");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SwordGuardSkill, "ability.block");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SwordGuardBlocking, "status.blocking");

void URAbilitySystemComponent::RecordSuccessfulBlock(AActor* Attacker)
{
    AActor* Avatar = GetAvatarActor();
    if (!IsValid(Avatar) || !Avatar->HasAuthority() || !IsValid(Attacker) || !GetWorld()
        || !URAbilitySystemLibrary::IsActorBlocking(Avatar) || !URAbilitySystemLibrary::IsNotFriend(Avatar,Attacker)
        || !URAbilitySystemLibrary::IsValidBlock(Attacker,Avatar)) return;
    TryParryConfirmedBlock(Attacker);
    if (GetLearnedSkillRank(CounterSkill) <= 0) return;
    CounterstrikeAttacker = Attacker;
    CounterstrikeExpires = GetWorld()->GetTimeSeconds() + FMath::Clamp(CounterstrikeWindow,.1f,5.f);
}

URParryTechnique::URParryTechnique() { SetAssetTags(FGameplayTagContainer(ParrySkill)); }

URSwordBlock::URSwordBlock()
{
    InstancingPolicy=EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;
    SetAssetTags(FGameplayTagContainer(SwordGuardSkill));
    ActivationOwnedTags.AddTag(SwordGuardBlocking);
    ActivationBlockedTags.AddTag(TechniqueAttack);
    ActivationBlockedTags.AddTag(TechniqueStunned);
    ActivationBlockedTags.AddTag(TechniqueEvading);
}
bool URSwordBlock::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* Source,const FGameplayTagContainer* Target,FGameplayTagContainer* Relevant) const
{
    if (!Super::CanActivateAbility(Handle,Info,Source,Target,Relevant)) return false;
    auto* Player=Info ? Cast<ARPlayer>(Info->AvatarActor.Get()) : nullptr;
    auto* Gear=Player ? Player->FindComponentByClass<UREquipmentComponent>() : nullptr;
    auto* ASC = Info ? Cast<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get()) : nullptr;
    return Player && ASC && ASC->GetLearnedSkillRank(SwordGuardSkill) > 0 && !Player->IsDead_Implementation() && Gear && IsValid(Gear->EquippedWeapon)
        && Gear->EquippedWeapon->IsEquipped && GetEquippedGuardMontage();
}
UAnimMontage* URSwordBlock::GetEquippedGuardMontage() const
{
    const auto* Player=Cast<ARPlayer>(GetAvatarActorFromActorInfo());
    const auto* Gear=Player ? Player->FindComponentByClass<UREquipmentComponent>() : nullptr;
    return Gear && Gear->EquippedWeapon && Gear->EquippedWeapon->CombatType==ECombatType::ECT_Staff
        ? StaffGuardMontage.Get() : GuardMontage.Get();
}
void URSwordBlock::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event)
{
    ActiveGuardMontage=GetEquippedGuardMontage();
    const auto* Gear=Info->AvatarActor->FindComponentByClass<UREquipmentComponent>();
    bStaffGuard=Gear && Gear->EquippedWeapon && Gear->EquippedWeapon->CombatType==ECombatType::ECT_Staff;
    if (!CommitAbility(Handle,Info,Activation)) { EndAbility(Handle,Info,Activation,true,true); return; }
    auto* ASC=CastChecked<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get());
    ASC->OnParrySucceeded.AddDynamic(this,&URSwordBlock::ParrySucceeded);
    if (auto* Anim=Info->GetAnimInstance()) Anim->OnMontageEnded.AddDynamic(this,&URSwordBlock::PresentationEnded);
    PlayGuardPresentation(ActiveGuardMontage);
    if (IsActive()) if (auto* Player=Cast<ARPlayer>(Info->AvatarActor.Get())) Player->SetIsUsingBlock(true);
}
void URSwordBlock::PlayGuardPresentation(UAnimMontage* Montage,float Rate)
{
    Presentation=Montage; // Set before PlayMontage interrupts the preceding presentation.
    if (!Montage || GetAbilitySystemComponentFromActorInfo()->PlayMontage(this,CurrentActivationInfo,Montage,Rate)<=0.f)
        EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}
void URSwordBlock::ParrySucceeded(AActor* Attacker,bool bInterrupted)
{
    if (!IsActive()) return;
    auto* ASC=CastChecked<URAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
    const auto* Spec=ASC->GetSpecFromAbilityTag(ParrySkill);
    const auto* Settings=Spec ? Cast<URParryTechnique>(Spec->Ability) : nullptr;
    TArray<UAnimMontage*> Valid;
    if (Settings) for (const auto& Montage : (bStaffGuard ? Settings->StaffParryMontages : Settings->ParryMontages)) if (IsValid(Montage.Get())) Valid.Add(Montage.Get());
    if (!Valid.IsEmpty()) PlayGuardPresentation(Valid[FMath::RandRange(0,Valid.Num()-1)],
        FMath::IsFinite(Settings->ParryPlayRate) ? FMath::Clamp(Settings->ParryPlayRate,.1f,3.f) : 1.f);
}
void URSwordBlock::PresentationEnded(UAnimMontage* Montage,bool bInterrupted)
{
    if (!IsActive() || Montage!=Presentation) return;
    auto* ASC = Cast<URAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
    if (bInterrupted || !ASC || !ASC->HasEquippedMeleeWeapon()) EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
    else PlayGuardPresentation(ActiveGuardMontage);
}
void URSwordBlock::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,bool bReplicate,bool bCancelled)
{
    if (!IsEndAbilityValid(Handle,Info)) return;
    auto* ASC=Info ? Cast<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get()) : nullptr;
    if (ASC) ASC->OnParrySucceeded.RemoveDynamic(this,&URSwordBlock::ParrySucceeded);
    if (auto* Anim=Info ? Info->GetAnimInstance() : nullptr) Anim->OnMontageEnded.RemoveDynamic(this,&URSwordBlock::PresentationEnded);
    auto* Player=Info ? Cast<ARPlayer>(Info->AvatarActor.Get()) : nullptr;
    // Releasing guard removes its blocking state immediately, but a confirmed deflection
    // can finish visually. Attacks, evasions, stun and death still interrupt normally.
    const bool bFinishReleasedParry=Player && !Player->GetIsUsingBlock()
        && !Player->IsDead_Implementation() && Presentation && Presentation!=ActiveGuardMontage
        && ASC && !ASC->HasMatchingGameplayTag(TechniqueAttack) && !ASC->HasMatchingGameplayTag(TechniqueStunned)
        && !ASC->HasMatchingGameplayTag(TechniqueEvading);
    if (ASC && ASC->GetAnimatingAbility()==this && !bFinishReleasedParry) ASC->CurrentMontageStop(.1f);
    Presentation=nullptr;
    Super::EndAbility(Handle,Info,Activation,bReplicate,bCancelled);
    if (Player) Player->SetIsUsingBlock(false);
}
FString URParryTechnique::GetDescription(int32 Level)
{
    return FString::Printf(TEXT("<Title>Parry</>\n\n<Default>Raise guard just before a melee hit.</>\n"
        "<Small>Timing window: </><Cooldown>%.2f s</>\n<Small>Stamina per attempt: </><EnergyCost>%.0f</>\n"
        "<Small>Rearm delay: </><Cooldown>%.2f s</>\n<Small>Stagger: </><Cooldown>%.2f s</>\n\n"
        "<Default>Uses your existing block control. Holding guard does not reopen the window. "
        "A confirmed parry interrupts susceptible attackers and opens Counterstrike if learned. Control immunity prevents interruption and stagger.</>"),
        ParryWindow,StaminaCost,GetCooldown(Level),StaggerSeconds);
}

void URAbilitySystemComponent::NotifyGuardStateChanged(bool bGuarding)
{
    // Existing input Blueprint writes bUsingBlock before calling the setter.
    // Track delivered transitions here so that path and repeated held calls agree.
    if (bParryGuardHeld == bGuarding) return;
    bParryGuardHeld = bGuarding;
    ParryExpires = -1;
    auto* Player = Cast<ARPlayer>(GetAvatarActor());
    if (!bGuarding || !HasEquippedMeleeWeapon() || !Player || !Player->HasAuthority() || !Player->GetIsUsingBlock()
        || Player->IsDead_Implementation() || !GetWorld()
        || HasMatchingGameplayTag(TechniqueAttack) || HasMatchingGameplayTag(TechniqueStunned)
        || HasMatchingGameplayTag(TechniqueEvading) || GetLearnedSkillRank(ParrySkill)<=0) return;
    auto* Spec = GetSpecFromAbilityTag(ParrySkill);
    auto* Settings = Spec ? Cast<URParryTechnique>(Spec->GetPrimaryInstance() ? Spec->GetPrimaryInstance() : Spec->Ability.Get()) : nullptr;
    if (!Settings || !FMath::IsFinite(Settings->StaminaCost)) return;
    const float Cost = FMath::Max(0.f,Settings->StaminaCost);
    const float Stamina = GetNumericAttribute(URAttributeSet::GetStaminaAttribute());
    if (!FMath::IsFinite(Stamina) || Stamina<Cost) return;
    // Passive guard technique: commit its configured GAS cooldown when the window opens.
    if (!Settings->GetCooldownGameplayEffect() || !Settings->CommitAbilityCooldown(Spec->Handle,AbilityActorInfo.Get(),FGameplayAbilityActivationInfo(Player),false)) return;
    if (Cost>0.f)
    {
        UGameplayEffect* Effect = NewObject<UGameplayEffect>();
        Effect->DurationPolicy = EGameplayEffectDurationType::Instant;
        FGameplayModifierInfo Modifier;
        Modifier.Attribute = URAttributeSet::GetStaminaAttribute();
        Modifier.ModifierOp = EGameplayModOp::Additive;
        Modifier.ModifierMagnitude = FScalableFloat(-Cost);
        Effect->Modifiers.Add(Modifier);
        ApplyGameplayEffectToSelf(Effect,1.f,MakeEffectContext());
    }
    const double Now = GetWorld()->GetTimeSeconds();
    const float Window = FMath::IsFinite(Settings->ParryWindow) ? FMath::Clamp(Settings->ParryWindow,.05f,1.f) : .25f;
    ParryExpires = Now+Window;
}

bool URAbilitySystemComponent::IsParryWindowOpen()
{
    auto* Player = Cast<ARPlayer>(GetAvatarActor());
    return Player && HasEquippedMeleeWeapon() && Player->GetIsUsingBlock() && !Player->IsDead_Implementation() && GetLearnedSkillRank(ParrySkill)>0
        && !HasMatchingGameplayTag(TechniqueAttack) && !HasMatchingGameplayTag(TechniqueStunned) && !HasMatchingGameplayTag(TechniqueEvading)
        && GetWorld() && ParryExpires>=0
        && GetWorld()->GetTimeSeconds()<ParryExpires;
}

bool URAbilitySystemComponent::TryParryConfirmedBlock(AActor* Attacker)
{
    AActor* Avatar = GetAvatarActor();
    auto* Enemy = Cast<AREnemy>(Attacker);
    const auto Reject=[&](const TCHAR* Reason)
    {
        if (CVarParryDebug.GetValueOnGameThread())
            UE_LOG(LogTemp,Display,TEXT("ParryDebug: %s blocked %s: %s (learned=%d window=%d guarding=%d attack=%d stun=%d evade=%d)."),
                *GetNameSafe(Avatar),*GetNameSafe(Attacker),Reason,GetLearnedSkillRank(ParrySkill),IsParryWindowOpen(),
                Avatar && URAbilitySystemLibrary::IsActorBlocking(Avatar),HasMatchingGameplayTag(TechniqueAttack),
                HasMatchingGameplayTag(TechniqueStunned),HasMatchingGameplayTag(TechniqueEvading));
        return false;
    };
    if (!Avatar || !Avatar->HasAuthority() || !IsParryWindowOpen() || GetLearnedSkillRank(ParrySkill)<=0
        || !Enemy || Enemy->IsDead_Implementation() || !URAbilitySystemLibrary::IsActorBlocking(Avatar)
        || !URAbilitySystemLibrary::IsNotFriend(Avatar,Enemy) || !URAbilitySystemLibrary::IsValidBlock(Enemy,Avatar)
        || HasMatchingGameplayTag(TechniqueAttack) || HasMatchingGameplayTag(TechniqueStunned) || HasMatchingGameplayTag(TechniqueEvading)) return Reject(TEXT("not eligible for a timed parry"));
    const auto* Spec = GetSpecFromAbilityTag(ParrySkill);
    const auto* Settings = Spec ? Cast<URParryTechnique>(Spec->Ability) : nullptr;
    if (!Settings || !FMath::IsFinite(Settings->MaximumRange) || Avatar->GetDistanceTo(Enemy)>FMath::Max(1.f,Settings->MaximumRange)) return Reject(TEXT("missing settings or outside maximum range"));
    const FVector Direction = (Enemy->GetActorLocation()-Avatar->GetActorLocation()).GetSafeNormal();
    if (FVector::DotProduct(Avatar->GetActorForwardVector(),Direction)<.35f) return Reject(TEXT("attacker outside facing cone"));
    FHitResult Hit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(PlayerParry),false,Avatar);
    TArray<AActor*> Attached; Avatar->GetAttachedActors(Attached,true,true); Query.AddIgnoredActors(Attached);
    if (GetWorld()->LineTraceSingleByChannel(Hit,Avatar->GetActorLocation(),Enemy->GetActorLocation(),ECC_Visibility,Query) && Hit.GetActor()!=Enemy) return Reject(TEXT("visibility blocked"));
    ParryExpires = -1; // One confirmed melee contact per guard press, including control-immune targets.
    const bool bImmune = Enemy->ElementalResistanceProfile && Enemy->ElementalResistanceProfile->bImmuneToControl;
    auto* TargetASC = Enemy->GetAbilitySystemComponent();
    if (TargetASC && !bImmune)
    {
        Enemy->ApplyParryStagger(FMath::Clamp(Settings->StaggerSeconds,0.f,3.f),Avatar);
    }
    OnParrySucceeded.Broadcast(Enemy,TargetASC && !bImmune);
    if (CVarParryDebug.GetValueOnGameThread())
        UE_LOG(LogTemp,Display,TEXT("ParryDebug: confirmed timed parry against %s (control immune=%d, enemy ASC=%d, stagger=%.2fs)."),
            *GetNameSafe(Enemy),bImmune,TargetASC!=nullptr,Settings->StaggerSeconds);
    return true;
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
URPlayerKick::URPlayerKick()
{
    SetAssetTags(FGameplayTagContainer(KickSkill));
    DamageMultiplier=.65f;
    bTraceFromCharacterMesh=true;
    ActivationBlockedTags.AddTag(TechniqueAttack);
}

FString URPlayerKick::GetDescription(int32 Level)
{
    return FString::Printf(TEXT("<Title>Kick</>\n\n<Default>Strike a nearby enemy with a standing kick, or a spin kick while moving forward. No shield or weapon required.</>\n\n<Small>Stagger: </><Cooldown>%.2f s</>\n<Small>Cooldown: </><Cooldown>%.1f s</>\n<Default>Control-immune enemies resist stagger. Assign to an active input slot.</>"),StaggerSeconds,GetCooldown(Level));
}

FRMeleeAttack URPlayerKick::GetCurrentAttack() const
{
    if (CurrentAttackIndex!=INDEX_NONE) return SelectedKickAttack;
    const AActor* Avatar=GetAvatarActorFromActorInfo();
    if (Avatar && ForwardKick.Montage)
    {
        const FVector Velocity=Avatar->GetVelocity().GetSafeNormal2D();
        const float Speed=Avatar->GetVelocity().Size2D();
        if (Speed>FMath::Max(0.f,ForwardKickMinimumSpeed)
            && FVector::DotProduct(Avatar->GetActorForwardVector().GetSafeNormal2D(),Velocity)>.5f)
            return ForwardKick;
    }
    return Attacks.IsEmpty() ? FRMeleeAttack() : Attacks[0];
}

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
    const bool bKick=IsA<URPlayerKick>();
    if (bKick && !GetCooldownGameplayEffect()) return false;
    if (!ASC || !Player || Player->IsDead_Implementation() || ASC->GetLearnedSkillRank(bKick ? KickSkill : CounterSkill)<=0
        || ASC->HasMatchingGameplayTag(bKick ? KickCooldown : CounterCooldown) || GetAttackMontages().IsEmpty() || !GetAttackMontages()[0].Montage
        || (!bKick && !GetMeleeWeapon())) return false;
    return bKick || CanHitFollowupTarget(ASC->GetCounterstrikeTarget());
}
void URPlayerMeleeFollowup::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event)
{
    auto* ASC = CastChecked<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get());
    CounterTarget = ASC->GetCounterstrikeTarget(); bCounterHit=false;
    auto* Kick=Cast<URPlayerKick>(this);
    if (Kick) Kick->SelectedKickAttack=Kick->GetCurrentAttack();
    if (!CommitAbility(Handle,Info,Activation) || (!Kick && !ASC->ConsumeCounterstrike())) { EndAbility(Handle,Info,Activation,true,true); return; }
    {
        FGameplayTagContainer Block(FRGameplayTags::Get().ability_block); ASC->CancelAbilities(&Block);
        if (auto* Player=Cast<ARPlayer>(Info->AvatarActor.Get())) Player->SetIsUsingBlock(false);
    }
    if (!BeginMeleeAttack()) { EndAbility(Handle,Info,Activation,true,true); return; }
    // Both actions commit their configured GAS cooldown through CommitAbility.
    const auto Attack=GetCurrentAttack();
    auto* Open=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FRGameplayTags::Get().Event_MeleeBegin,nullptr,false,true);
    Open->EventReceived.AddDynamic(this,&URPlayerMeleeFollowup::OpenHitWindow); Open->ReadyForActivation();
    auto* Close=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FRGameplayTags::Get().Event_MeleeEnd,nullptr,false,true);
    Close->EventReceived.AddDynamic(this,&URPlayerMeleeFollowup::CloseHitWindow); Close->ReadyForActivation();
    // An incoming hit/attack can replace this montage after its normal blend-out starts.
    // Keep the interrupted callback in that interval so the ability cannot remain active forever.
    auto* Montage=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,Attack.Montage,1.f,Attack.SectionName,true,1.f,0.f,true);
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
    CounterTarget.Reset();
    Super::EndAbility(Handle,Info,Activation,bReplicate,bCancelled);
}
void URPlayerMeleeFollowup::ProcessMeleeHit(const FHitResult& Hit)
{
    if (!bCounterHit && (IsA<URPlayerKick>() || Hit.GetActor()==CounterTarget.Get()) && CanHitFollowupTarget(Hit.GetActor()))
    { bCounterHit=true; DamageTarget(Hit.GetActor()); }
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
    if (IsA<URPlayerKick>() && Enemy && !Enemy->IsDead_Implementation() && !bImmune)
    {
        Enemy->ApplyControlEffect(true,StaggerSeconds,GetAvatarActorFromActorInfo());
    }
}
