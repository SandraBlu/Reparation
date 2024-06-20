// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

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

void ARCharacterBase::Die()
{
	MulticastHandleDeath();
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


void ARCharacterBase::MulticastHandleDeath_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathCry, GetActorLocation(), GetActorRotation());
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	bDead = true;
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
	RASC->AddPassiveAbilities(GrantedPassiveAbilities);
}
	

