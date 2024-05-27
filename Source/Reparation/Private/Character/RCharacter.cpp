// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/GameplayStatics.h>
#include "RGameplayTags.h"


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

URAbilitySystemComponent* ARCharacter::GetASC()
{
	if (RASC == nullptr)
	{
		RASC = CastChecked<URAbilitySystemComponent>(AbilitySystemComponent);
	}
	return RASC;
}

void ARCharacter::InitAbilityActorInfo()
{

}

void ARCharacter::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level) const
{
	if (GetAbilitySystemComponent() && EffectClass)
	{
		FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(EffectClass, Level, EffectContext);

		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
	}
}

void ARCharacter::InitializeAttributes() const
{
	ApplyEffectToSelf(PrimaryAttributes, 1.f);
	ApplyEffectToSelf(SecondaryAttributes, 1.f);
	ApplyEffectToSelf(BaseAttributes, 1.f);
	ApplyEffectToSelf(ResistanceAttributes, 1.f);
}

void ARCharacter::GrantAbilities()
{
	//URAbilitySystemComponent* RASComp = CastChecked<URAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;
	//RASComp->AddGrantedAbilities(GrantedAbilities);
	GetASC()->AddGrantedAbilities(GrantedAbilities);
	//GetASC()->AddPassiveAbilities(PassiveAbilities);
}

TArray<FTaggedMontage> ARCharacter::GetAttackMontages_Implementation()
{
	return AttackMontages;
}

AActor* ARCharacter::GetAvatar_Implementation()
{
	return this;
}

UAnimMontage* ARCharacter::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

FVector ARCharacter::GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag)
{
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
	//if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_weapon) && IsValid(EquippedWeapon))
	//{
//		return EquippedWeapon->GetWeaponMesh()->GetSocketLocation(EquippedWeapon->FiringSocket);
//	}
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_handL))
	{
		return GetMesh()->GetSocketLocation(LHand);
	}
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_handR))
	{
		return GetMesh()->GetSocketLocation(RHand);
	}
	return FVector();
}

UNiagaraSystem* ARCharacter::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage ARCharacter::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for (FTaggedMontage TaggedMontage : AttackMontages)
	{
		if (TaggedMontage.MontageTag == MontageTag)
		{
			return TaggedMontage;
		}
	}
	return FTaggedMontage();
}

void ARCharacter::Die()
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSFX, GetActorLocation(), GetActorRotation());
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bDead = true;
	OnDeath.Broadcast(this);
}

FOnDeath ARCharacter::GetOnDeathDelegate()
{
	return OnDeath;
}

bool ARCharacter::IsDead_Implementation() const
{
	return bDead;
}
