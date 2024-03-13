// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AOCharacter.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Weapon/AOWeapon.h"
#include "Components/CapsuleComponent.h"


AAOCharacter::AAOCharacter()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
}

UAbilitySystemComponent* AAOCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAOCharacter::GrantAbilities()
{
	UAOAbilitySystemComponent* AOASC = CastChecked<UAOAbilitySystemComponent>(AbilitySystemComponent);

	AOASC->AddGrantedAbilities(GrantedAbilities);
}

UAnimMontage* AAOCharacter::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void AAOCharacter::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level) const
{
	if (GetAbilitySystemComponent() && EffectClass)
	{
		FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(EffectClass, Level, EffectContext);

		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
	}
}

void AAOCharacter::InitializeAttributes() const
{
	ApplyEffectToSelf(PrimaryAttributes, 1.f);
	ApplyEffectToSelf(SecondaryAttributes, 1.f);
	ApplyEffectToSelf(BaseAttributes, 1.f);
}

void AAOCharacter::InitAbilityActorInfo()
{

}

FVector AAOCharacter::GetRHandSocketLocation()
{
	return GetMesh()->GetSocketLocation(RHandProjectile);
}

FVector AAOCharacter::GetLHandSocketLocation()
{
	return GetMesh()->GetSocketLocation(LHandProjectile);
}
