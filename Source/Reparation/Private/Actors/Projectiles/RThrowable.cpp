#include "Actors/Projectiles/RThrowable.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "RGameplayTags.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Interfaces/RCombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Stealth/RSneakArea.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "AI/REnemy.h"
#include "AI/RAIController.h"
#include "EngineUtils.h"

ARThrowable::ARThrowable()
{
    bReplicates = false;
    SetReplicateMovement(false);
    PrimaryActorTick.bCanEverTick = false;
    CombatType = ECombatType::ECT_Throwable;
    WeaponTag = FRGameplayTags::Get().weapon_throwable;
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    WeaponMesh->SetupAttachment(Collision);
    Collision->InitSphereRadius(8.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    Collision->SetCanEverAffectNavigation(false);
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Collision);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MotionEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MotionEffect"));
    MotionEffect->SetupAttachment(Collision);
    MotionEffect->SetAutoActivate(false);
    MotionEffect->SetAutoDestroy(false);
    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    Movement->SetAutoActivate(false);
    Movement->InitialSpeed = 1400.f;
    Movement->MaxSpeed = 0.f;
    Movement->ProjectileGravityScale = 1.f;
    Movement->bRotationFollowsVelocity = true;
    Movement->bInitialVelocityInLocalSpace = false;
    Movement->bForceSubStepping = true;
    InitialLifeSpan = 0.f;
}

void ARThrowable::BeginPlay()
{
    Super::BeginPlay();
    Collision->IgnoreActorWhenMoving(GetInstigator(), true);
    Collision->OnComponentHit.AddDynamic(this, &ARThrowable::OnHit);
}

void ARThrowable::OnEquip()
{
    if (!bInFlight && !bDetonated) Super::OnEquip();
}

void ARThrowable::OnUnEquip()
{
    if (!bInFlight && !bDetonated) Super::OnUnEquip();
}

void ARThrowable::Launch(FVector InitialVelocity)
{
    if (bInFlight || bDetonated || InitialVelocity.ContainsNaN() || InitialVelocity.IsNearlyZero()) return;
    bInFlight = true;
    CancelWeaponTransition();
    IsEquipped = false;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Collision->IgnoreActorWhenMoving(GetInstigator(), true);
    Collision->IgnoreActorWhenMoving(GetOwner(), true);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Movement->SetUpdatedComponent(Collision);
    Movement->Activate(true);
    Movement->Velocity = InitialVelocity;
    SetLifeSpan(10.f);
    if (MotionVFX)
    {
        MotionEffect->SetAsset(MotionVFX);
        MotionEffect->Activate(true);
    }
}

void ARThrowable::DamageTarget(AActor* Target)
{
    auto* SourceASC = DamageParams.SourceAbilitySystemComponent.Get();
    AActor* Source = IsValid(SourceASC) ? SourceASC->GetAvatarActor() : nullptr;
    if (!IsValid(Source) || !IsValid(Target) || Source == Target ||
        !DamageParams.DamageGameplayEffectClass || !URAbilitySystemLibrary::IsNotFriend(Source, Target)) return;
    if (Target->Implements<URCombatInterface>() && IRCombatInterface::Execute_IsDead(Target)) return;
    auto* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    if (!TargetASC) return;
    FDamageEffectParams Params = DamageParams;
    Params.WorldContextObject = this;
    Params.TargetAbilitySystemComponent = TargetASC;
    const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    Params.DeathImpulse = Direction * Params.DeathImpulseMagnitude;
    Params.KnockbackForce = FMath::FRandRange(0.f,100.f) < Params.KnockbackChance ? Direction * Params.KnockbackMagnitude : FVector::ZeroVector;
    URAbilitySystemLibrary::ApplyDamageEffect(Params);
}

void ARThrowable::Pulse()
{
    if (Kind == ERThrowableKind::Distraction)
    {
        DistractNearbyEnemies();
        return;
    }
    if (Kind != ERThrowableKind::Molotov && Kind != ERThrowableKind::Blast) return;
    TArray<FOverlapResult> Hits;
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_Pawn);
    GetWorld()->OverlapMultiByObjectType(Hits, GetActorLocation(), FQuat::Identity, Objects, FCollisionShape::MakeSphere(Radius));
    TSet<AActor*> Seen;
    for (const FOverlapResult& Hit : Hits)
    {
        AActor* Target = Hit.GetActor();
        if (!Target || Seen.Contains(Target)) continue;
        Seen.Add(Target);
        FCollisionQueryParams Query;
        Query.AddIgnoredActor(this);
        Query.AddIgnoredActor(GetInstigator());
        FHitResult Wall;
        if (!GetWorld()->LineTraceSingleByChannel(Wall, GetActorLocation()+FVector(0,0,10), Target->GetActorLocation(), ECC_Visibility, Query) || Wall.GetActor()==Target)
            DamageTarget(Target);
    }
}

