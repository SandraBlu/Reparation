#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Characters/RPlayer.h"
#include "AI/RCompanion.h"
#include "AbilitySystemComponent.h"
#include "RGameplayTags.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyBlockingTest, "Reparation.Combat.DefenderBlocking",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnemyBlockingTest::RunTest(const FString& Parameters)
{
    TestFalse(TEXT("Absent defender safely reports no block"), URAbilitySystemLibrary::IsActorBlocking(nullptr));
    TestFalse(TEXT("Absent attacker cannot form a valid block"), URAbilitySystemLibrary::IsValidBlock(nullptr, nullptr));
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world"), World)) return false;
    ARPlayer* Player = World->SpawnActor<ARPlayer>();
    Player->SetIsUsingBlock(false);
    TestFalse(TEXT("Player without block flag reports no block"), URAbilitySystemLibrary::IsActorBlocking(Player));
    Player->SetIsUsingBlock(true);
    TestTrue(TEXT("Existing player block flag remains supported"), URAbilitySystemLibrary::IsActorBlocking(Player));
    ARCompanion* Companion = World->SpawnActor<ARCompanion>();
    TestFalse(TEXT("Non-player defender does not require a player cast"), URAbilitySystemLibrary::IsActorBlocking(Companion));
    Companion->GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("status.blocking")));
    TestTrue(TEXT("Companion blocking tag is supported"), URAbilitySystemLibrary::IsActorBlocking(Companion));
    Companion->GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("status.blocking")));
    TestFalse(TEXT("Removing blocking tag ends block"), URAbilitySystemLibrary::IsActorBlocking(Companion));
    Companion->Destroy();
    TestFalse(TEXT("Destroyed defender safely reports no block"), URAbilitySystemLibrary::IsActorBlocking(Companion));
    World->DestroyWorld(false);
    return true;
}
#endif

