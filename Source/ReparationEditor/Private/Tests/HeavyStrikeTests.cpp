#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "GAS/Ability/RHeavyStrike.h"
#include "GAS/Ability/RChainAttack.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/AbilityInfo.h"
#include "GAS/Data/LevelUpInfo.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Actors/RWeapon.h"
#include "Components/REquipmentComponent.h"
#include "Components/RInventoryComponent.h"
#include "AI/REnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "RGameplayTags.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeavyStrikeTest,"Reparation.Combat.HeavyStrike",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FHeavyStrikeTest::RunTest(const FString& Parameters)
{
    TGuardValue<bool> Events(GAllowActorScriptExecutionInEditor,true);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game,false);
    auto* Player = World->SpawnActor<ARPlayer>(); Player->Tags.AddUnique(TEXT("player"));
    auto* PS = World->SpawnActor<ARPlayerState>(); Player->SetPlayerState(PS);
    PS->LevelUpInfo = LoadObject<ULevelUpInfo>(nullptr,TEXT("/Game/GAS/Data/DA_XPInfo"));
    PS->SetLevel(25); PS->AddToAbilityPts(30); PS->PlayerInventory->SetCurrency(500);
    auto* ASC = CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent());
    ASC->InitAbilityActorInfo(PS,Player); ASC->AddAttributeSetSubobject(PS->GetAttributeSet());
    FindFProperty<FObjectPropertyBase>(Player->GetClass(),TEXT("AbilitySystemComponent"))->SetObjectPropertyValue_InContainer(Player,ASC);
    FindFProperty<FObjectPropertyBase>(Player->GetClass(),TEXT("AttributeSet"))->SetObjectPropertyValue_InContainer(Player,PS->GetAttributeSet());
    auto* PlayerAttributes = CastChecked<URAttributeSet>(PS->GetAttributeSet());
    PlayerAttributes->SetMaxEnergy(1000); PlayerAttributes->SetEnergy(1000);
    PlayerAttributes->SetMaxStamina(1000); PlayerAttributes->SetStamina(1000);
    ASC->SkillDefinitions = DuplicateObject<UAbilityInfo>(LoadObject<UAbilityInfo>(nullptr,TEXT("/Game/GAS/Data/DA_AbilityInfo")),ASC);
    const auto& T = FRGameplayTags::Get();
    const auto Heavy = FGameplayTag::RequestGameplayTag(TEXT("ability.technique.heavyStrike"));
    ASC->UpdateAbilityStatus(25);
    auto* Spec = ASC->GetSpecFromAbilityTag(Heavy);
    if (!TestNotNull(TEXT("Heavy Strike is available from the real saved definitions"),Spec)) { World->DestroyWorld(false); return false; }
    auto* Ability = Cast<URHeavyStrike>(Spec->GetPrimaryInstance());
    if (!TestNotNull(TEXT("Heavy is the new melee ability, not inert knowledge"),Ability)) { World->DestroyWorld(false); return false; }
    if (const UGameplayEffect* Cost = Ability->GetCostGameplayEffect())
        for (const FGameplayModifierInfo& Modifier : Cost->Modifiers)
            AddInfo(FString::Printf(TEXT("Retained Katana cost attribute: %s"),*Modifier.Attribute.GetName()));
    TestFalse(TEXT("Unlearned heavy cannot activate"),Ability->CanActivateAbility(Spec->Handle,ASC->AbilityActorInfo.Get()));
    ASC->ServerEquipAbility(Heavy,T.InputTag_RMB);
    TestTrue(TEXT("Unlearned heavy cannot equip"),ASC->InputSlotIsEmpty(T.InputTag_RMB));
    ASC->ServerSpendAbilityPoint(Heavy);
    TestEqual(TEXT("Heavy cannot skip Light Strike"),ASC->GetLearnedSkillRank(Heavy),0);
    ASC->ServerSpendAbilityPoint(T.ability_Melee);
    ASC->ServerSpendAbilityPoint(Heavy);
    TestEqual(TEXT("Heavy is learned once"),ASC->GetLearnedSkillRank(Heavy),1);
    ASC->ServerEquipAbility(Heavy,T.InputTag_RMB);
    TestEqual(TEXT("Heavy uses an ordinary active input"),ASC->GetInputTagFromAbilityTag(Heavy),T.InputTag_RMB);
    TestFalse(TEXT("Learned heavy still needs an equipped weapon"),Ability->CanActivateAbility(Spec->Handle,ASC->AbilityActorInfo.Get()));
    auto* Weapon = World->SpawnActor<ARWeapon>(); Weapon->CombatType=ECombatType::ECT_LightSword; Weapon->IsEquipped=true;
    Player->FindComponentByClass<UREquipmentComponent>()->EquippedWeapon=Weapon;
    FGameplayTagContainer Failure;
    const bool bCanActivate = Ability->CanActivateAbility(Spec->Handle,ASC->AbilityActorInfo.Get(),nullptr,nullptr,&Failure);
    if (!bCanActivate)
    {
        AddInfo(FString::Printf(TEXT("Heavy activation: failures=%s cost=%d cooldown=%d weapon=%d blocking=%d dead=%d learned=%d available=%d energy=%.1f"),
            *Failure.ToStringSimple(),Ability->CheckCost(Spec->Handle,ASC->AbilityActorInfo.Get()),
            Ability->CheckCooldown(Spec->Handle,ASC->AbilityActorInfo.Get()),Ability->GetMeleeWeapon()!=nullptr,
            URAbilitySystemLibrary::IsActorBlocking(Player),Player->IsDead_Implementation(),ASC->GetLearnedSkillRank(Heavy),
            Ability->GetAvailableAttackIndices().Num(),PlayerAttributes->GetEnergy()));
    }
    TestTrue(TEXT("Learned heavy with weapon and energy can activate"),bCanActivate);
    ASC->AddLooseGameplayTag(T.ability_Melee);
    TestFalse(TEXT("Heavy cannot interrupt active light"),Ability->CanActivateAbility(Spec->Handle,ASC->AbilityActorInfo.Get()));
    ASC->RemoveLooseGameplayTag(T.ability_Melee);
    Player->SetIsUsingBlock(true);
    TestFalse(TEXT("Heavy cannot replace held guard"),Ability->CanActivateAbility(Spec->Handle,ASC->AbilityActorInfo.Get()));
    Player->SetIsUsingBlock(false);
    TestEqual(TEXT("All three saved slash alternatives are eligible"),Ability->GetAvailableAttackIndices().Num(),3);
    Ability->bThrustAttack=true;
    const TArray<int32> ThrustIndices = Ability->GetAvailableAttackIndices();
    TestEqual(TEXT("Both saved thrust alternatives are eligible"),ThrustIndices.Num(),2);
    for (const int32 Index : ThrustIndices)
    {
        Ability->CurrentAttackIndex=Index;
        TestTrue(TEXT("Thrust lookup never selects a slash"),Ability->ThrustAttacks.ContainsByPredicate([&](const FRMeleeAttack& A){ return A.Montage==Ability->GetCurrentAttack().Montage; }));
    }
    Ability->CurrentAttackIndex=INDEX_NONE; Ability->bThrustAttack=false;
    Player->GetCharacterMovement()->Velocity=FVector(200,0,0);
    TestTrue(TEXT("Forward movement requests thrust"),Ability->ShouldUseThrust());
    Player->GetCharacterMovement()->Velocity=FVector(0,200,0);
    TestFalse(TEXT("Strafing requests slash"),Ability->ShouldUseThrust());
    Player->GetCharacterMovement()->Velocity=FVector(-200,0,0);
    TestFalse(TEXT("Backward movement requests slash"),Ability->ShouldUseThrust());
    Player->GetCharacterMovement()->Velocity=FVector::ZeroVector;
    TestFalse(TEXT("Standing requests slash"),Ability->ShouldUseThrust());
    auto* Enemy = World->SpawnActor<AREnemy>(); Enemy->Tags.AddUnique(TEXT("enemy")); Enemy->SetActorLocation(FVector(150,0,0));
    auto* EnemyASC = Enemy->GetAbilitySystemComponent(); EnemyASC->InitAbilityActorInfo(Enemy,Enemy);
    auto* Attributes = CastChecked<URAttributeSet>(Enemy->GetAttributeSet()); EnemyASC->AddAttributeSetSubobject(Attributes);
    Attributes->SetMaxHealth(1000); Attributes->SetHealth(1000);
    FHitResult Hit; Hit.HitObjectHandle=FActorInstanceHandle(Enemy);
    Ability->ProcessMeleeHit(Hit);
    TestTrue(TEXT("Heavy applies the real configured damage effect"),Attributes->GetHealth()<1000);
    ASC->ServerRespecBranch(ERSkillDiscipline::Melee);
    TestEqual(TEXT("Respec removes learned Heavy Strike"),ASC->GetLearnedSkillRank(Heavy),0);
    TestTrue(TEXT("Respec clears its active slot"),ASC->InputSlotIsEmpty(T.InputTag_RMB));
    TestFalse(TEXT("Respec prevents activation"),Ability->CanActivateAbility(Spec->Handle,ASC->AbilityActorInfo.Get()));
    const auto Chain = FGameplayTag::RequestGameplayTag(TEXT("ability.technique.chainAttack"));
    auto* ChainSpec = ASC->GetSpecFromAbilityTag(Chain);
    if (TestNotNull(TEXT("Chain is available from saved definitions"),ChainSpec))
    {
        auto* ChainAbility = Cast<URChainAttack>(ChainSpec->GetPrimaryInstance());
        if (TestNotNull(TEXT("Chain is a real full-montage attack"),ChainAbility))
        {
            TestFalse(TEXT("Unlearned Chain cannot activate"),ChainAbility->CanActivateAbility(ChainSpec->Handle,ASC->AbilityActorInfo.Get()));
            ASC->ServerSpendAbilityPoint(T.ability_Melee);
            ASC->ServerSpendAbilityPoint(Chain);
            TestEqual(TEXT("Chain cannot skip Heavy prerequisite"),ASC->GetLearnedSkillRank(Chain),0);
            ASC->ServerSpendAbilityPoint(Heavy);
            ASC->ServerSpendAbilityPoint(Chain);
            TestEqual(TEXT("Chain purchase learns the attack"),ASC->GetLearnedSkillRank(Chain),1);
            ASC->ServerEquipAbility(Chain,T.InputTag_RMB);
            TestEqual(TEXT("Learned Chain can use an active slot"),ASC->GetInputTagFromAbilityTag(Chain),T.InputTag_RMB);
            TestTrue(TEXT("Learned Chain is ready with the existing weapon"),ChainAbility->CanActivateAbility(ChainSpec->Handle,ASC->AbilityActorInfo.Get()));
            TestEqual(TEXT("All four full-combo alternatives are available"),ChainAbility->GetAvailableAttackIndices().Num(),4);
            Player->GetCharacterMovement()->Velocity=FVector(200,0,0);
            TestFalse(TEXT("Moving forward never routes Chain to Heavy thrusts"),ChainAbility->ShouldUseThrust());
            ASC->AddLooseGameplayTag(T.ability_Melee);
            TestFalse(TEXT("Chain cannot replace active light"),ChainAbility->CanActivateAbility(ChainSpec->Handle,ASC->AbilityActorInfo.Get()));
            ASC->RemoveLooseGameplayTag(T.ability_Melee);
            const float BeforeChainHit = Attributes->GetHealth();
            ChainAbility->ProcessMeleeHit(Hit);
            TestTrue(TEXT("Chain uses the real damage effect"),Attributes->GetHealth()<BeforeChainHit);
            ASC->ServerRespecBranch(ERSkillDiscipline::Melee);
            TestEqual(TEXT("Respec removes Chain"),ASC->GetLearnedSkillRank(Chain),0);
            TestTrue(TEXT("Respec clears Chain's slot"),ASC->InputSlotIsEmpty(T.InputTag_RMB));
        }
    }
    World->DestroyWorld(false);
    return true;
}
#endif
