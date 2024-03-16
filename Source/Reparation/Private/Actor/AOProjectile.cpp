// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AOProjectile.h"
#include "Reparation/Reparation.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Attributes/AOAttributeSet.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAOProjectile::AAOProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionObjectType(ECC_Projectile);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = SphereComp;

	EffectComp = CreateDefaultSubobject<UNiagaraComponent>("EffectComp");
	EffectComp->SetupAttachment(SphereComp);

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MovementComp->InitialSpeed = 1000.f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->bInitialVelocityInLocalSpace = true;

	LaunchSFX = CreateDefaultSubobject<UAudioComponent>("AudioComp");
	LaunchSFX->SetupAttachment(RootComponent);
}

void AAOProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(Lifespan);
}

void AAOProjectile::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Explode();
}

void AAOProjectile::Explode_Implementation()
{
	// Check to make sure we aren't already being 'destroyed'
		// Adding ensure to see if we encounter this situation at all
	if (IsValid(this))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSFX, GetActorLocation());

		Destroy();
	}
}

void AAOProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// More consistent to bind here compared to Constructor which may fail to bind if Blueprint was created before adding this binding (or when using hotreload)
	// PostInitializeComponent is the preferred way of binding any events.
	SphereComp->OnComponentHit.AddDynamic(this, &AAOProjectile::OnActorHit);
}

