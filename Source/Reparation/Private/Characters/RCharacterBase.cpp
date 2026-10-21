// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RCharacterBase.h"
#include "AI/RProtectionComponent.h"
#include "AbilitySystemComponent.h"
#include "MotionWarpingComponent.h"
#include "RGameplayTags.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Debuff/DebuffNiagaraComponent.h"
#include "GAS/Passive/PassiveNiagaraComponent.h"
#include "Stealth/RStealthComponent.h"
#include "Engine/World.h"


class ARGameMode;
// Sets default values
ARCharacterBase::ARCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	Stealth = CreateDefaultSubobject<URStealthComponent>(TEXT("Stealth"));
	EffectDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("EffectDebuffComponent");
	Protection = CreateDefaultSubobject<URProtectionComponent>(TEXT("Protection"));
	EffectDebuffComponent->SetupAttachment(GetRootComponent());
	EffectDebuffComponent->DebuffTag = FRGameplayTags::Get().Debuff_Stun;

	EffectAttachComp = CreateDefaultSubobject<USceneComponent>("EffectAttachPoint");
	EffectAttachComp->SetupAttachment(GetRootComponent());
	PassiveCloak = CreateDefaultSubobject<UPassiveNiagaraComponent>("CloakComponent");
	PassiveCloak->SetupAttachment(EffectAttachComp);
	PassiveLifeSiphon = CreateDefaultSubobject<UPassiveNiagaraComponent>("LifeSiphonComponent");
	PassiveLifeSiphon->SetupAttachment(EffectAttachComp);
	PassiveStaminaSiphon = CreateDefaultSubobject<UPassiveNiagaraComponent>("StaminaSiphonComponent");
	PassiveStaminaSiphon->SetupAttachment(EffectAttachComp);

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

void ARCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EffectAttachComp->SetWorldRotation((FRotator::ZeroRotator));
}

UAISense_Sight::EVisibilityResult ARCharacterBase::CanBeSeenFrom(const FCanBeSeenFromContext& Context,
	FVector& OutSeenLocation, int32& OutNumberOfLoSChecksPerformed, int32& OutNumberOfAsyncLosCheckRequested,
	float& OutSightStrength, int32* UserData, const FOnPendingVisibilityQueryProcessedDelegate* Delegate)
{
	OutNumberOfLoSChecksPerformed = 0;
	OutNumberOfAsyncLosCheckRequested = 0;
	OutSightStrength = 0.f;
	if (bDead || !GetWorld() || (Stealth && Stealth->IsConcealedFrom(Context.ObserverLocation,
		Context.bWasVisible && *Context.bWasVisible))) return UAISense_Sight::EVisibilityResult::NotVisible;

	OutSeenLocation = GetActorLocation();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(StealthSight), false);
	if (Context.IgnoreActor) Params.AddIgnoredActor(Context.IgnoreActor);
	FHitResult Hit;
	++OutNumberOfLoSChecksPerformed;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Context.ObserverLocation, OutSeenLocation, ECC_Visibility, Params);
	if (!bHit || Hit.GetActor() == this || (Hit.GetActor() && Hit.GetActor()->IsOwnedBy(this)))
	{
		OutSightStrength = 1.f;
		return UAISense_Sight::EVisibilityResult::Visible;
	}
	return UAISense_Sight::EVisibilityResult::NotVisible;
}

float ARCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageTaken =  Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamageDelegate.Broadcast(DamageTaken);
	return DamageTaken;
}

UAbilitySystemComponent* ARCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPawnCombatComponent* ARCharacterBase::GetPawnCombatComponent() const
{
	return CombatComp;
}

UAnimMontage* ARCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

UAnimMontage* ARCharacterBase::GetStunnedMontage_Implementation()
{
	return StunnedMontage;
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

FOnDamageSignature& ARCharacterBase::GetOnDamageSignature()
{
	return OnDamageDelegate;
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
	

