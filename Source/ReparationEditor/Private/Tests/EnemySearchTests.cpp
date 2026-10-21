#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "AI/RAIController.h"
#include "Characters/RCharacterBase.h"
#include "Stealth/RSneakArea.h"
#include "Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "AI/RCompanion.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemySearchTest, "Reparation.Stealth.EnemySearch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnemySearchTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world"), World)) return false;
	if (!TestNotNull(TEXT("Test world AI system"), World->CreateAISystem())) { World->DestroyWorld(false); return false; }
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARCharacterBase* Target = World->SpawnActor<ARCharacterBase>(ARCharacterBase::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
	ARAIController* AI = World->SpawnActor<ARAIController>();
	ARCharacterBase* Observer = World->SpawnActor<ARCharacterBase>(ARCharacterBase::StaticClass(), FVector(-1000.f, 0.f, 0.f), FRotator::ZeroRotator, Spawn);
	AI->Possess(Observer);
	UBlackboardData* Data = NewObject<UBlackboardData>(AI);
	FBlackboardEntry Entry;
	Entry.EntryName = TEXT("TargetActor");
	Entry.KeyType = NewObject<UBlackboardKeyType_Object>(Data);
	Data->Keys.Add(Entry);
	if (!TestTrue(TEXT("Test Blackboard initializes"), AI->GetBlackboardComponent()->InitializeBlackboard(*Data))) { World->DestroyWorld(false); return false; }
	const FVector SeenLocation(200.f, 100.f, 0.f);
	AI->HandleSightUpdate(Target, true, SeenLocation);
	TestTrue(TEXT("Visible target acquired"), AI->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")) == Target);
	TestFalse(TEXT("Visible target is not searching"), AI->bSearching);
	AI->HandleSightUpdate(Target, false, SeenLocation);
	TestTrue(TEXT("Sight loss starts searching"), AI->bSearching);
	TestNull(TEXT("Sight loss clears live target"), AI->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	TestTrue(TEXT("Search retains last seen snapshot"), AI->LastKnownLocation.Equals(SeenLocation));
	ARSneakArea* Area = World->SpawnActor<ARSneakArea>();
	Target->bIsCrouched = true;
	Target->SetActorLocation(FVector(300.f, 0.f, 0.f));
	AI->HandleSightUpdate(Target, true, Target->GetActorLocation());
	TestTrue(TEXT("Concealed target cannot be reacquired by sight"), AI->bSearching);
	TestTrue(TEXT("Hidden movement never updates search snapshot"), AI->LastKnownLocation.Equals(SeenLocation));
	Area->bEnabled = false;
	AI->HandleSightUpdate(Target, true, Target->GetActorLocation());
	TestFalse(TEXT("Genuine sight reacquisition ends search"), AI->bSearching);
	TestTrue(TEXT("Reacquisition updates last seen position"), AI->LastKnownLocation.Equals(Target->GetActorLocation()));
	const FVector Origin = Observer->GetActorLocation();
	TestTrue(TEXT("Forward location is inside sight cone"), AI->IsWithinSightCone(Origin + FVector(100.f, 0.f, 0.f)));
	TestFalse(TEXT("Side location is outside sight cone"), AI->IsWithinSightCone(Origin + FVector(0.f, 100.f, 0.f)));
	TestFalse(TEXT("Rear location is outside sight cone"), AI->IsWithinSightCone(Origin + FVector(-100.f, 0.f, 0.f)));
	Observer->SetActorRotation(FRotator(0.f, 90.f, 0.f));
	TestTrue(TEXT("Sight cone follows pawn facing"), AI->IsWithinSightCone(Origin + FVector(0.f, 100.f, 0.f)));
	TestFalse(TEXT("Previous forward direction is excluded after turning"), AI->IsWithinSightCone(Origin + FVector(100.f, 0.f, 0.f)));
	AI->HandleSightUpdate(Target, false, AI->LastKnownLocation);
	const FVector SearchSnapshot = AI->LastKnownLocation;
	const float PreviousYaw = Observer->GetActorRotation().Yaw;
	AI->Tick(0.1f);
	TestTrue(TEXT("Searching enemy scans while standing at the last-known area"), !FMath::IsNearlyEqual(PreviousYaw, Observer->GetActorRotation().Yaw));
	TestTrue(TEXT("Scanning preserves the last-seen snapshot"), AI->LastKnownLocation.Equals(SearchSnapshot));
	TestTrue(TEXT("Scanning does not silently abandon the search"), AI->bSearching);
	// The tree owns search ticks only when its service has enabled the branch.
	UBlackboardData* TreeData = NewObject<UBlackboardData>(AI);
	TreeData->Keys.Add(Entry);
	FBlackboardEntry SearchEntry;
	SearchEntry.EntryName = TEXT("SearchActive");
	SearchEntry.KeyType = NewObject<UBlackboardKeyType_Bool>(TreeData);
	TreeData->Keys.Add(SearchEntry);
	FBlackboardEntry LocationEntry;
	LocationEntry.EntryName = TEXT("LastKnownLocation");
	LocationEntry.KeyType = NewObject<UBlackboardKeyType_Vector>(TreeData);
	TreeData->Keys.Add(LocationEntry);
	TestTrue(TEXT("Tree-search blackboard initializes"), AI->GetBlackboardComponent()->InitializeBlackboard(*TreeData));
	TestFalse(TEXT("Shared blackboard keys alone do not change unmigrated trees"), AI->UsesBehaviorTreeSearch());
	AI->EnableBehaviorTreeSearch();
	TestTrue(TEXT("Perception service enables tree-owned search"), AI->UsesBehaviorTreeSearch());
	Observer->SetActorRotation(FRotator::ZeroRotator);
	AI->HandleSightUpdate(Target, true, Target->GetActorLocation());
	AI->HandleSightUpdate(Target, false, AI->LastKnownLocation);
	TestTrue(TEXT("Sight loss selects the search branch"), AI->GetBlackboardComponent()->GetValueAsBool(TEXT("SearchActive")));
	TestTrue(TEXT("Tree receives the frozen last-seen location"), AI->GetBlackboardComponent()->GetValueAsVector(TEXT("LastKnownLocation")).Equals(AI->LastKnownLocation));
	const double TreeYaw = Observer->GetActorRotation().Yaw;
	AI->Tick(0.1f);
	TestEqual(TEXT("Controller does not duplicate tree search movement"), Observer->GetActorRotation().Yaw, TreeYaw);
	AI->StartSearchMovement();
	AI->TickSearchMovement(0.1f);
	TestFalse(TEXT("Tree task can drive search scanning"), FMath::IsNearlyEqual(Observer->GetActorRotation().Yaw, TreeYaw));
	Observer->SetActorRotation(FRotator::ZeroRotator);
	AI->HandleSightUpdate(Target, true, Target->GetActorLocation());
	TestFalse(TEXT("Reacquisition releases the search branch"), AI->GetBlackboardComponent()->GetValueAsBool(TEXT("SearchActive")));
	TestTrue(TEXT("Combat target restored on reacquisition"), AI->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")) == Target);
	ARCompanion* Attacker = World->SpawnActor<ARCompanion>(ARCompanion::StaticClass(), FVector(-500.f, 100.f, 0.f), FRotator::ZeroRotator, Spawn);
	AI->NotifyDamagedBy(Attacker, 25.f);
	AI->Tick(3.1f);
	TestTrue(TEXT("Enemy retaliates against a damaging companion"), AI->GetSightTarget() == Attacker);
	const FVector RetaliationSnapshot = AI->LastKnownLocation;
	Area->SetActorLocation(Attacker->GetActorLocation());
	Area->bEnabled = true;
	Attacker->bIsCrouched = true;
	AI->NotifyDamagedBy(Attacker, 25.f);
	AI->Tick(0.1f);
	TestTrue(TEXT("Retaliation does not bypass concealment"), AI->bSearching);
	TestTrue(TEXT("Hidden damage source does not update last-seen snapshot"), AI->LastKnownLocation.Equals(RetaliationSnapshot));
	World->DestroyWorld(false);
	return true;
}
#endif
