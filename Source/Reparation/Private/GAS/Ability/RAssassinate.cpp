#include "GAS/Ability/RAssassinate.h"
#include "AI/REnemy.h"
#include "AI/RAIController.h"
#include "Characters/RPlayer.h"
#include "Components/REquipmentComponent.h"
#include "Actors/RWeapon.h"
#include "Stealth/RStealthComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/SkeletalMesh.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(AssassinateSkill,"ability.stealth.assassinate");
UE_DEFINE_GAMEPLAY_TAG_STATIC(AssassinateHit,"event.assassinate.hit");
UE_DEFINE_GAMEPLAY_TAG_STATIC(Assassinating,"status.assassinating");
UE_DEFINE_GAMEPLAY_TAG_STATIC(AssassinationVictim,"status.assassinationVictim");
UE_DEFINE_GAMEPLAY_TAG_STATIC(AssassinateAttack,"ability.attack");

static bool IsCompatibleExecution(const FGameplayAbilityActorInfo* Info,const UAnimMontage* Montage)
{
    const auto* Mesh=Info ? Info->SkeletalMeshComponent.Get() : nullptr;
    const auto* Asset=Mesh ? Mesh->GetSkeletalMeshAsset() : nullptr;
    return Montage && Info && Info->GetAnimInstance() && Asset && Montage->GetSkeleton()
        && (Asset->GetSkeleton()==Montage->GetSkeleton() || Montage->GetSkeleton()->IsCompatibleMesh(Asset));
}

URAssassinate::URAssassinate()
{
    InstancingPolicy=EGameplayAbilityInstancingPolicy::InstancedPerActor;
    SetAssetTags(FGameplayTagContainer(AssassinateSkill));
    ActivationOwnedTags.AddTag(Assassinating);
    ActivationOwnedTags.AddTag(AssassinateAttack);
    for (const TCHAR* Name:{TEXT("ability.attack"),TEXT("ability.melee"),TEXT("status.throwing"),TEXT("status.blocking"),TEXT("status.evading"),TEXT("debuff.stun"),TEXT("debuff.stagger")})
        ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName(Name)));
    BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(TEXT("ability")));
    bSilentAttack=true;
}

const TArray<FRAssassinationMontage>& URAssassinate::MontagesFor(ERAssassinationMode Mode) const
{
    return Mode==ERAssassinationMode::Air ? AirMontages : Mode==ERAssassinationMode::HideSpot ? HideSpotMontages : GroundMontages;
}

bool URAssassinate::HasClearReach(const ARPlayer* Player,const AREnemy* Enemy) const
{
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AssassinationReach),false,Player);
    Params.AddIgnoredActor(Enemy);
    return !Player->GetWorld()->LineTraceTestByChannel(Player->GetActorLocation(),Enemy->GetActorLocation(),ECC_Visibility,Params);
}

bool URAssassinate::IsUnseen(ARPlayer* Player) const
{
    if (Player->Stealth && Player->Stealth->IsConcealed()) return true;
    for (TActorIterator<AREnemy> It(Player->GetWorld()); It; ++It)
    {
        auto* Observer=*It;
        if (Observer->IsDead_Implementation() || !URAbilitySystemLibrary::IsNotFriend(Player,Observer)
            || FVector::DistSquared(Player->GetActorLocation(),Observer->GetActorLocation())>FMath::Square(FMath::Max(0.f,ObserverRange))) continue;
        const auto* AI=Cast<ARAIController>(Observer->GetController());
        const float HalfAngle=AI ? AI->SightHalfAngle : 60.f;
        const FVector Direction=(Player->GetActorLocation()-Observer->GetActorLocation()).GetSafeNormal();
        if (FVector::DotProduct(Observer->GetActorForwardVector(),Direction)<FMath::Cos(FMath::DegreesToRadians(HalfAngle))) continue;
        FVector Eye; FRotator Rotation; Observer->GetActorEyesViewPoint(Eye,Rotation);
        FVector PlayerEye; Player->GetActorEyesViewPoint(PlayerEye,Rotation);
        FCollisionQueryParams Params(SCENE_QUERY_STAT(AssassinationSight),false,Observer);
        Params.AddIgnoredActor(Player);
        if (!Player->GetWorld()->LineTraceTestByChannel(Eye,PlayerEye,ECC_Visibility,Params)) return false;
    }
    return true;
}

