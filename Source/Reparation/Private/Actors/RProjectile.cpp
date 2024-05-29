// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARProjectile::ARProjectile()
{
	bReplicates = true;
	
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//SphereComp->SetCollisionObjectType(ECC_Projectile);
	RootComponent = SphereComp;
	
	SphereComp->SetSphereRadius(20.0f);
	InitialLifeSpan = 7.0f;

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

void ARProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//AActor* SourceAvatarActor = DamageEffectParams.SourceASC->GetAvatarActor();
//	if  (SourceAvatarActor == OtherActor) return;
	//if (!UAOBFL::IsNotFriend(SourceAvatarActor, OtherActor)) return;
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
	{
		//DamageEffectParams.TargetASC = TargetASC;
		//UAOBFL::ApplyDamageEffect(DamageEffectParams);
	}
	Destroy();
}

void ARProjectile::PostInitializeComponents()
{
	// PostInitializeComponent is the preferred way of binding any events.
	Super::PostInitializeComponents();
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ARProjectile::OnActorOverlap);
	SetLifeSpan(Lifespan);
}

