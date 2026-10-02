// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RWeapon.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RGameplayTags.h"
#include "Characters/RPlayer.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


ARWeapon::ARWeapon()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->bReceivesDecals = false;
	WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = WeaponMesh;
    InactiveWeaponSocket = FName("inactive_katana");
	ActiveWeaponSocket = FName("weapon_r");
}

void ARWeapon::OnEquip()
{
    if (!IsValid(PawnOwner) || IsEquipped) return;
    GetWorldTimerManager().ClearTimer(HolsterTimer);
    AttachMeshToPawn(PawnOwner->GetMesh(), ActiveWeaponSocket);
    PlayEquipMontage("Draw");
    IsEquipped = true;
    if (EquipSFX) UGameplayStatics::PlaySoundAtLocation(this, EquipSFX, GetActorLocation());
}

void ARWeapon::OnUnEquip()
{
    if (!IsValid(PawnOwner)) return;
    const bool bWasEquipped = IsEquipped;
    float HolsterDuration = 0.f;
    if (bWasEquipped)
    {
        // Some weapons use one montage with Draw and Disarm sections.
        UAnimMontage* Montage = DisarmAnim;
        if (!Montage && EquipAnim && EquipAnim->GetSectionIndex(TEXT("Disarm")) != INDEX_NONE)
        {
            Montage = EquipAnim;
        }
        HolsterDuration = PlayWeaponMontage(Montage, TEXT("Disarm"));
    }
    IsEquipped = false;
    GetWorldTimerManager().ClearTimer(HolsterTimer);
    if (HolsterDuration > 0.f)
    {
        GetWorldTimerManager().SetTimer(HolsterTimer, this, &ThisClass::FinishHolster, HolsterDuration, false);
    }
    else FinishHolster();
    if (bWasEquipped && DisarmSFX) UGameplayStatics::PlaySoundAtLocation(this, DisarmSFX, GetActorLocation());
}

USkeletalMeshComponent* ARWeapon::GetWeaponMesh() const
{
    return WeaponMesh;
}

void ARWeapon::BeginPlay()
{
    Super::BeginPlay();

    PawnOwner = Cast<ARPlayer>(GetOwner());
}

void ARWeapon::PlayEquipMontage(const FName SectionName)
{
    PlayWeaponMontage(EquipAnim, SectionName);
}

float ARWeapon::PlayWeaponMontage(UAnimMontage* Montage, const FName SectionName)
{
    if (!IsValid(PawnOwner)) return 0.f;
    UAnimInstance* AnimInstance = PawnOwner->GetMesh()->GetAnimInstance();
    if (AnimInstance && Montage)
    {
        const float Duration = AnimInstance->Montage_Play(Montage);
        if (Duration <= 0.f) return 0.f;
        if (Montage->GetSectionIndex(SectionName) != INDEX_NONE)
        {
            AnimInstance->Montage_JumpToSection(SectionName, Montage);
        }
        return FMath::Max(0.f, Duration - AnimInstance->Montage_GetPosition(Montage));
    }
    return 0.f;
}

void ARWeapon::FinishHolster()
{
    GetWorldTimerManager().ClearTimer(HolsterTimer);
    if (IsValid(PawnOwner) && !IsEquipped)
    {
        AttachMeshToPawn(PawnOwner->GetMesh(), InactiveWeaponSocket);
    }
}

void ARWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(HolsterTimer);
    Super::EndPlay(EndPlayReason);
}

class ARPlayer* ARWeapon::GetPawnOwner() const
{
    return PawnOwner;
}

void ARWeapon::AttachMeshToPawn(USceneComponent* InParent, FName InSocketName)
{
    if (InParent)
    {
        FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
        AttachToComponent(InParent, TransformRules, InSocketName);
    }
}
