#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "Components/RInventoryComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/LevelUpInfo.h"
#include "RGameplayTags.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTierRewardTest,"Reparation.Combat.TierReward",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTierRewardTest::RunTest(const FString& Parameters)
{
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    auto* Player=World->SpawnActor<ARPlayer>(); auto* PS=World->SpawnActor<ARPlayerState>(); Player->SetPlayerState(PS);
    PS->LevelUpInfo=LoadObject<ULevelUpInfo>(nullptr,TEXT("/Game/GAS/Data/DA_XPInfo")); PS->SetLevel(25); PS->AddToAbilityPts(100);
    auto* ASC=CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent()); ASC->InitAbilityActorInfo(PS,Player);
    auto* Data=DuplicateObject<UAbilityInfo>(LoadObject<UAbilityInfo>(nullptr,TEXT("/Game/GAS/Data/DA_AbilityInfo")),ASC); ASC->SkillDefinitions=Data;
    const auto& T=FRGameplayTags::Get();
    const auto Heavy=FGameplayTag::RequestGameplayTag(TEXT("ability.technique.heavyStrike"));
    const auto Counter=FGameplayTag::RequestGameplayTag(TEXT("ability.technique.counterstrike"));
    for (auto& Entry:Data->AbilityInfo) if (Entry.AbilityTag==Heavy) Entry.bSkillTreePurchasable=true;
    ASC->AddGrantedAbilities({Data->FindAbilityInfoForTag(T.ability_targetLock).Ability});
    TestEqual(TEXT("Startup cannot bypass tier reward"),ASC->GetLearnedSkillRank(T.ability_targetLock),0);
    ASC->UpdateAbilityStatus(25);
    ASC->ServerSpendAbilityPoint(T.ability_targetLock); TestEqual(TEXT("Automatic reward cannot cost points"),PS->GetAbilityPts(),100);
    ASC->ServerSpendAbilityPoint(T.ability_Melee); ASC->ServerSpendAbilityPoint(T.ability_block);
    ASC->ServerSpendAbilityPoint(Heavy); ASC->ServerSpendAbilityPoint(T.ability_evade);
    TestFalse(TEXT("Missing planned Parry cannot reduce a full tier to two skills"),ASC->IsTierRewardEarned());
    // Simulate the third tier skill becoming implemented, without authoring a fake production Parry.
    for (auto& E:Data->AbilityInfo) if (E.AbilityTag==Counter) E.BranchTier=2;
    ASC->ServerSpendAbilityPoint(Counter);
    TestTrue(TEXT("Three learned tier skills earn reward"),ASC->IsTierRewardEarned());
    TestEqual(TEXT("Reward automatically learned"),ASC->GetLearnedSkillRank(T.ability_targetLock),1);
    TestEqual(TEXT("Reward uses original fixed control"),ASC->GetInputTagFromAbilityTag(T.ability_targetLock),T.InputTag_Toggle_TargetLock);
    const int32 Points=PS->GetAbilityPts(); const int32 Refund=ASC->GetBranchRefund(ERSkillDiscipline::Melee);
    ASC->RefreshTierReward(); TestEqual(TEXT("Repeated evaluation grants no extra rank"),ASC->GetLearnedSkillRank(T.ability_targetLock),1);
    TestEqual(TEXT("Reward never changes point balance"),PS->GetAbilityPts(),Points);
    Data->bRewardRequiresAllBranches=true; ASC->RefreshTierReward();
    TestEqual(TEXT("All-branches option does not accept one branch"),ASC->GetLearnedSkillRank(T.ability_targetLock),0);
    Data->bRewardRequiresAllBranches=false; ASC->RefreshTierReward();
    PS->PlayerInventory->SetCurrency(500); ASC->ServerRespecBranch(ERSkillDiscipline::Melee);
    TestEqual(TEXT("Reset removes reward when full tier is lost"),ASC->GetLearnedSkillRank(T.ability_targetLock),0);
    TestEqual(TEXT("Reset refunds purchases only, not free reward"),PS->GetAbilityPts(),Points+Refund);
    const auto Cloak=FGameplayTag::RequestGameplayTag(TEXT("ability.passive.protectiveCloak"));
    ASC->AddPassiveAbilities({Data->FindAbilityInfoForTag(Cloak).Ability});
    TestNull(TEXT("Gear-only cloak cannot occupy a starting passive slot"),ASC->GetSpecFromAbilityTag(Cloak));
    ASC->ServerSpendAbilityPoint(Cloak); TestEqual(TEXT("Gear cloak cannot be bought through hidden RPC path"),ASC->GetLearnedSkillRank(Cloak),0);
    World->DestroyWorld(false); return true;
}
#endif
