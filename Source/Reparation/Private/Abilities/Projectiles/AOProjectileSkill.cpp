// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Projectiles/AOProjectileSkill.h"
#include "Interfaces/CombatInterface.h"
#include "Actor/AOMagicMissile.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AOGameplayTags.h"


UAOProjectileSkill::UAOProjectileSkill()
{
	SweepRadius = 20.0f;
	SweepDistanceFallback = 5000;
}

void UAOProjectileSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}

void UAOProjectileSkill::SpawnProjectile(ACharacter* InstigatorCharacter)
{
	
	ICombatInterface* iCombat = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (iCombat)
	{
		const FVector SocketLocation = iCombat->GetLHandSocketLocation();
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = InstigatorCharacter;

		FCollisionShape Shape;
		Shape.SetSphere(SweepRadius);

		// Ignore Player
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(InstigatorCharacter);

		FVector TraceDirection = InstigatorCharacter->GetControlRotation().Vector();
		// Add sweep radius onto start to avoid the sphere clipping into floor/walls the camera is directly against.
		FVector TraceStart = InstigatorCharacter->GetPawnViewLocation() + (TraceDirection * SweepRadius);
		// endpoint far into the look-at distance (not too far, still adjust somewhat towards crosshair on a miss)
		FVector TraceEnd = TraceStart + (TraceDirection * SweepDistanceFallback);

		FHitResult Hit;
		// returns true if we got to a blocking hit (Channel1="Projectile" defined in DefaultGame.ini)
		if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, ECC_GameTraceChannel1, Shape, Params))
		{
			// Overwrite trace end with impact point in world
			TraceEnd = Hit.ImpactPoint;
		}
		// find new direction/rotation from Hand pointing to impact point in world.
		FRotator ProjRotation = (TraceEnd - SocketLocation).Rotation();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(ProjRotation.Quaternion());

		AAOMagicMissile* Missile = GetWorld()->SpawnActorDeferred<AAOMagicMissile>(ProjectileClass, SpawnTransform, GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.SetAbility(this);
		EffectContextHandle.AddSourceObject(Missile);
		TArray<TWeakObjectPtr<AActor>> Actors;
		Actors.Add(Missile);
		EffectContextHandle.AddActors(Actors);
		FHitResult HitResult;
		HitResult = Hit;
		EffectContextHandle.AddHitResult(HitResult);
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());

		const FAOGameplayTags GameplayTags = FAOGameplayTags::Get();

		for (auto& Pair : DamageTypes)
		{
			const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
		}
		
		Missile->DamageEffectSpecHandle = SpecHandle;

		Missile->FinishSpawning(SpawnTransform);
	}
}
