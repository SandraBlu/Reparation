// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RLightningBolt.h"

#include "RGameplayTags.h"
#include "Actors/Projectiles/ROverlapMissile.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

FString URLightningBolt::GetDescription(int32 Level)
{
	
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float EnergyCost = FMath::Abs(GetEnergyCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Electric Bolt</>\n\n"
		//Details: Level, EnergyCost, cooldown
		"<Small>Level: </><Level>%d</>\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Description: 
		"<Default>Launches a shocking bolt.</>\n\n"
		//Damage
		"<Default>Spreads on impact dealing:</> <Damage>%d</> <Default>electric damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Electric Bolt</>\n\n"
		//Details: Level, EnergyCost, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Num Projectiles
		"<Default>Launches %d electrifying bolts.</>\n\n"
		//Damage
		"<Default>Spreads rapidly on impact dealing:</> <Damage>%d</> <Default>electric damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, FMath::Min(Level, NumProjectiles), ScaledDamage);
	}
}

FString URLightningBolt::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float EnergyCost = FMath::Abs(GetEnergyCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		//Title
		"<Title>Next Level:</>\n\n"
		//Details: Level, EnergyCost, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Num Projectiles
		"<Default>Launches %d electrifying bolts.</>\n\n"
		//Damage
		"<Default>Spreads rapidly on impact dealing:</> <Damage>%d</> <Default>electric damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, FMath::Min(Level, NumProjectiles), ScaledDamage);
}

void URLightningBolt::SpawnProjectiles(ACharacter* InstigatorCharacter, bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
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
