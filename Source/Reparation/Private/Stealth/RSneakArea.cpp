#include "Stealth/RSneakArea.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

ARSneakArea::ARSneakArea()
{
	PrimaryActorTick.bCanEverTick = false;
	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("HidingBounds"));
	SetRootComponent(Bounds);
	Bounds->SetBoxExtent(FVector(500.f, 500.f, 150.f));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bounds->SetGenerateOverlapEvents(false);
	Bounds->SetCanEverAffectNavigation(false);
	Vegetation = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Vegetation"));
	Vegetation->SetupAttachment(Bounds);
	Vegetation->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Vegetation->SetCanEverAffectNavigation(false);
}

bool ARSneakArea::ContainsLocation(FVector Location) const
{
	if (!bEnabled || Location.ContainsNaN()) return false;
	const FVector Local = Bounds->GetComponentTransform().InverseTransformPosition(Location);
	const FVector Extent = Bounds->GetUnscaledBoxExtent();
	return FMath::Abs(Local.X) <= Extent.X && FMath::Abs(Local.Y) <= Extent.Y && FMath::Abs(Local.Z) <= Extent.Z;
}

void ARSneakArea::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Vegetation->ClearInstances();
	// The component also exposes a Static Mesh picker. Preserve that assignment
	// when the actor-level override is empty instead of clearing it on construction.
	if (VegetationMesh) Vegetation->SetStaticMesh(VegetationMesh);
	if (!Vegetation->GetStaticMesh()) return;
	FRandomStream Random(RandomSeed);
	const FVector Extent = Bounds->GetUnscaledBoxExtent();
	for (int32 Index = 0; Index < FMath::Clamp(InstanceCount, 0, 2000); ++Index)
	{
		const FVector Position(Random.FRandRange(-Extent.X, Extent.X), Random.FRandRange(-Extent.Y, Extent.Y), -Extent.Z);
		Vegetation->AddInstance(FTransform(FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f), Position,
			FVector(FMath::Max(0.01f, VegetationScale))), false);
	}
}