bool URAssassinate::IsInJumpPath(ARPlayer* Player,const AREnemy* Enemy) const
{
    if (Player->GetActorLocation().Z-Enemy->GetActorLocation().Z<FMath::Max(0.f,MinimumAirHeight)) return false;
    const FVector Origin=Player->GetActorLocation(),Velocity=Player->GetVelocity();
    const FVector Gravity(0,0,Player->GetCharacterMovement()->GetGravityZ());
    const auto* Capsule=Enemy->GetCapsuleComponent();
    const float Radius=Capsule->GetScaledCapsuleRadius();
    const FVector Extent=Enemy->GetActorUpVector()*FMath::Max(0.f,Capsule->GetScaledCapsuleHalfHeight()-Radius);
    const FVector Bottom=Enemy->GetActorLocation()-Extent,Top=Enemy->GetActorLocation()+Extent;
    FVector Previous=Origin;
    const float Duration=FMath::Clamp(JumpPredictionTime,.05f,3.f);
    for (float Time=.05f; Time<Duration+.05f; Time+=.05f)
    {
        const float T=FMath::Min(Time,Duration);
        const FVector Next=Origin+Velocity*T+Gravity*(.5f*T*T);
        FCollisionQueryParams Params(SCENE_QUERY_STAT(AssassinationJump),false,Player);
        Params.AddIgnoredActor(Enemy);
        FHitResult Obstacle;
        const bool bBlocked=Player->GetWorld()->LineTraceSingleByChannel(Obstacle,Previous,Next,ECC_Visibility,Params);
        const FVector End=bBlocked ? Obstacle.ImpactPoint : Next;
        FVector OnPath,OnCapsule;
        FMath::SegmentDistToSegmentSafe(Previous,End,Bottom,Top,OnPath,OnCapsule);
        if (FVector::DistSquared(OnPath,OnCapsule)<=FMath::Square(Radius+FMath::Max(0.f,JumpPathRadius))) return true;
        if (bBlocked || T>=Duration) break;
        Previous=Next;
    }
    return false;
}

bool URAssassinate::EvaluateTarget(ARPlayer* Player,AREnemy* Enemy,ERAssassinationMode& Mode) const
{
    if (!Player || !IsValid(Enemy) || Player->IsDead_Implementation() || Enemy->IsDead_Implementation()
        || !Enemy->bCanBeAssassinated || !URAbilitySystemLibrary::IsNotFriend(Player,Enemy)) return false;
    const auto* EnemyASC=Enemy->GetAbilitySystemComponent();
    if (!EnemyASC || EnemyASC->HasMatchingGameplayTag(AssassinationVictim)) return false;
    const auto* Gear=Player->FindComponentByClass<UREquipmentComponent>();
    const auto* Weapon=Gear ? Gear->EquippedWeapon : nullptr;
    if (!IsValid(Weapon) || !Weapon->IsEquipped || (Weapon->CombatType!=ECombatType::ECT_LightSword && Weapon->CombatType!=ECombatType::ECT_Staff)) return false;
    if (!Player->GetCharacterMovement()) return false;
    if (Player->GetCharacterMovement()->IsFalling())
    {
        Mode=ERAssassinationMode::Air;
        return IsInJumpPath(Player,Enemy) && IsUnseen(Player);
    }
    if (!Player->GetCharacterMovement()->IsMovingOnGround() || !Player->Stealth || !Player->Stealth->IsQuiet(QuietSpeed,QuietSeconds)) return false;
    Mode=Player->Stealth->IsInSneakArea() ? ERAssassinationMode::HideSpot : ERAssassinationMode::Ground;
    if (Mode==ERAssassinationMode::Ground && !Player->bIsCrouched) return false;
    const float Range=Mode==ERAssassinationMode::HideSpot ? HideSpotRange : GroundRange;
    return FVector::DistSquared(Player->GetActorLocation(),Enemy->GetActorLocation())<=FMath::Square(FMath::Max(0.f,Range))
        && HasClearReach(Player,Enemy) && IsUnseen(Player);
}

