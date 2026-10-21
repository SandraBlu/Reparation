#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Characters/RCharacterBase.h"
#include "Stealth/RSneakArea.h"
#include "Stealth/RStealthComponent.h"
#include "Engine/World.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStealthAreaTest, "Reparation.Stealth.SneakAreas",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStealthAreaTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world"), World)) return false;
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARCharacterBase* Character = World->SpawnActor<ARCharacterBase>(ARCharacterBase::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
	ARSneakArea* Area = World->SpawnActor<ARSneakArea>();
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	TestNotNull(TEXT("Vegetation fixture mesh loads"), Cube);
	Area->Vegetation->SetStaticMesh(Cube);
	Area->OnConstruction(Area->GetActorTransform());
	TestTrue(TEXT("Component mesh assignment survives construction"), Area->Vegetation->GetStaticMesh() == Cube);
	TestEqual(TEXT("Component mesh generates requested instances"), Area->Vegetation->GetInstanceCount(), Area->InstanceCount);
	Area->Vegetation->SetStaticMesh(nullptr);
	Area->VegetationMesh = Cube;
	Area->OnConstruction(Area->GetActorTransform());
	TestTrue(TEXT("Actor mesh override is assigned"), Area->Vegetation->GetStaticMesh() == Cube);
	TestEqual(TEXT("Actor mesh generates requested instances"), Area->Vegetation->GetInstanceCount(), Area->InstanceCount);
	TestTrue(TEXT("Standing inside area registers the area"), Character->Stealth->IsInSneakArea());
	TestFalse(TEXT("Standing does not conceal by default"), Character->Stealth->IsConcealed());
	Character->bIsCrouched = true;
	TestTrue(TEXT("Crouching inside area conceals"), Character->Stealth->IsConcealed());
	TestTrue(TEXT("Distant observer cannot see unspotted crouched character"), Character->Stealth->IsConcealedFrom(FVector(1000.f, 0.f, 0.f), false));
	TestTrue(TEXT("Close observer cannot reveal concealed character"), Character->Stealth->IsConcealedFrom(FVector(100.f, 0.f, 0.f), false));
	TestTrue(TEXT("Previously spotted character becomes hidden in cover"), Character->Stealth->IsConcealedFrom(FVector(1000.f, 0.f, 0.f), true));
	FCanBeSeenFromContext Context;
	Context.ObserverLocation = FVector(1000.f, 0.f, 0.f);
	FVector Seen;
	int32 Sync = -1, Async = -1;
	float Strength = -1.f;
	TestTrue(TEXT("AI sight respects grass concealment"), Character->CanBeSeenFrom(Context, Seen, Sync, Async, Strength) == UAISense_Sight::EVisibilityResult::NotVisible);
	TestEqual(TEXT("Concealed sight performs no trace"), Sync, 0);
	ARSneakArea* OtherArea = World->SpawnActor<ARSneakArea>();
	Area->bEnabled = false;
	TestTrue(TEXT("Another overlapping area preserves concealment"), Character->Stealth->IsConcealed());
	OtherArea->Destroy();
	TestFalse(TEXT("Disabled and destroyed areas do not conceal"), Character->Stealth->IsConcealed());
	Area->bEnabled = true;
	Area->bRequiresCrouch = false;
	Character->bIsCrouched = false;
	TestTrue(TEXT("Dense cover can optionally conceal standing characters"), Character->Stealth->IsConcealed());
	Character->SetActorLocation(FVector(10000.f, 0.f, 0.f));
	TestFalse(TEXT("Leaving removes concealment"), Character->Stealth->IsConcealed());
	Area->SetActorTransform(FTransform(FRotator(0.f, 45.f, 0.f), FVector(3000.f, 0.f, 0.f), FVector(2.f, 1.f, 1.f)));
	TestTrue(TEXT("Rotated scaled area contains its transformed local point"), Area->ContainsLocation(Area->GetActorTransform().TransformPosition(FVector(400.f, 200.f, 0.f))));
	TestFalse(TEXT("Rotated scaled area excludes outside point"), Area->ContainsLocation(Area->GetActorTransform().TransformPosition(FVector(600.f, 0.f, 0.f))));
	World->DestroyWorld(false);
	return true;
}
#endif
