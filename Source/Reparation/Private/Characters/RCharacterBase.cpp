// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RCharacterBase.h"
#include "AbilitySystemComponent.h"

// Sets default values
ARCharacterBase::ARCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

UAbilitySystemComponent* ARCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void ARCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARCharacterBase::InitAbilityActorInfo()
{
}
void ARCharacterBase::InitializeAttributes() const
{
	const FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(PrimaryAttributes, 1.f, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

