// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/RAbilitySystemComponent.h"

// Sets default values
ARCharacterBase::ARCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

UAbilitySystemComponent* ARCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* ARCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

// Called when the game starts or when spawned
void ARCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARCharacterBase::InitAbilityActorInfo()
{
}

void ARCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
 	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
 	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ARCharacterBase::InitializeAttributes() const
{
	ApplyEffectToSelf(PrimaryAttributes, 1.f);
	ApplyEffectToSelf(SecondaryAttributes, 1.f);
	ApplyEffectToSelf(VitalAttributes, 1.f);
}

void ARCharacterBase::GrantAbilities()
{
	URAbilitySystemComponent* RASC = CastChecked<URAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;
	RASC->AddGrantedAbilities(GrantedAbilities);
}
	

