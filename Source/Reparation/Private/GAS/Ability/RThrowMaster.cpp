#include "GAS/Ability/RThrowMaster.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Characters/RPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Engine/World.h"
#include "Animation/AnimMontage.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Components/REquipmentComponent.h"
#include "Components/RInventoryComponent.h"
#include "Items/REquipItem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Controller.h"
#include "Actors/Projectiles/RThrowAimPreview.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"

namespace
{
    bool HasInventoryQuantity(const UREquipItem_Weapon* Item, UClass* ProjectileClass)
    {
        return IsValid(Item) && Item->WeaponClass == ProjectileClass && Item->GetQuantity() > 0
            && IsValid(Item->OwningInventory) && Item->OwningInventory->GetItems().Contains(Item);
    }

    UREquipItem_Weapon* FindThrowableItem(ARPlayer* Player, UClass* ProjectileClass)
    {
        if (!Player || !ProjectileClass) return nullptr;
        if (auto* Gear = Player->FindComponentByClass<UREquipmentComponent>())
        {
            auto* Item = Cast<UREquipItem_Weapon>(Gear->GetEquippedItemAtSlot(EEquipSlot::ES_Throwable));
            if (HasInventoryQuantity(Item, ProjectileClass)) return Item;
        }
        // Player inventories may live on the pawn, Player State, or controller.
        for (AActor* Owner : {static_cast<AActor*>(Player), static_cast<AActor*>(Player->GetPlayerState()), static_cast<AActor*>(Player->GetController())})
        {
            if (!Owner) continue;
            TArray<URInventoryComponent*> Inventories;
            Owner->GetComponents(Inventories);
            for (auto* Inventory : Inventories)
                for (auto* Entry : Inventory->GetItems())
                    if (auto* Item = Cast<UREquipItem_Weapon>(Entry); HasInventoryQuantity(Item, ProjectileClass)) return Item;
        }
        return nullptr;
    }
}

UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowRelease, "event.throw.release");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowAction, "status.throwing");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowDistractionSkill, "ability.throw.distraction");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowStarSkill, "ability.throw.star");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowSmokeSkill, "ability.throw.smokeGrenade");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowMolotovSkill, "ability.throw.molotov");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowBlastSkill, "ability.grenade.stun");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowStun, "debuff.stun");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowEvade, "status.evading");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowAttack, "ability.attack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowMelee, "ability.melee");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowPhysical, "damage.physical");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowFire, "damage.fire");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowDistractionCooldown, "cooldown.throw.distraction");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowStarCooldown, "cooldown.throw.star");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowSmokeCooldown, "cooldown.throw.smokeGrenade");
UE_DEFINE_GAMEPLAY_TAG_STATIC(ThrowMolotovCooldown, "cooldown.throw.molotov");

void URThrowMaster::ConfigureThrowKind(ERThrowableKind NewKind)
{
    if (!HasAnyFlags(RF_ClassDefaultObject)) return;
    Kind = NewKind;
    SkillTag = Kind==ERThrowableKind::Blast ? ThrowBlastSkill.GetTag() : Kind==ERThrowableKind::Distraction ? ThrowDistractionSkill.GetTag() : Kind==ERThrowableKind::Star ? ThrowStarSkill.GetTag() : Kind==ERThrowableKind::Smoke ? ThrowSmokeSkill.GetTag() : ThrowMolotovSkill.GetTag();
#if WITH_EDITOR
    EditorGetAssetTags() = FGameplayTagContainer(SkillTag);
#endif
    bSilentAttack = Kind!=ERThrowableKind::Molotov && Kind!=ERThrowableKind::Blast;
    if (Kind==ERThrowableKind::Blast) { bAimBeforeThrow=true; HandSocket=TEXT("throw_r"); }
    GravityScale = Kind==ERThrowableKind::Star ? 0.f : 1.f;
    LaunchSpeed = Kind==ERThrowableKind::Star ? 3000.f : 1400.f;
    AimDistance = Kind==ERThrowableKind::Star ? 5000.f : 3000.f;
    DamageType = Kind==ERThrowableKind::Molotov ? ThrowFire.GetTag() : ThrowPhysical.GetTag();
    Damage = FScalableFloat(Kind==ERThrowableKind::Molotov ? 10.f : 25.f);
}

URThrowMaster::URThrowMaster()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
    ThrowableClass = ARThrowable::StaticClass();
    AimPreviewClass = ARThrowAimPreview::StaticClass();
    ActivationOwnedTags.AddTag(ThrowAction);
    ActivationBlockedTags.AddTag(ThrowAction);
    ActivationBlockedTags.AddTag(ThrowStun);
    ActivationBlockedTags.AddTag(ThrowEvade);
    ActivationBlockedTags.AddTag(ThrowAttack);
    ActivationBlockedTags.AddTag(ThrowMelee);
    BlockAbilitiesWithTag.AddTag(ThrowMelee);
    BlockAbilitiesWithTag.AddTag(ThrowAttack);
    DamageType = ThrowPhysical;
    Damage = FScalableFloat(25.f);
    DebuffChance = 0.f;
}

