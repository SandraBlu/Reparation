// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/RFireball.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayCueManager.h"
#include "RGameplayTags.h"
#include "Framework/RAbilitySystemLibrary.h"

void ARFireball::BeginPlay()
{
	Super::BeginPlay();
	StartOutgoingTimeline();
}

void ARFireball::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidOverlap(OtherActor)) return;

	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			const bool bKnockback = FMath::RandRange(1, 100) <DamageEffectParams.KnockbackChance;
			
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			URAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}
}

void ARFireball::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