AREnemy* URAssassinate::FindTarget(ARPlayer* Player,ERAssassinationMode& Mode) const
{
    if (!Player) return nullptr;
    AREnemy* Best=nullptr; float BestDistance=TNumericLimits<float>::Max();
    for (TActorIterator<AREnemy> It(Player->GetWorld()); It; ++It)
    {
        ERAssassinationMode CandidateMode;
        if (!EvaluateTarget(Player,*It,CandidateMode)) continue;
        const float Distance=FVector::DistSquared(Player->GetActorLocation(),It->GetActorLocation());
        if (Distance<BestDistance) { Best=*It; BestDistance=Distance; Mode=CandidateMode; }
    }
    return Best;
}

AREnemy* URAssassinate::FindAssassinationTarget() const
{
    ERAssassinationMode Mode;
    return FindTarget(Cast<ARPlayer>(GetAvatarActorFromActorInfo()),Mode);
}

bool URAssassinate::CanAssassinateTarget(AREnemy* Enemy,ERAssassinationMode& Mode) const
{
    return EvaluateTarget(Cast<ARPlayer>(GetAvatarActorFromActorInfo()),Enemy,Mode);
}

bool URAssassinate::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* Source,const FGameplayTagContainer* Target,FGameplayTagContainer* Relevant) const
{
    if (!Super::CanActivateAbility(Handle,Info,Source,Target,Relevant)) return false;
    auto* ASC=Info ? Cast<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get()) : nullptr;
    auto* Player=Info ? Cast<ARPlayer>(Info->AvatarActor.Get()) : nullptr;
    ERAssassinationMode Mode;
    if (!ASC || ASC->GetLearnedSkillRank(AssassinateSkill)<=0 || !FindTarget(Player,Mode)) return false;
    return MontagesFor(Mode).ContainsByPredicate([Info](const FRAssassinationMontage& Entry)
        { return IsCompatibleExecution(Info,Entry.Montage); });
}

void URAssassinate::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event)
{
    Super::ActivateAbility(Handle,Info,Activation,Event);
    if (!IsActive()) return;
    auto* Player=Cast<ARPlayer>(Info->AvatarActor.Get());
    AssassinationTarget=FindTarget(Player,ActiveMode);
    TArray<FRAssassinationMontage> Options;
    for (const auto& Entry:MontagesFor(ActiveMode))
        if (IsCompatibleExecution(Info,Entry.Montage)) Options.Add(Entry);
    if (!AssassinationTarget || Options.IsEmpty() || !CommitAbility(Handle,Info,Activation)) { Interrupted(); return; }
    bStrikeApplied=false;
    auto* Enemy=AssassinationTarget.Get();
    Enemy->GetAbilitySystemComponent()->AddLooseGameplayTag(AssassinationVictim);
    bTargetClaimed=true;
    if (auto* AI=Cast<AAIController>(Enemy->GetController())) AI->StopMovement();
    auto* Movement=Enemy->GetCharacterMovement();
    SavedTargetMovement=Movement->MovementMode; SavedTargetCustomMode=Movement->CustomMovementMode;
    Movement->DisableMovement();
    if (Player->GetController()) { Player->GetController()->SetIgnoreMoveInput(true); bIgnoredMoveInput=true; }
    if (ActiveMode!=ERAssassinationMode::Air) Player->GetCharacterMovement()->StopMovementImmediately();
    FVector Facing=Enemy->GetActorLocation()-Player->GetActorLocation(); Facing.Z=0;
    if (!Facing.IsNearlyZero()) Player->SetActorRotation(Facing.Rotation());
    const auto& Entry=Options[FMath::RandRange(0,Options.Num()-1)];
    ActiveVictimMontage=Entry.VictimMontage;
    if (auto* Anim=Enemy->GetMesh()->GetAnimInstance(); Anim && ActiveVictimMontage) Anim->Montage_Play(ActiveVictimMontage);
    auto* Hit=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,AssassinateHit,nullptr,false,true);
    Hit->EventReceived.AddDynamic(this,&URAssassinate::Strike); Hit->ReadyForActivation();
    auto* Play=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,Entry.Montage);
    Play->OnCompleted.AddDynamic(this,&URAssassinate::Finished);
    Play->OnInterrupted.AddDynamic(this,&URAssassinate::Interrupted);
    Play->OnCancelled.AddDynamic(this,&URAssassinate::Interrupted);
    Play->ReadyForActivation();
}