URDistraction::URDistraction()
{
    Kind = ERThrowableKind::Distraction;
    SkillTag = ThrowDistractionSkill;
    SetAssetTags(FGameplayTagContainer(SkillTag));
    bSilentAttack = true;
}
URStar::URStar()
{
    Kind = ERThrowableKind::Star;
    SkillTag = ThrowStarSkill;
    SetAssetTags(FGameplayTagContainer(SkillTag));
    GravityScale = 0.f;
    LaunchSpeed = 3000.f;
    AimDistance = 5000.f;
    bSilentAttack = true;
}
URSmokeGrenade::URSmokeGrenade()
{
    Kind = ERThrowableKind::Smoke;
    SkillTag = ThrowSmokeSkill;
    SetAssetTags(FGameplayTagContainer(SkillTag));
    bSilentAttack = true;
}
URMolotov::URMolotov()
{
    Kind = ERThrowableKind::Molotov;
    SkillTag = ThrowMolotovSkill;
    SetAssetTags(FGameplayTagContainer(SkillTag));
    DamageType = ThrowFire;
    Damage = FScalableFloat(10.f);
    // The ground fire pulses damage; do not stack an additional random burn per pulse.
    DebuffChance = 0.f;
}

bool URThrowMaster::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* Source, const FGameplayTagContainer* Target, FGameplayTagContainer* Relevant) const
{
    if (!Super::CanActivateAbility(Handle, Info, Source, Target, Relevant)) return false;
    auto* Player = Info ? Cast<ARPlayer>(Info->AvatarActor.Get()) : nullptr;
    auto* ASC = Info ? Cast<URAbilitySystemComponent>(Info->AbilitySystemComponent.Get()) : nullptr;
    const bool bDamaging = Kind == ERThrowableKind::Star || Kind == ERThrowableKind::Molotov || Kind == ERThrowableKind::Blast;
    return Player && !Player->IsDead_Implementation() && !URAbilitySystemLibrary::IsActorBlocking(Player)
        && ASC && SkillTag.IsValid() && ASC->GetLearnedSkillRank(SkillTag)>0
        && ThrowableClass && (!bDamaging || DamageEffectClass)
        && FindThrowableItem(Player, Kind==ERThrowableKind::Star && bThrowKnife && KnifeClass ? KnifeClass.Get() : ThrowableClass.Get());
}

void URThrowMaster::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Event)
{
    Super::ActivateAbility(Handle, Info, Activation, Event);
    if (!IsActive()) return;
    bReleased = false;
    bWaitingForAim = false;
    auto* Player = Cast<ARPlayer>(GetAvatarActorFromActorInfo());
    if (!Player) { Interrupted(); return; }
    FActorSpawnParameters Spawn;
    Spawn.Owner = Spawn.Instigator = Player;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const TSubclassOf<ARThrowable> Class = Kind==ERThrowableKind::Star && bThrowKnife && KnifeClass ? KnifeClass : ThrowableClass;
    SourceItem = FindThrowableItem(Player, Class);
    if (!SourceItem) { Interrupted(); return; }
    HeldProjectile = GetWorld()->SpawnActor<ARThrowable>(Class, Player->GetActorTransform(), Spawn);
    if (!HeldProjectile) { Interrupted(); return; }
    SourceWeapon.Reset();
    if (auto* Gear = Player->FindComponentByClass<UREquipmentComponent>())
        if (auto* Item = Cast<UREquipItem_Weapon>(Gear->GetEquippedItemAtSlot(EEquipSlot::ES_Throwable));
            Item == SourceItem && IsValid(Gear->EquippedThrowable))
        {
            SourceItem = Item;
            SourceWeapon = Gear->EquippedThrowable;
            SourceWeapon->SetActorHiddenInGame(true);
        }
    HeldProjectile->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocket);
    if (bAimBeforeThrow)
    {
        bWaitingForAim = true;
        FActorSpawnParameters PreviewSpawn;
        PreviewSpawn.Owner = Player;
        AimPreview = GetWorld()->SpawnActor<ARThrowAimPreview>(AimPreviewClass, Player->GetActorTransform(), PreviewSpawn);
        UpdateAim();
        GetWorld()->GetTimerManager().SetTimer(AimTimer, this, &URThrowMaster::UpdateAim, 1.f/30.f, true);
        auto* Input = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
        Input->OnRelease.AddDynamic(this, &URThrowMaster::ConfirmAim);
        Input->ReadyForActivation();
        return;
    }
    BeginThrow();
}

