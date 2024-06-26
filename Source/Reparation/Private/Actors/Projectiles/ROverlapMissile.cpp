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

void AROverlapMissile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor()
;	if (SourceAvatarActor == OtherActor) return;
	if (!URAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return;
	if (!bHit) OnHit();
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			URAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
		Destroy();
	}
	else bHit = true;
}



