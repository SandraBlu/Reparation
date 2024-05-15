// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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

}