void URThrowMaster::ConfirmAim(float HeldTime)
{
    if (!IsActive() || !bWaitingForAim) return;
    bWaitingForAim = false;
    BeginThrow();
}

void URThrowMaster::UpdateAim()
{
    CalculateTrajectory(AimedOrigin, AimedVelocity);
    if (IsValid(AimPreview) && IsValid(HeldProjectile))
        AimPreview->UpdateTrajectory(AimedOrigin, AimedVelocity, GravityScale, EffectRadius, HeldProjectile->Collision->GetScaledSphereRadius());
}

void URThrowMaster::BeginThrow()
{
    auto* Player = Cast<ARPlayer>(GetAvatarActorFromActorInfo());
    const bool bPlayableMontage = ThrowMontage && ThrowMontage->GetSkeleton() && Player && Player->GetMesh()
        && Player->GetMesh()->GetSkeletalMeshAsset()
        && (ThrowMontage->GetSkeleton() == Player->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton()
            || ThrowMontage->GetSkeleton()->IsCompatibleMesh(Player->GetMesh()->GetSkeletalMeshAsset()));
    if (!bPlayableMontage)
    {
        if (ThrowMontage)
            UE_LOG(LogTemp, Warning, TEXT("%s: Throw Montage %s is incompatible with the player's mesh; throwing immediately."), *GetName(), *GetNameSafe(ThrowMontage));
        Release(FGameplayEventData());
        if (IsActive()) Finished();
        return;
    }
    auto* ReleaseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ThrowRelease, nullptr, true, true);
    ReleaseTask->EventReceived.AddDynamic(this, &URThrowMaster::Release);
    ReleaseTask->ReadyForActivation();
    auto* Montage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ThrowMontage);
    Montage->OnCompleted.AddDynamic(this, &URThrowMaster::Finished);
    Montage->OnInterrupted.AddDynamic(this, &URThrowMaster::Interrupted);
    Montage->OnCancelled.AddDynamic(this, &URThrowMaster::Interrupted);
    Montage->ReadyForActivation();
}

void URThrowMaster::CalculateTrajectory(FVector& Origin, FVector& Velocity) const
{
    auto* Player = Cast<ARPlayer>(GetAvatarActorFromActorInfo());
    if (!Player) { Origin=FVector::ZeroVector; Velocity=FVector::ZeroVector; return; }
    FVector View;
    FRotator Rotation;
    Player->GetActorEyesViewPoint(View, Rotation);
    // Match the existing projectile ability's control-rotation targeting.
    Rotation = Player->GetControlRotation();
    FCollisionQueryParams Query;
    Query.AddIgnoredActor(Player);
    FHitResult Hit;
    FVector Target = View + Rotation.Vector() * AimDistance;
    if (GetWorld()->LineTraceSingleByChannel(Hit, View, Target, ECC_Visibility, Query)) Target = Hit.ImpactPoint;
    Origin = Player->GetActorLocation() + FVector(0,0,50) + Player->GetActorForwardVector()*50.f;
    if (Player->GetMesh() && Player->GetMesh()->DoesSocketExist(HandSocket)) Origin = Player->GetMesh()->GetSocketLocation(HandSocket);
    // Do not launch from a hand that has clipped through cover.
    if (GetWorld()->LineTraceSingleByChannel(Hit, Player->GetActorLocation()+FVector(0,0,50), Origin, ECC_Visibility, Query))
        Origin = Hit.ImpactPoint + Hit.ImpactNormal*10.f;
    Velocity = (Target-Origin).GetSafeNormal()*FMath::Max(1.f,LaunchSpeed);
}