void URAssassinate::Strike(FGameplayEventData Payload)
{
    if (!IsActive() || bStrikeApplied) return;
    auto* Player=Cast<ARPlayer>(GetAvatarActorFromActorInfo());
    auto* Enemy=AssassinationTarget.Get();
    if (!Player || !Player->HasAuthority() || Player->IsDead_Implementation() || !IsValid(Enemy)
        || Enemy->IsDead_Implementation() || !Enemy->bCanBeAssassinated
        || FVector::DistSquared(Player->GetActorLocation(),Enemy->GetActorLocation())>FMath::Square(FMath::Max(0.f,StrikeReach))
        || !HasClearReach(Player,Enemy)) return;
    bStrikeApplied=true;
    auto* EnemyASC=Enemy->GetAbilitySystemComponent();
    if (!IsValid(EnemyASC)) return;
    const float Health=EnemyASC->GetNumericAttribute(URAttributeSet::GetHealthAttribute());
    if (!FMath::IsFinite(Health) || Health<=0.f) return;
    UGameplayEffect* Effect=NewObject<UGameplayEffect>();
    Effect->DurationPolicy=EGameplayEffectDurationType::Instant;
    FGameplayModifierInfo Modifier;
    Modifier.Attribute=URAttributeSet::GetDamageAttribute(); Modifier.ModifierOp=EGameplayModOp::Additive;
    Modifier.ModifierMagnitude=FScalableFloat(Health+1.f); Effect->Modifiers.Add(Modifier);
    auto* ASC=GetAbilitySystemComponentFromActorInfo();
    auto Context=ASC->MakeEffectContext(); Context.AddSourceObject(Player);
    ASC->ApplyGameplayEffectSpecToTarget(FGameplayEffectSpec(Effect,Context,GetAbilityLevel()),EnemyASC);
}

void URAssassinate::Finished() { if (IsActive()) EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,false,false); }
void URAssassinate::Interrupted() { if (IsActive()) EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,false,true); }

void URAssassinate::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation,bool bReplicate,bool bCancelled)
{
    if (!IsEndAbilityValid(Handle,Info)) return;
    auto* Player=Info ? Cast<ARPlayer>(Info->AvatarActor.Get()) : nullptr;
    if (bIgnoredMoveInput && Player && Player->GetController()) Player->GetController()->SetIgnoreMoveInput(false);
    bIgnoredMoveInput=false;
    if (auto* Enemy=AssassinationTarget.Get(); IsValid(Enemy) && bTargetClaimed)
    {
        if (auto* ASC=Enemy->GetAbilitySystemComponent()) ASC->RemoveLooseGameplayTag(AssassinationVictim);
        if (!Enemy->IsDead_Implementation())
        {
            if (Enemy->GetCharacterMovement()->MovementMode==MOVE_None) Enemy->GetCharacterMovement()->SetMovementMode(SavedTargetMovement,SavedTargetCustomMode);
            if (auto* Anim=Enemy->GetMesh()->GetAnimInstance(); Anim && ActiveVictimMontage) Anim->Montage_Stop(.1f,ActiveVictimMontage);
        }
    }
    bTargetClaimed=false;
    AssassinationTarget=nullptr; ActiveVictimMontage=nullptr;
    Super::EndAbility(Handle,Info,Activation,bReplicate,bCancelled);
}

FString URAssassinate::GetDescription(int32 Level)
{
    return TEXT("<Title>Assassinate</>\n\n<Default>Ground: crouched, quiet and unseen. Hide spot: quiet, unseen and close. Air: unseen, with a target in your jump path. A hit notify performs a lethal silent execution. Requires a drawn melee weapon. Air requires its own authored montage.</>");
}
