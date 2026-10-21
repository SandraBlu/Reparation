#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/RCoordinationTestAbility.h"
#include "AI/RCombatCoordinator.h"
#include "AI/REnemy.h"
#include "AI/RCompanion.h"
#include "Characters/RPlayer.h"
#include "AbilitySystemComponent.h"
#include "GAS/Data/LevelUpInfo.h"
#include "GAS/Calcs/GEC_Damage.h"
#include "Engine/World.h"
#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatFoundationTest, "Reparation.Combat.DifficultyAndProgression",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatFoundationTest::RunTest(const FString& Parameters)
{
    ULevelUpInfo* XP = NewObject<ULevelUpInfo>();
    TestEqual(TEXT("Empty progression is safe"), XP->FindLevelForXP(MAX_int32), 1);
    TestEqual(TEXT("Empty progression bar is safe"), XP->GetProgressForXP(0), 0.f);
    XP->LevelUpInfo.SetNum(4);
    XP->LevelUpInfo[1].LevelUpRequirement = 100;
    XP->LevelUpInfo[2].LevelUpRequirement = 300;
    XP->LevelUpInfo[3].LevelUpRequirement = 600;
    TestEqual(TEXT("Below first threshold stays level one"), XP->FindLevelForXP(99), 1);
    TestEqual(TEXT("Exact threshold advances level"), XP->FindLevelForXP(100), 2);
    TestEqual(TEXT("Large XP remains at data-defined cap"), XP->FindLevelForXP(MAX_int32), 3);
    TestEqual(TEXT("Mid-level progress"), XP->GetProgressForXP(200), 0.5f);
    TestEqual(TEXT("Cap has a full bar without indexing past the table"), XP->GetProgressForXP(300), 1.f);
    TestEqual(TEXT("No reward for leveling beyond cap"), XP->GetAbilityReward(3), 0);
    TestEqual(TEXT("Invalid reward lookup is safe"), XP->GetAttributeReward(-1), 0);
    TestEqual(TEXT("Excess armor cannot turn damage into healing"), UGEC_Damage::CalculateArmorMultiplier(1000.f, 0.f, 1.f, 1.f), 0.f);
    TestEqual(TEXT("Excess penetration cannot amplify damage"), UGEC_Damage::CalculateArmorMultiplier(100.f, 1000.f, 1.f, 1.f), 1.f);
    TestEqual(TEXT("Ordinary armor retains its reduction"), UGEC_Damage::CalculateArmorMultiplier(50.f, 0.f, 1.f, 1.f), 0.5f);
    for (int32 I=0; I<200; ++I)
    {
        TestTrue(TEXT("100 percent chance always succeeds"), UGEC_Damage::RollChance(100.f));
        TestFalse(TEXT("Zero percent chance never succeeds"), UGEC_Damage::RollChance(0.f));
    }
    const URCombatBalanceSettings* Settings = GetDefault<URCombatBalanceSettings>();
    TestEqual(TEXT("Easy attacker limit"), Settings->Easy.GetPlayerAttackerLimit(1), 1);
    TestEqual(TEXT("Normal attacker limit"), Settings->Normal.GetPlayerAttackerLimit(1), 2);
    TestEqual(TEXT("Hard attacker limit"), Settings->Hard.GetPlayerAttackerLimit(1), 3);
    TestTrue(TEXT("Very Hard has an explicit difficulty profile"), Settings->GetProfile(ERGameDifficulty::VeryHard).EnemyDamageMultiplier > Settings->Hard.EnemyDamageMultiplier);
    FRCombatDifficultyProfile Profile = Settings->Normal;
    FRCombatLevelBand Band; Band.MinimumPlayerLevel = 10; Band.PlayerAttackers = 3; Profile.PlayerLevelBands.Add(Band);
    TestEqual(TEXT("Level gate does not apply early"), Profile.GetPlayerAttackerLimit(9), 2);
    TestEqual(TEXT("Level gate applies at exact threshold"), Profile.GetPlayerAttackerLimit(10), 3);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCoordinationTest, "Reparation.Combat.AttackCoordination",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FEnemyCoordinationTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world"), World)) return false;
    URCombatCoordinator* Coordinator = World->GetSubsystem<URCombatCoordinator>();
    if (!TestNotNull(TEXT("Combat coordinator"), Coordinator)) { World->DestroyWorld(false); return false; }
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ARPlayer* Player = World->SpawnActor<ARPlayer>(ARPlayer::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
    ARCompanion* Companion = World->SpawnActor<ARCompanion>(ARCompanion::StaticClass(), FVector(100.f,0.f,0.f), FRotator::ZeroRotator, Spawn);
    TArray<AREnemy*> Enemies;
    TArray<FGameplayAbilitySpecHandle> Handles;
    for (int32 I=0; I<3; ++I)
    {
        AREnemy* Enemy = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), FVector(300.f+I*100.f,0.f,0.f), FRotator::ZeroRotator, Spawn);
        Enemy->GetAbilitySystemComponent()->InitAbilityActorInfo(Enemy, Enemy);
        Handles.Add(Enemy->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(URCoordinationTestAbility::StaticClass())));
        Enemy->SetCombatTarget_Implementation(Player);
        Enemies.Add(Enemy);
    }
    TestTrue(TEXT("First real GAS attack acquires permission"), Enemies[0]->GetAbilitySystemComponent()->TryActivateAbility(Handles[0]));
    TestTrue(TEXT("Second attack fits Normal pressure limit"), Enemies[1]->GetAbilitySystemComponent()->TryActivateAbility(Handles[1]));
    TestFalse(TEXT("Third simultaneous player attack is denied"), Enemies[2]->GetAbilitySystemComponent()->TryActivateAbility(Handles[2]));
    TestEqual(TEXT("Player has exactly two committed attackers"), Coordinator->GetAttackerCount(Player), 2);
    Enemies[2]->SetCombatTarget_Implementation(Companion);
    TestTrue(TEXT("Enemy can attack companion while player's budget is full"), Enemies[2]->GetAbilitySystemComponent()->TryActivateAbility(Handles[2]));
    Enemies[0]->GetAbilitySystemComponent()->CancelAbilityHandle(Handles[0]);
    TestEqual(TEXT("Canceled ability releases its permission"), Coordinator->GetAttackerCount(Player), 1);
    Enemies[2]->GetAbilitySystemComponent()->CancelAbilityHandle(Handles[2]);
    Enemies[2]->SetCombatTarget_Implementation(Player);
    TestTrue(TEXT("Freed player opening can be reassigned"), Enemies[2]->GetAbilitySystemComponent()->TryActivateAbility(Handles[2]));
    for (int32 I=0; I<3; ++I) Enemies[I]->GetAbilitySystemComponent()->CancelAbilityHandle(Handles[I]);
    TestEqual(TEXT("All attack permissions cleaned up"), Coordinator->GetAttackerCount(Player), 0);
    World->DestroyWorld(false);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityPointSafetyTest, "Reparation.Combat.AbilityPointSafety",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAbilityPointSafetyTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world"), World)) return false;
    ARPlayer* Player = World->SpawnActor<ARPlayer>();
    ARPlayerState* PS = World->SpawnActor<ARPlayerState>();
    Player->SetPlayerState(PS);
    PS->LevelUpInfo = NewObject<ULevelUpInfo>();
    PS->LevelUpInfo->LevelUpInfo.SetNum(4);
    PS->SetLevel(1);
    URAbilitySystemComponent* ASC = CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent());
    ASC->InitAbilityActorInfo(PS, Player);
    const FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(TEXT("ability.attack.melee"));
    const FGameplayTag Status = FGameplayTag::RequestGameplayTag(TEXT("ability.status.unlocked"));
    FGameplayAbilitySpec Spec(URCoordinationTestAbility::StaticClass(), 1);
    Spec.GetDynamicSpecSourceTags().AddTag(Status);
    const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
    ASC->ServerSpendAbilityPoint(AbilityTag);
    TestEqual(TEXT("No free ability upgrade with zero points"), ASC->FindAbilitySpecFromHandle(Handle)->Level, 1);
    TestEqual(TEXT("Rejected upgrade cannot make points negative"), PS->GetAbilityPts(), 0);
    PS->AddToAbilityPts(3);
    ASC->ServerSpendAbilityPoint(AbilityTag);
    TestEqual(TEXT("Paid upgrade advances exactly one rank"), ASC->FindAbilitySpecFromHandle(Handle)->Level, 2);
    TestEqual(TEXT("Paid upgrade consumes exactly one point"), PS->GetAbilityPts(), 2);
    ASC->ServerSpendAbilityPoint(AbilityTag);
    ASC->ServerSpendAbilityPoint(AbilityTag);
    TestEqual(TEXT("Ability ranks obey progression cap"), ASC->FindAbilitySpecFromHandle(Handle)->Level, 3);
    TestEqual(TEXT("Cap rejection does not spend a point"), PS->GetAbilityPts(), 1);
    const FGameplayTag Attribute = FGameplayTag::RequestGameplayTag(TEXT("attributes.primary.strength"));
    ASC->ServerUpgradeAttribute(Attribute);
    TestEqual(TEXT("No free attribute upgrade with zero points"), PS->GetAttributePts(), 0);
    PS->AddToAttributePts(1);
    ASC->ServerUpgradeAttribute(AbilityTag);
    TestEqual(TEXT("Invalid attribute tag does not spend a point"), PS->GetAttributePts(), 1);
    PS->SetLevel(MAX_int32);
    TestEqual(TEXT("Player level is clamped to progression data"), PS->GetPlayerLevel(), 3);
    World->DestroyWorld(false);
    return true;
}
#endif