void URThrowMaster::Release(FGameplayEventData Payload)
{
    auto* Player = Cast<ARPlayer>(GetAvatarActorFromActorInfo());
    if (!IsActive() || bReleased || bWaitingForAim || !Player || Player->IsDead_Implementation()) return;
    FVector Origin, Velocity;
    CalculateTrajectory(Origin, Velocity);
    const FVector Direction = Velocity.GetSafeNormal();
    const FTransform Transform(Direction.Rotation(), Origin);
    auto* Projectile = IsValid(HeldProjectile) ? HeldProjectile.Get() : nullptr;
    if (!Projectile) { Interrupted(); return; }
    // Possession can change during windup. Do not call inventory CanUse: it also checks equip/use state.
    if (!HasInventoryQuantity(SourceItem, Projectile->GetClass())) { Interrupted(); return; }
    // Commit only at release: interrupted windups spend no resources.
    if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Throw release failed cost/cooldown commit."), *GetName());
        Projectile->Destroy();
        Interrupted();
        return;
    }
    bReleased = true;
    Projectile->Kind = Kind;
    Projectile->Radius = FMath::Max(1.f,EffectRadius);
    Projectile->Duration = FMath::Max(.1f,EffectDuration);
    Projectile->PulseInterval = FMath::Max(.1f,EffectInterval);
    Projectile->NoiseRange = FMath::Max(1.f,HearingRange);
    Projectile->DistractionRadius = FMath::Max(0.f, DistractionRadius);
    Projectile->MaxDistractedEnemies = FMath::Max(0, MaxDistractedEnemies);
    Projectile->bSoundDistraction = bSoundDistraction;
    Projectile->bSightDistraction = bSightDistraction;
    Projectile->bDistractInCombat = bDistractInCombat;
    Projectile->DamageParams = MakeDamageEffectParamsFromClassDefaults();
    Projectile->Movement->InitialSpeed = FMath::Max(1.f,LaunchSpeed);
    Projectile->Movement->ProjectileGravityScale = FMath::Max(0.f,GravityScale);
    Projectile->Movement->Velocity = Direction * FMath::Max(1.f,LaunchSpeed);
    Projectile->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Projectile->SetActorTransform(Transform);
    Projectile->Launch(Velocity);
    GetWorld()->GetTimerManager().ClearTimer(AimTimer);
    if (IsValid(AimPreview)) AimPreview->Destroy();
    AimPreview = nullptr;
    UE_LOG(LogTemp, Log, TEXT("%s: Launched %s from %s with velocity %s."), *GetName(), *GetNameSafe(Projectile), *Origin.ToString(), *Projectile->Movement->Velocity.ToString());
}

void URThrowMaster::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation, bool bReplicateEndAbility, bool bWasCancelled)
{
    auto* UsedItem = bReleased ? SourceItem.Get() : nullptr;
    GetWorld()->GetTimerManager().ClearTimer(AimTimer);
    if (IsValid(AimPreview)) AimPreview->Destroy();
    AimPreview = nullptr;
    bWaitingForAim = false;
    bReleased = false;
    if (IsValid(HeldProjectile) && !HeldProjectile->IsInFlight()) HeldProjectile->Destroy();
    HeldProjectile = nullptr;
    if (SourceWeapon.IsValid() && (!UsedItem || UsedItem->GetQuantity() > 1)) SourceWeapon->SetActorHiddenInGame(false);
    SourceWeapon.Reset();
    SourceItem = nullptr;
    Super::EndAbility(Handle, Info, Activation, bReplicateEndAbility, bWasCancelled);
    // OnUse must see the completed ability, with status.throwing already removed.
    if (IsValid(UsedItem) && IsValid(UsedItem->OwningInventory))
        UsedItem->OwningInventory->CompleteThrowableUse(UsedItem);
}

void URThrowMaster::Finished()
{
    if (!IsActive()) return;
    if (!bReleased) Release(FGameplayEventData());
    if (!IsActive()) return;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, !bReleased);
}
void URThrowMaster::Interrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}
FString URThrowMaster::GetDescription(int32 Level)
{
    const TCHAR* Name = Kind==ERThrowableKind::Blast ? TEXT("Blast Grenade") : Kind==ERThrowableKind::Distraction ? TEXT("Distraction") : Kind==ERThrowableKind::Star ? TEXT("Throwing Star / Knife") : Kind==ERThrowableKind::Smoke ? TEXT("Smoke Grenade") : TEXT("Molotov");
    const TCHAR* Detail = Kind==ERThrowableKind::Blast ? TEXT("Hold to aim the projectile arc and blast marker. Release to throw; explodes on impact.") : Kind==ERThrowableKind::Distraction ? TEXT("Repeated noise draws the attention of AI hearing listeners.") : Kind==ERThrowableKind::Star ? TEXT("Throw a star or knife at the aimed target.") : Kind==ERThrowableKind::Smoke ? TEXT("Create temporary concealment; crouching is not required inside the cloud.") : TEXT("Create ground fire that damages hostile targets every pulse.");
    return FString::Printf(TEXT("<Title>%s</>\n\n<Default>%s</>\n\n<Small>Rank: </><Level>%d</>\n<Small>Damage per hit/pulse: </><Damage>%.1f</>\n<Small>Cooldown: </><Cooldown>%.1f s</>"), Name, Detail, Level,
        Kind==ERThrowableKind::Star || Kind==ERThrowableKind::Molotov || Kind==ERThrowableKind::Blast ? Damage.GetValueAtLevel(Level) : 0.f, GetCooldown(Level));
}
