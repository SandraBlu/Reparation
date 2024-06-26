// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "RGameplayTags.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Debuff/DebuffNiagaraComponent.h"


// Sets default values
ARCharacterBase::ARCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	EffectDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("EffectDebuffComponent");
	EffectDebuffComponent->SetupAttachment(GetRootComponent());
	EffectDebuffComponent->DebuffTag = FRGameplayTags::Get().Debuff_Stun;
}

UAbilitySystemComponent* ARCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* ARCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

bool ARCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* ARCharacterBase::GetAvatar_Implementation()
{
	return this;
}

TArray<FTaggedMontage> ARCharacterBase::GetAttackMontages_Implementation()
{
	return AttackMontages;
}

UNiagaraSystem* ARCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

ECharacterClass ARCharacterBase::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

FOnASCRegistered ARCharacterBase::GetOnASCRegisteredDelegate()
{
	return OnASCRegistered;
}

FOnDeath ARCharacterBase::GetOnDeathDelegate()
{
	return OnDeath;
}

void ARCharacterBase::Die(const FVector& DeathImpulse)
{
}

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
	ApplyEffectToSelf(ResistanceAttributes, 1.f);
	ApplyEffectToSelf(VitalAttributes, 1.f);
}

void ARCharacterBase::GrantAbilities()
{
	URAbilitySystemComponent* RASC = CastChecked<URAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;
	RASC->AddGrantedAbilities(GrantedAbilities);
	RASC->AddPassiveAbilities(GrantedPassiveAbilities);
}
	

