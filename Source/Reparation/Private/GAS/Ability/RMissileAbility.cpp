// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RMissileAbility.h"
#include "RGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Actors/Projectiles/ROverlapMissile.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/RCombatInterface.h"
#include "Kismet/GameplayStatics.h"

URMissileAbility::URMissileAbility()
{
	SweepRadius = 20.0f;
	SweepDistanceFallback = 5000;
}

FString URMissileAbility::GetDescription(int32 Level)
{
	return FString();
}

FString URMissileAbility::GetNextLevelDescription(int32 Level)
{
	return FString();
}

void URMissileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	const bool bIsServer = HasAuthority(&ActivationInfo);
	if (!bIsServer) return;
	
}

void URMissileAbility::SpawnProjectiles(ACharacter* InstigatorCharacter, bool bOverridePitch, float PitchOverride,
	AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	const FVector SocketLocation = IRCombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), FRGameplayTags::Get().combatSocket_weapon);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(InstigatorCharacter);
	FCollisionShape Shape;
	Shape.SetSphere(SweepRadius);
	
	FVector TraceDirection = InstigatorCharacter->GetControlRotation().Vector();
	FVector TraceStart = InstigatorCharacter->GetPawnViewLocation() + (TraceDirection * SweepRadius);
	FVector TraceEnd = TraceStart + (TraceDirection * SweepDistanceFallback);
	FHitResult Hit;
	if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, ECC_GameTraceChannel1, Shape, Params))
	{
		TraceEnd = Hit.Location;
		TraceEnd = Hit.ImpactPoint;
	}
	FRotator Rotation = (TraceEnd - SocketLocation).Rotation();
	if (bOverridePitch) Rotation.Pitch = PitchOverride;
	
	const FVector Forward = Rotation.Vector();
	const int32 EffectiveNumMissiles = FMath::Min(NumProjectiles, GetAbilityLevel());
	
	TArray<FRotator> Rotations = URAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, SweepRadius, EffectiveNumMissiles);

	for (const FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform = FTransform(Rot, SocketLocation);
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());
		
		AROverlapMissile* Missile = GetWorld()->SpawnActorDeferred<AROverlapMissile>(ProjectileClass, SpawnTransform, GetOwningActorFromActorInfo(), Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
		Missile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		if (HomingTarget && HomingTarget->Implements<URCombatInterface>())
		{
			Missile->MovementComp->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			Missile->HomingTarget = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Missile->HomingTarget->SetWorldLocation(TraceEnd);
			Missile->MovementComp->HomingTargetComponent = Missile->HomingTarget;
		}
		Missile->MovementComp->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
		Missile->MovementComp->bIsHomingProjectile = bLaunchHomingProjectiles;
		Missile->FinishSpawning(SpawnTransform);
	}
}

FHitResult URMissileAbility::GetHit(bool bBlockingHit, AActor* HitActor)
{
	FHitResult Hit;
	Hit.bBlockingHit = bBlockingHit;
	Hit.HitObjectHandle = FActorInstanceHandle(HitActor);
	return Hit;
}

