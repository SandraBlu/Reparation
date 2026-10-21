#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/AbilityInfo.h"
#include "GAS/Data/LevelUpInfo.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillTreeProgressionTest, "Reparation.Combat.SkillTreeProgression", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSkillTreeProgressionTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    ARPlayer* Player = World->SpawnActor<ARPlayer>();
    ARPlayerState* PS = World->SpawnActor<ARPlayerState>(); Player->SetPlayerState(PS);
    PS->LevelUpInfo = LoadObject<ULevelUpInfo>(nullptr, TEXT("/Game/GAS/Data/DA_XPInfo.DA_XPInfo"));
    PS->SetLevel(25); PS->AddToAbilityPts(100);
    URAbilitySystemComponent* ASC = CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent()); ASC->InitAbilityActorInfo(PS, Player);
    ASC->SkillDefinitions = LoadObject<UAbilityInfo>(nullptr, TEXT("/Game/GAS/Data/DA_AbilityInfo.DA_AbilityInfo"));
    ASC->UpdateAbilityStatus(25);
    const FGameplayTag Katana = FGameplayTag::RequestGameplayTag(TEXT("ability.melee"));
    const FGameplayTag Grenade = FGameplayTag::RequestGameplayTag(TEXT("ability.grenade.stun"));
    const FGameplayTag Poison = FGameplayTag::RequestGameplayTag(TEXT("ability.mastery.poison"));
    const FGameplayTag Advanced = FGameplayTag::RequestGameplayTag(TEXT("ability.technique.advancedProjectiles"));
    const FGameplayTag Dark = FGameplayTag::RequestGameplayTag(TEXT("ability.mastery.dark"));
    TestFalse(TEXT("Free available specs do not count as investment"), ASC->CoreSkillPointsSpent > 0);
    ASC->ServerSpendAbilityPoint(Poison);
    TestEqual(TEXT("Poison investment gate rejects spending"), PS->GetAbilityPts(), 100);
    // Five purchases in Ranged alone must reach Poison without a katana prerequisite.
    for (int32 Rank = 0; Rank < 5; ++Rank) ASC->ServerSpendAbilityPoint(Grenade);
    TestEqual(TEXT("Ranged purchases count actual point costs"), ASC->CoreSkillPointsSpent, 5);
    TestEqual(TEXT("No melee ranks required"), ASC->GetLearnedSkillRank(Katana), 0);
    TestTrue(TEXT("Any lower branch can reach Poison"), ASC->GetSkillUpgradeIssue(Poison).IsEmpty());
    ASC->ServerSpendAbilityPoint(Poison);
    TestEqual(TEXT("Poison learned without a quest"), ASC->GetLearnedSkillRank(Poison), 1);
    TestEqual(TEXT("Shared purchases do not feed their own investment gate"), ASC->CoreSkillPointsSpent, 5);
    ASC->ServerSpendAbilityPoint(Grenade);
    TestEqual(TEXT("Rejected capped purchase cannot inflate investment"), ASC->CoreSkillPointsSpent, 5);
    ASC->CoreSkillPointsSpent = 10;
    TestTrue(TEXT("Level independently makes sci-fi quest eligible"), ASC->IsSkillQuestEligible(TEXT("AdvancedProjectiles")));
    TestFalse(TEXT("Quest eligibility does not grant knowledge"), ASC->GetSkillUpgradeIssue(Advanced).IsEmpty());
    PS->SetLevel(1);
    TestTrue(TEXT("Investment independently makes sci-fi quest eligible"), ASC->IsSkillQuestEligible(TEXT("AdvancedProjectiles")));
    ASC->CoreSkillPointsSpent = 9;
    TestFalse(TEXT("Neither condition means quest is not eligible"), ASC->IsSkillQuestEligible(TEXT("AdvancedProjectiles")));
    ASC->CoreSkillPointsSpent = 10;
    ASC->GrantProgressionUnlock(TEXT("Unknown")); TestEqual(TEXT("Unknown completion keys are ignored"), ASC->ProgressionUnlocks.Num(), 0);
    ASC->GrantProgressionUnlock(TEXT("AdvancedProjectiles")); ASC->GrantProgressionUnlock(TEXT("AdvancedProjectiles"));
    TestEqual(TEXT("Completion unlock is idempotent"), ASC->ProgressionUnlocks.Num(), 1);
    ASC->ServerSpendAbilityPoint(Advanced);
    TestEqual(TEXT("Completed teaching quest permits learning knowledge"), ASC->GetLearnedSkillRank(Advanced), 1);
    ASC->CoreSkillPointsSpent = 20; PS->SetLevel(25);
    TestFalse(TEXT("Dark requires its separate quest"), ASC->GetSkillUpgradeIssue(Dark).IsEmpty());
    ASC->GrantProgressionUnlock(TEXT("Dark"));
    TestTrue(TEXT("Dark permits purchase after independent completion"), ASC->GetSkillUpgradeIssue(Dark).IsEmpty());
    World->DestroyWorld(false); return true;
}
#endif
