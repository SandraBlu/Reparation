#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "AI/REnemy.h"
#include "AI/RCompanion.h"
#include "Components/Combat/REnemyMeleeSweepComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyMeleeSweepTest, "Reparation.Combat.MaceHeadSweep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnemyMeleeSweepTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world"), World)) return false;
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AREnemy* Enemy = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), FVector(-2000.f, 0.f, 0.f), FRotator::ZeroRotator, Spawn);
	ARCompanion* Target = World->SpawnActor<ARCompanion>(ARCompanion::StaticClass(), FVector(0.f, 0.f, 88.f), FRotator::ZeroRotator, Spawn);
	USkeletalMeshComponent* Weapon = nullptr;
	TArray<USkeletalMeshComponent*> Components;
	Enemy->GetComponents(Components);
	for (USkeletalMeshComponent* Component : Components) if (Component->GetFName() == TEXT("CombatRH")) Weapon = Component;
	USkeletalMesh* Mace = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/ResourcePack/Weapons/Mace/Skeletal_Mesh/SK_Mace.SK_Mace"));
	if (!TestNotNull(TEXT("Mace mesh"), Mace) || !TestNotNull(TEXT("Weapon component"), Weapon)) { World->DestroyWorld(false); return false; }
	Weapon->SetSkeletalMesh(Mace);
	FindFProperty<FNameProperty>(AREnemy::StaticClass(), TEXT("WeaponDamageSocket"))->SetPropertyValue_InContainer(Enemy, TEXT("attack"));
	Enemy->Tags.AddUnique(TEXT("enemy"));
	Target->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Target->GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	Target->GetCapsuleComponent()->SetCapsuleSize(34.f, 88.f);
	UREnemyMeleeSweepComponent* Sweep = Enemy->MeleeSweep;
	Sweep->bUseHeadSweep = true;
	Sweep->HeadRadius = 15.f;
	const FGameplayTag Event = FGameplayTag::RequestGameplayTag(TEXT("montage.attack01"));
	const auto PutHead = [&](const FVector& Location)
	{
		Weapon->SetWorldLocation(Weapon->GetComponentLocation() + Location - Weapon->GetSocketLocation(TEXT("attack")));
	};
	TArray<AActor*> Hits;
	PutHead(FVector(-200.f, 0.f, 88.f));
	Sweep->BeginWindow(Event);
	PutHead(FVector(200.f, 0.f, 88.f));
	Sweep->SampleWindow();
	Sweep->ConsumeTargets(Hits);
	TestTrue(TEXT("Head crossing between frames hits even when final tip is beyond target"), Hits.Contains(Target));
	PutHead(FVector(-200.f, 0.f, 88.f));
	Sweep->SampleWindow();
	Sweep->ConsumeTargets(Hits);
	TestTrue(TEXT("One hit per target per strike window"), Hits.IsEmpty());
	Sweep->EndWindow();
	Target->GetCapsuleComponent()->SetCapsuleHalfHeight(40.f);
	Target->SetActorLocation(FVector(0.f, 0.f, 40.f));
	PutHead(FVector(-200.f, 0.f, 160.f));
	Sweep->BeginWindow(Event);
	PutHead(FVector(200.f, 0.f, 160.f));
	Sweep->SampleWindow();
	Sweep->ConsumeTargets(Hits);
	TestTrue(TEXT("High swing above crouched capsule remains duckable"), Hits.IsEmpty());
	Sweep->EndWindow();
	PutHead(FVector(-200.f, 0.f, 40.f));
	Sweep->BeginWindow(Event);
	PutHead(FVector(200.f, 0.f, 40.f));
	Sweep->SampleWindow();
	URAbilitySystemLibrary::GetEnemyMeleeTargets(Enemy, Hits, {}, 50.f, FVector(200.f, 0.f, 40.f));
	TestTrue(TEXT("Low head sweep hits crouched target through existing ability query"), Hits.Contains(Target));
	Sweep->EndWindow();
	PutHead(FVector(-200.f, 0.f, 40.f));
	PutHead(FVector(200.f, 0.f, 40.f));
	Sweep->SampleWindow();
	Sweep->ConsumeTargets(Hits);
	TestTrue(TEXT("Closed damage window cannot hit"), Hits.IsEmpty());
	World->DestroyWorld(false);
	return true;
}
#endif
