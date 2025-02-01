// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RWeapon.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RGameplayTags.h"
#include "Characters/RPlayer.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


ARWeapon::ARWeapon()
{
	SKMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SKMesh->bReceivesDecals = false;
	SKMesh->SetCollisionObjectType(ECC_WorldDynamic);
	SKMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SKMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = SKMesh;
    InactiveWeaponSocket = FName("inactive_katana");
	ActiveWeaponSocket = FName("weapon_r");
}

void ARWeapon::OnEquip()
{
    AttachMeshToPawn(PawnOwner->GetMesh(), ActiveWeaponSocket);
    PlayEquipMontage("Draw");
}

void ARWeapon::OnUnEquip()
{
    AttachMeshToPawn(PawnOwner->GetMesh(), InactiveWeaponSocket);
}

USkeletalMeshComponent* ARWeapon::GetWeaponMesh() const
{
    return SKMesh;
}

void ARWeapon::BeginPlay()
{
    Super::BeginPlay();

    PawnOwner = Cast<ARPlayer>(GetOwner());
}

void ARWeapon::PlayEquipMontage(const FName SectionName)
{
    UAnimInstance* AnimInstance = PawnOwner->GetMesh()->GetAnimInstance();
    if (AnimInstance && EquipAnim)
    {
        AnimInstance->Montage_Play(EquipAnim);
        AnimInstance->Montage_JumpToSection(SectionName, EquipAnim);
    }
}

class ARPlayer* ARWeapon::GetPawnOwner() const
{
    return PawnOwner;
}

void ARWeapon::AttachMeshToPawn(USceneComponent* InParent, FName InSocketName)
{
    if (PawnOwner)
    {
        FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
        USkeletalMeshComponent* PawnMesh = PawnOwner->GetMesh();
        AttachToComponent(PawnOwner->GetMesh(), TransformRules, InSocketName);
    }
}