void ARThrowable::DistractNearbyEnemies()
{
    if (!bDistractionTargetsChosen)
    {
        bDistractionTargetsChosen = true;
        if (DistractionRadius <= 0.f || MaxDistractedEnemies <= 0) return;
        TArray<AREnemy*> Candidates;
        const FVector Location = GetActorLocation();
        for (TActorIterator<AREnemy> It(GetWorld()); It; ++It)
        {
            AREnemy* Enemy = *It;
            auto* AI = Cast<ARAIController>(Enemy->GetController());
            if (!AI || !AI->CanInvestigateDistraction(bDistractInCombat)
                || FVector::DistSquared(Location, Enemy->GetActorLocation()) > FMath::Square(DistractionRadius)) continue;
            bool bPerceived = bSoundDistraction;
            if (!bPerceived && bSightDistraction && AI->IsWithinSightCone(Location))
            {
                FVector Eyes; FRotator EyeRotation;
                Enemy->GetActorEyesViewPoint(Eyes, EyeRotation);
                FCollisionQueryParams Query;
                Query.AddIgnoredActor(Enemy);
                Query.AddIgnoredActor(this);
                FHitResult Wall;
                bPerceived = !GetWorld()->LineTraceSingleByChannel(Wall, Eyes, Location + FVector(0,0,10), ECC_Visibility, Query);
            }
            if (bPerceived) Candidates.Add(Enemy);
        }
        Candidates.Sort([Location](const AREnemy& A, const AREnemy& B)
        {
            const double ADistance = FVector::DistSquared(Location, A.GetActorLocation());
            const double BDistance = FVector::DistSquared(Location, B.GetActorLocation());
            return ADistance == BDistance ? A.GetUniqueID() < B.GetUniqueID() : ADistance < BDistance;
        });
        for (int32 Index = 0; Index < FMath::Min(MaxDistractedEnemies, Candidates.Num()); ++Index)
            DistractionTargets.Add(Cast<ARAIController>(Candidates[Index]->GetController()));
    }
    // Dispatch only to the chosen listeners. A global hearing event would bypass the cap.
    for (const auto& Target : DistractionTargets)
        if (Target.IsValid()) Target->InvestigateDistraction(GetActorLocation(), GetLifeSpan() > 0.f ? GetLifeSpan() : Duration, bDistractInCombat);
}

void ARThrowable::OnHit(UPrimitiveComponent* Component, AActor* Other, UPrimitiveComponent* OtherComponent, FVector Impulse, const FHitResult& Hit)
{
    if (!bInFlight || bDetonated || Other == GetInstigator()) return;
    bDetonated = true;
    MotionEffect->DeactivateImmediate();
    Movement->StopMovementImmediately();
    Movement->Deactivate();
    Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PlayImpact(GetActorLocation());
    if (Kind == ERThrowableKind::Star)
    {
        DamageTarget(Other);
        SetLifeSpan(.15f);
        return;
    }
    Duration = FMath::Max(.1f, Duration);
    if (Kind == ERThrowableKind::Blast)
    {
        Pulse(); // One radial explosion only, never a recurring fire pulse.
        UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 1.f, GetInstigator(), NoiseRange, TEXT("Explosion"));
        SetLifeSpan(Duration);
        return;
    }
    SetLifeSpan(Duration);
    if (Kind == ERThrowableKind::Smoke)
    {
        // Reuse existing sight/concealment rules without granting permanent tags.
        auto* Cover = GetWorld()->SpawnActor<ARSneakArea>(GetActorLocation(), FRotator::ZeroRotator);
        if (Cover)
        {
            Cover->bRequiresCrouch = false;
            Cover->Bounds->SetBoxExtent(FVector(Radius, Radius, Radius));
            Cover->SetLifeSpan(Duration);
        }
    }
    else
    {
        Pulse();
        GetWorldTimerManager().SetTimer(PulseTimer, this, &ARThrowable::Pulse, FMath::Max(.1f,PulseInterval), true);
    }
}

void ARThrowable::PlayImpact(FVector Location)
{
    Mesh->SetVisibility(false);
    if (ImpactVFX)
    {
        if (Kind==ERThrowableKind::Star) UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, Location);
        else UNiagaraFunctionLibrary::SpawnSystemAttached(ImpactVFX, Collision, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
    }
    if (ImpactSound) UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Location);
}
