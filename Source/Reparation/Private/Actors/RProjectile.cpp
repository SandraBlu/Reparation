// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RProjectile.h"

#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UProjectileMovementComponent;
class USphereComponent;
// Sets default values
ARProjectile::ARProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//SphereComp->SetCollisionObjectType(ECC_Projectile);
	SetRootComponent(SphereComp);

	EffectComp = CreateDefaultSubobject<UNiagaraComponent>("EffectComp");
	EffectComp->SetupAttachment(SphereComp);

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MovementComp->InitialSpeed = 1000.f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->bInitialVelocityInLocalSpace = true;
	MovementComp->ProjectileGravityScale = 0.0f;

}


void ARProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void ARProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void ARProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ARProjectile::OnSphereOverlap);
}

