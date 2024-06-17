// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/ROverlapMissile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
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

void AROverlapMissile::Destroyed()
{
	if (!bHit && !HasAuthority())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	Super::Destroyed();
}

void AROverlapMissile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (DamageEffectSpecHandle.Data.IsValid() && DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() == OtherActor)
	{
		return;
	}
	if (!URAbilitySystemLibrary::IsNotFriend(DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser(), OtherActor))
	{
		return;
	}
	if (!bHit)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}
		Destroy();
	}
	else
	{
		bHit = true;
	}
}



