#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Characters/RPlayer.h"
#include "AIController.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "AI/REnemy.h"
#include "GAS/RAbilitySystemComponent.h"
#include "RGameplayTags.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerPossessionSafetyTest,"Reparation.Combat.PlayerPossessionSafety",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPlayerPossessionSafetyTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game,false);
    auto* Player = World->SpawnActor<ARPlayer>();
    auto* AI = World->SpawnActor<AAIController>();
    // Matches the reported PIE stack: AI OnPossess -> player PossessedBy with no RPS.
    AI->Possess(Player);
    TestTrue(TEXT("Unexpected AI possession does not crash"),Player->GetController()==AI);
    TestNull(TEXT("AI possession does not invent player progression"),Player->GetPlayerState());
    TestNull(TEXT("AI possession does not initialize player GAS"),Player->GetAbilitySystemComponent());
    const FGameplayTag Tag = FRGameplayTags::Get().Debuff_Stun;
    ERConfirmType Result = ERConfirmType::Yes;
    URAbilitySystemLibrary::BP_DoesActorHaveTag(Player,Tag,Result);
    TestTrue(TEXT("Actual BT tag query safely rejects player without GAS"),Result==ERConfirmType::No);
    TestNull(TEXT("Null actor has no ability component"),URAbilitySystemLibrary::NativeGetRASCFromActor(nullptr));
    TestFalse(TEXT("Null actor tag query is false"),URAbilitySystemLibrary::NativeDoesActorHaveTag(nullptr,Tag));
    URAbilitySystemLibrary::AddGameplayTagToActorIfNone(Player,Tag);
    URAbilitySystemLibrary::RemoveGameplayTagFromActorIfFound(Player,Tag);
    auto* Enemy = World->SpawnActor<AREnemy>();
    auto* EnemyASC = Enemy->GetAbilitySystemComponent();
    EnemyASC->InitAbilityActorInfo(Enemy,Enemy);
    URAbilitySystemLibrary::AddGameplayTagToActorIfNone(Enemy,Tag);
    TestTrue(TEXT("Initialized enemy tag checks still work"),URAbilitySystemLibrary::NativeDoesActorHaveTag(Enemy,Tag));
    URAbilitySystemLibrary::RemoveGameplayTagFromActorIfFound(Enemy,Tag);
    TestFalse(TEXT("Initialized enemy tag removal still works"),URAbilitySystemLibrary::NativeDoesActorHaveTag(Enemy,Tag));
    AI->UnPossess();
    Player->OnRep_PlayerState();
    TestNull(TEXT("Missing replicated state safely defers GAS initialization"),Player->GetAbilitySystemComponent());
    auto* Controller = World->SpawnActor<APlayerController>();
    Player->PossessedBy(Controller);
    TestNull(TEXT("Player possession without RPS also safely defers"),Player->GetAbilitySystemComponent());
    World->DestroyWorld(false);
    return true;
}
#endif
