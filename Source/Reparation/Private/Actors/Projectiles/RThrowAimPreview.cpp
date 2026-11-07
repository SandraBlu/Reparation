#include "Actors/Projectiles/RThrowAimPreview.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

ARThrowAimPreview::ARThrowAimPreview()
{
    bReplicates = false;
    PrimaryActorTick.bCanEverTick = false;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    SegmentMesh = Cylinder.Object;
}

void ARThrowAimPreview::DrawSegment(int32 Index, FVector Start, FVector End)
{
    if (!Segments.IsValidIndex(Index))
    {
        auto* Segment = NewObject<UStaticMeshComponent>(this);
        Segment->SetupAttachment(GetRootComponent());
        Segment->SetStaticMesh(SegmentMesh);
        Segment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Segment->SetCanEverAffectNavigation(false);
        Segment->SetCastShadow(false);
        if (PreviewMaterial) Segment->SetMaterial(0, PreviewMaterial);
        Segment->RegisterComponent();
        Segments.Add(Segment);
    }
    auto* Segment = Segments[Index].Get();
    const FVector Delta = End-Start;
    Segment->SetWorldLocationAndRotation((Start+End)*.5f, FRotationMatrix::MakeFromZ(Delta).Rotator());
    Segment->SetWorldScale3D(FVector(LineRadius/50.f, LineRadius/50.f, Delta.Size()/100.f));
    Segment->SetVisibility(true);
}

void ARThrowAimPreview::UpdateTrajectory(FVector Origin, FVector Velocity, float GravityScale, float BlastRadius, float ProjectileRadius)
{
    FPredictProjectilePathParams Params;
    Params.StartLocation = Origin;
    Params.LaunchVelocity = Velocity;
    Params.ProjectileRadius = ProjectileRadius;
    Params.MaxSimTime = 10.f;
    Params.SimFrequency = 15.f;
    Params.bTraceWithCollision = true;
    Params.bTraceWithChannel = false;
    Params.ObjectTypes = {UEngineTypes::ConvertToObjectType(ECC_WorldStatic), UEngineTypes::ConvertToObjectType(ECC_WorldDynamic), UEngineTypes::ConvertToObjectType(ECC_Pawn)};
    Params.ActorsToIgnore.Add(GetOwner());
    Params.ActorsToIgnore.Add(this);
    Params.OverrideGravityZ = FMath::IsNearlyZero(GravityScale) ? -UE_SMALL_NUMBER : GetWorld()->GetGravityZ()*GravityScale;
    FPredictProjectilePathResult Result;
    bHasTarget = UGameplayStatics::PredictProjectilePath(this, Params, Result);
    int32 Used = 0;
    for (int32 Index=1; Index<Result.PathData.Num(); ++Index)
        DrawSegment(Used++, Result.PathData[Index-1].Location, Result.PathData[Index].Location);
    if (bHasTarget)
    {
        TargetLocation = Result.HitResult.ImpactPoint;
        const FVector Normal = Result.HitResult.ImpactNormal.GetSafeNormal();
        FVector X,Y;
        Normal.FindBestAxisVectors(X,Y);
        const FVector Center = TargetLocation+Normal*3.f;
        // Ring marks the actual predicted impact and its editable damage radius.
        for (int32 Index=0; Index<32; ++Index)
        {
            const float A=2.f*PI*Index/32.f, B=2.f*PI*(Index+1)/32.f;
            DrawSegment(Used++, Center+(X*FMath::Cos(A)+Y*FMath::Sin(A))*BlastRadius,
                Center+(X*FMath::Cos(B)+Y*FMath::Sin(B))*BlastRadius);
        }
        DrawSegment(Used++, Center-X*8.f, Center+X*8.f);
        DrawSegment(Used++, Center-Y*8.f, Center+Y*8.f);
    }
    for (int32 Index=Used; Index<Segments.Num(); ++Index) Segments[Index]->SetVisibility(false);
}
