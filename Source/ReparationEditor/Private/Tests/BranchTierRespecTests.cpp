#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "Components/RInventoryComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/LevelUpInfo.h"
#include "RGameplayTags.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBranchTierRespecTest, "Reparation.Combat.BranchTierRespec", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBranchTierRespecTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    ARPlayer* Player = World->SpawnActor<ARPlayer>();
    ARPlayerState* PS = World->SpawnActor<ARPlayerState>(); Player->SetPlayerState(PS);
    PS->LevelUpInfo = LoadObject<ULevelUpInfo>(nullptr, TEXT("/Game/GAS/Data/DA_XPInfo.DA_XPInfo"));
    PS->SetLevel(25); PS->AddToAbilityPts(100);
    auto* ASC = CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent()); ASC->InitAbilityActorInfo(PS, Player);
    auto* Data = DuplicateObject<UAbilityInfo>(LoadObject<UAbilityInfo>(nullptr, TEXT("/Game/GAS/Data/DA_AbilityInfo.DA_AbilityInfo")), ASC);
    ASC->SkillDefinitions = Data;
    const FGameplayTag Stamina = FGameplayTag::RequestGameplayTag(TEXT("ability.passive.staminaSiphon"));
    const FGameplayTag Grenade = FGameplayTag::RequestGameplayTag(TEXT("ability.grenade.stun"));
    const FGameplayTag Katana = FGameplayTag::RequestGameplayTag(TEXT("ability.melee"));
    for (FRAbilityInfo& E : Data->AbilityInfo)
    {
        if (E.AbilityTag == Stamina) { E.Discipline = ERSkillDiscipline::Stealth; E.BranchTier = 1; E.SkillPointCost = 2; E.bCountsTowardsCoreInvestment = true; }
        if (E.AbilityTag == Grenade) { E.Discipline = ERSkillDiscipline::Stealth; E.BranchTier = 2; E.SkillPointCost = 3; E.bCountsTowardsCoreInvestment = true; }
    }
    ASC->UpdateAbilityStatus(25);
    ASC->ServerSpendAbilityPoint(Grenade);
    TestEqual(TEXT("Server rejects skipping lower tier"), PS->GetAbilityPts(),100);
    ASC->ServerSpendAbilityPoint(Stamina);
    TestEqual(TEXT("Basics cost two authored points"), PS->GetAbilityPts(),98);
    TestTrue(TEXT("A learned lower-tier skill opens next tier"), ASC->IsBranchTierUnlocked(ERSkillDiscipline::Stealth,2));
    TestFalse(TEXT("Another branch cannot open it"), ASC->IsBranchTierUnlocked(ERSkillDiscipline::Ranged,2));
    TestFalse(TEXT("Unlearned second-tier skills do not open third tier"), ASC->IsBranchTierUnlocked(ERSkillDiscipline::Stealth,3));
    TestFalse(TEXT("Editable full-tier requirement counts distinct skills, not ranks"), ASC->IsBranchTierUnlocked(ERSkillDiscipline::Stealth,2,2));
    ASC->ServerSpendAbilityPoint(Grenade);
    TestEqual(TEXT("Technique costs three authored points"),PS->GetAbilityPts(),95);
    TestEqual(TEXT("Refund tracks exact payments"),ASC->GetBranchRefund(ERSkillDiscipline::Stealth),5);
    TestEqual(TEXT("Investment tracks variable costs"),ASC->CoreSkillPointsSpent,5);
    Data->RespecPayment = ERRespecPayment::Gold; Data->RespecCost = 100;
    PS->PlayerInventory->SetCurrency(0);
    ASC->ServerRespecBranch(ERSkillDiscipline::Stealth);
    TestEqual(TEXT("Insufficient gold leaves ranks and ledger intact"), ASC->GetBranchRefund(ERSkillDiscipline::Stealth),5);
    PS->PlayerInventory->SetCurrency(150);
    ASC->ServerEquipAbility(Stamina, FRGameplayTags::Get().InputTag_Passive01);
    TestTrue(TEXT("Passive runs before branch reset"),ASC->GetSpecFromAbilityTag(Stamina)->IsActive());
    // Changing balance after purchase must not inflate refunds.
    for (FRAbilityInfo& E : Data->AbilityInfo) if (E.AbilityTag == Stamina) E.SkillPointCost = 5;
    ASC->ServerRespecBranch(ERSkillDiscipline::Stealth);
    TestEqual(TEXT("Gold fee charged once"),PS->PlayerInventory->GetCurrency(),50);
    TestEqual(TEXT("Paid points refunded using recorded costs"),PS->GetAbilityPts(),100);
    TestEqual(TEXT("Reset clears paid learned status"),ASC->GetLearnedSkillRank(Stamina),0);
    TestFalse(TEXT("Reset stops the equipped passive"),ASC->GetSpecFromAbilityTag(Stamina)->IsActive());
    TestFalse(TEXT("Reset removes its loadout assignment"),ASC->GetInputTagFromAbilityTag(Stamina).IsValid());
    TestEqual(TEXT("Reset removes corresponding investment"),ASC->CoreSkillPointsSpent,0);
    TestFalse(TEXT("Reset closes next tier"),ASC->IsBranchTierUnlocked(ERSkillDiscipline::Stealth,2));
    ASC->ServerRespecBranch(ERSkillDiscipline::Stealth);
    TestEqual(TEXT("Repeated reset cannot refund twice"),PS->GetAbilityPts(),100);
    TestEqual(TEXT("Repeated reset cannot charge twice"),PS->PlayerInventory->GetCurrency(),50);
    // A freely learned starting rank is preserved; only its paid upgrade is refunded.
    auto* Spec = ASC->GetSpecFromAbilityTag(Stamina);
    Spec->GetDynamicSpecSourceTags().RemoveTag(FRGameplayTags::Get().ability_status_available);
    Spec->GetDynamicSpecSourceTags().AddTag(FRGameplayTags::Get().ability_status_unlocked);
    ASC->ServerSpendAbilityPoint(Stamina);
    Data->RespecPayment = ERRespecPayment::SkillPoints; Data->RespecCost = 1;
    ASC->ServerRespecBranch(ERSkillDiscipline::Stealth);
    TestEqual(TEXT("Free starting rank survives respec"),ASC->GetLearnedSkillRank(Stamina),1);
    TestEqual(TEXT("Skill-point fee deducted from refund"),PS->GetAbilityPts(),99);
    Data->RespecPayment = ERRespecPayment::QuestItem;
    ASC->ServerSpendAbilityPoint(Stamina);
    TestFalse(TEXT("No quest item configured cannot reset"),ASC->GetBranchRespecIssue(ERSkillDiscipline::Stealth).IsEmpty());
    World->DestroyWorld(false); return true;
}
#endif
