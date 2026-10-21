#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/AbilityInfo.h"
#include "RGameplayTags.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLoadoutRulesTest, "Reparation.Combat.LoadoutRules", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLoadoutRulesTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    auto* Player = World->SpawnActor<ARPlayer>(); auto* PS = World->SpawnActor<ARPlayerState>(); Player->SetPlayerState(PS);
    auto* ASC = CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent()); ASC->InitAbilityActorInfo(PS, Player);
    ASC->SkillDefinitions = DuplicateObject<UAbilityInfo>(LoadObject<UAbilityInfo>(nullptr, TEXT("/Game/GAS/Data/DA_AbilityInfo")), ASC);
    const auto& T = FRGameplayTags::Get();
    const FGameplayTag Katana = FGameplayTag::RequestGameplayTag(TEXT("ability.melee"));
    const FGameplayTag Grenade = FGameplayTag::RequestGameplayTag(TEXT("ability.grenade.stun"));
    const FGameplayTag Stamina = FGameplayTag::RequestGameplayTag(TEXT("ability.passive.staminaSiphon"));
    const FGameplayTag Siphon = FGameplayTag::RequestGameplayTag(TEXT("ability.passive.lifeSiphon"));
    ASC->UpdateAbilityStatus(25);
    ASC->ServerEquipAbility(Katana, T.InputTag_LMB);
    TestTrue(TEXT("Available rank-one spec cannot equip"), ASC->InputSlotIsEmpty(T.InputTag_LMB));
    PS->SetLevel(25); PS->AddToAbilityPts(20);
    for (FGameplayTag Skill : {Katana, Grenade, Stamina, Siphon}) ASC->ServerSpendAbilityPoint(Skill);
    PS->SetLevel(1);
    TestTrue(TEXT("One passive starts unlocked"), ASC->IsLoadoutSlotUnlocked(T.InputTag_Passive01));
    TestFalse(TEXT("Second passive locked initially"), ASC->IsLoadoutSlotUnlocked(T.InputTag_Passive02));
    TestFalse(TEXT("Fourth passive disabled by default"), ASC->IsLoadoutSlotUnlocked(T.InputTag_Passive04));
    ASC->ServerEquipAbility(Katana, T.InputTag_Passive01);
    ASC->ServerEquipAbility(Stamina, T.InputTag_LMB);
    ASC->ServerEquipAbility(Stamina, T.InputTag_Passive02);
    ASC->ServerEquipAbility(Katana, T.InputTag_Rage);
    TestFalse(TEXT("Wrong type, locked and fifth inputs rejected"), ASC->GetInputTagFromAbilityTag(Katana).IsValid());
    TestFalse(TEXT("Rejected passive never assigned"), ASC->GetInputTagFromAbilityTag(Stamina).IsValid());
    ASC->ServerEquipAbility(Katana, T.InputTag_LMB);
    ASC->ServerEquipAbility(Grenade, T.InputTag_LMB);
    TestEqual(TEXT("Replaced skill returns to learned status"), ASC->GetStatusFromAbilityTag(Katana), T.ability_status_unlocked);
    TestEqual(TEXT("New occupant is equipped"), ASC->GetStatusFromAbilityTag(Grenade), T.ability_status_equipped);
    ASC->ServerEquipAbility(Grenade, T.InputTag_RMB);
    TestTrue(TEXT("Moving frees former input"), ASC->InputSlotIsEmpty(T.InputTag_LMB));
    PS->SetLevel(9); TestFalse(TEXT("Passive two locked at nine"), ASC->IsLoadoutSlotUnlocked(T.InputTag_Passive02));
    PS->SetLevel(10); TestTrue(TEXT("Passive two unlocks at ten"), ASC->IsLoadoutSlotUnlocked(T.InputTag_Passive02));
    PS->SetLevel(19); TestFalse(TEXT("Passive three locked at nineteen"), ASC->IsLoadoutSlotUnlocked(T.InputTag_Passive03));
    PS->SetLevel(20); TestTrue(TEXT("Passive three unlocks at twenty"), ASC->IsLoadoutSlotUnlocked(T.InputTag_Passive03));
    ASC->ServerEquipAbility(Stamina, T.InputTag_Passive02);
    TestEqual(TEXT("Learned passive occupies unlocked slot"), ASC->GetInputTagFromAbilityTag(Stamina), T.InputTag_Passive02);
    TestTrue(TEXT("Equipped passive runs its ability"), ASC->GetSpecFromAbilityTag(Stamina)->IsActive());
    ASC->ServerEquipAbility(Siphon, T.InputTag_Passive02);
    TestTrue(TEXT("Replacement passive runs its ability"), ASC->GetSpecFromAbilityTag(Siphon)->IsActive());
    TestFalse(TEXT("Replaced passive loses its slot"), ASC->GetInputTagFromAbilityTag(Stamina).IsValid());
    TestFalse(TEXT("Replaced passive stops running"), ASC->GetSpecFromAbilityTag(Stamina)->IsActive());
    PS->SetLevel(1);
    TestFalse(TEXT("Lowering test level removes newly locked passive"), ASC->GetInputTagFromAbilityTag(Siphon).IsValid());
    TestFalse(TEXT("Removed passive stops running"), ASC->GetSpecFromAbilityTag(Siphon)->IsActive());
    ASC->SkillDefinitions->PassiveSlotUnlockLevels = {1, 5, 15, 30}; PS->SetLevel(30);
    TestTrue(TEXT("Fourth passive can be enabled through data"), ASC->IsLoadoutSlotUnlocked(T.InputTag_Passive04));
    ASC->ServerUnequipAbility(Grenade);
    TestEqual(TEXT("Unequip retains skill rank"), ASC->GetLearnedSkillRank(Grenade), 1);
    TestFalse(TEXT("Unequip frees input"), ASC->GetInputTagFromAbilityTag(Grenade).IsValid());
    World->DestroyWorld(false); return true;
}
#endif
