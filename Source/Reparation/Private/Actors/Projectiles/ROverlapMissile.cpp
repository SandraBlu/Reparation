// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/ROverlapMissile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "RGameplayTags.h"
#include "Components/SphereComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GAS/Debuff/DebuffNiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

AROverlapMissile::AROverlapMissile()
{
	PrimaryActorTick.bCanEverTick = false;
	SphereComp->SetSphereRadius(20.0f);
}

void AROverlapMissile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AROverlapMissile::OnActorOverlap);
}

void AROverlapMissile::OnHit()
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	bHit = true;
}

void AROverlapMissile::Destroyed()
{
	if (!bHit && !HasAuthority()) OnHit();
	Super::Destroyed();
}

bool AROverlapMissile::IsValidOverlap(AActor* OtherActor)
{
	if (DamageEffectParams.SourceAbilitySystemComponent == nullptr) return false;
	AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	if (SourceAvatarActor == OtherActor) return false;
	if (!URAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return false;
	return true;
}


void AROverlapMissile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidOverlap(OtherActor)) return;
	if (!bHit) OnHit();
	if (HasAuthority())
	{
		const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
		APawn* HitPawn = Cast<APawn>(OtherActor);
		bool bIsValidBlock = false;
		const bool bIsPlayerBlocking = URAbilitySystemLibrary::NativeDoesActorHaveTag(HitPawn,GameplayTags.Event_Blocking);
		if (bIsPlayerBlocking)
		{
			bIsValidBlock = URAbilitySystemLibrary::IsValidBlock(this, HitPawn);
		}
		FGameplayEventData Data;
		Data.Instigator = this;
		Data.Target = HitPawn;
		if (bIsValidBlock)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitPawn, GameplayTags.Event_Blocking_Successful, Data);
			
		}
		else
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
			{
				const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
				DamageEffectParams.DeathImpulse = DeathImpulse;
				const bool bKnockback = FMath::RandRange(1, 100) <DamageEffectParams.KnockbackChance;
				if (bKnockback)
				{
					FRotator Rotation = GetActorRotation();
					Rotation.Pitch = 25.f;
					const FVector KnockbackDirection = Rotation.Vector();
					const FVector KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackMagnitude;
					DamageEffectParams.KnockbackForce = KnockbackForce;
				}
			
				DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
				URAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
			}
		}
		Destroy();
	}
	else bHit = true;
}



