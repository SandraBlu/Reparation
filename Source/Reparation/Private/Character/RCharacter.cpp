// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/GameplayStatics.h>
#include "RGameplayTags.h"
#include "Actors/Weapon/RWeapon.h"
#include "Components/REquipmentComponent.h"


// Sets default values
ARCharacter::ARCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

}

UAbilitySystemComponent* ARCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

URFootstepsComponent* ARCharacter::GetFootstepsComp() const
{
	return FootstepsComp;
}