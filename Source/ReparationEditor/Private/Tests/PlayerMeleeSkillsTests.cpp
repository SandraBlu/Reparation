#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "GAS/Ability/RPlayerMeleeSkills.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GAS/Data/LevelUpInfo.h"
#include "GAS/Data/RElementalResistanceProfile.h"
#include "AI/REnemy.h"
#include "Actors/RWeapon.h"
#include "Components/REquipmentComponent.h"
#include "Components/RInventoryComponent.h"
#include "Engine/World.h"
#include "RGameplayTags.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerMeleeSkillsTest,"Reparation.Combat.PlayerMeleeSkills",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPlayerMeleeSkillsTest::RunTest(const FString& Parameters)
{
    TGuardValue<bool> Events(GAllowActorScriptExecutionInEditor,true);
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    auto* Player=World->SpawnActor<ARPlayer>(); Player->Tags.AddUnique(TEXT("player"));
    auto* PS=World->SpawnActor<ARPlayerState>(); Player->SetPlayerState(PS);
    PS->LevelUpInfo=LoadObject<ULevelUpInfo>(nullptr,TEXT("/Game/GAS/Data/DA_XPInfo"));
    PS->SetLevel(25); PS->AddToAbilityPts(100);
    auto* ASC=CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent()); ASC->InitAbilityActorInfo(PS,Player);
    ASC->AddAttributeSetSubobject(PS->GetAttributeSet());
    // Match references normally bound by ARPlayer::InitAbilityActorInfo.
    FindFProperty<FObjectPropertyBase>(Player->GetClass(),TEXT("AbilitySystemComponent"))->SetObjectPropertyValue_InContainer(Player,ASC);
    FindFProperty<FObjectPropertyBase>(Player->GetClass(),TEXT("AttributeSet"))->SetObjectPropertyValue_InContainer(Player,PS->GetAttributeSet());
    ASC->SkillDefinitions=DuplicateObject<UAbilityInfo>(LoadObject<UAbilityInfo>(nullptr,TEXT("/Game/GAS/Data/DA_AbilityInfo")),ASC);
    // Private fixture retains technique purchase validation while separate heavy/chain authoring is pending.
    for (auto& Entry:ASC->SkillDefinitions->AbilityInfo)
        if (Entry.AbilityTag.ToString()==TEXT("ability.technique.heavyStrike") || Entry.AbilityTag.ToString()==TEXT("ability.technique.chainAttack")) Entry.bSkillTreePurchasable=true;
    ASC->UpdateAbilityStatus(25);
    const auto& T=FRGameplayTags::Get();
    const FGameplayTag Light=T.ability_Melee, Heavy=FGameplayTag::RequestGameplayTag(TEXT("ability.technique.heavyStrike")), Chain=FGameplayTag::RequestGameplayTag(TEXT("ability.technique.chainAttack"));
    const FGameplayTag Counter=FGameplayTag::RequestGameplayTag(TEXT("ability.technique.counterstrike")), Bash=FGameplayTag::RequestGameplayTag(TEXT("ability.technique.shieldBash"));
    ASC->ServerSpendAbilityPoint(Chain); TestEqual(TEXT("Chain cannot skip heavy technique"),ASC->GetLearnedSkillRank(Chain),0);
    ASC->ServerSpendAbilityPoint(Light); ASC->ServerSpendAbilityPoint(T.ability_block);
    ASC->ServerSpendAbilityPoint(Heavy); ASC->ServerSpendAbilityPoint(Chain);
    ASC->ServerSpendAbilityPoint(Counter); ASC->ServerSpendAbilityPoint(Bash);
    auto* Melee=CastChecked<URMeleeAbility>(ASC->GetSpecFromAbilityTag(Light)->GetPrimaryInstance());
    TestEqual(TEXT("Level-25 player has all four saved light alternatives"),Melee->GetUnlockedAttackCount(),4);
    auto* CounterAbility=CastChecked<URPlayerMeleeFollowup>(ASC->GetSpecFromAbilityTag(Counter)->GetPrimaryInstance());
    auto* BashAbility=CastChecked<URPlayerMeleeFollowup>(ASC->GetSpecFromAbilityTag(Bash)->GetPrimaryInstance());
    auto* Enemy=World->SpawnActor<AREnemy>(); Enemy->Tags.AddUnique(TEXT("enemy")); Enemy->SetActorLocation(FVector(150,0,0)); Enemy->SetActorRotation(FRotator(0,180,0));
    auto* EnemyASC=Enemy->GetAbilitySystemComponent(); EnemyASC->InitAbilityActorInfo(Enemy,Enemy);
    auto* Attributes=CastChecked<URAttributeSet>(Enemy->GetAttributeSet()); EnemyASC->AddAttributeSetSubobject(Attributes); Attributes->SetMaxHealth(1000); Attributes->SetHealth(1000);
    ASC->RecordSuccessfulBlock(Enemy); TestNull(TEXT("No guard cannot open counter"),ASC->GetCounterstrikeTarget());
    Player->SetIsUsingBlock(true); URAbilitySystemLibrary::NotifySuccessfulBlock(Enemy,Player);
    TestEqual(TEXT("Actual successful-block hook records attacker"),ASC->GetCounterstrikeTarget(),static_cast<AActor*>(Enemy));
    TestTrue(TEXT("Enemy in front and reach is valid"),CounterAbility->CanHitFollowupTarget(Enemy));
    Enemy->SetActorLocation(FVector(-150,0,0)); TestFalse(TEXT("Cannot counter behind player"),CounterAbility->CanHitFollowupTarget(Enemy));
    Enemy->SetActorLocation(FVector(600,0,0)); TestFalse(TEXT("Cannot counter beyond reach"),CounterAbility->CanHitFollowupTarget(Enemy));
    Enemy->SetActorLocation(FVector(150,0,0));
    TestNotNull(TEXT("Counter target remains before consumption"),ASC->GetCounterstrikeTarget());
    TestTrue(TEXT("Test ASC owner is authoritative"),ASC->GetOwner() && ASC->GetOwner()->HasAuthority());
    const bool bConsumed=ASC->ConsumeCounterstrike();
    TestTrue(TEXT("Window can be consumed once"),bConsumed); TestFalse(TEXT("No repeated counter from same block"),ASC->ConsumeCounterstrike());
    ASC->RecordSuccessfulBlock(Enemy); World->TimeSeconds+=2.; TestNull(TEXT("Expired block cannot counter"),ASC->GetCounterstrikeTarget());
    auto* Weapon=World->SpawnActor<ARWeapon>(); Weapon->CombatType=ECombatType::ECT_LightSword; Weapon->IsEquipped=true;
    Player->FindComponentByClass<UREquipmentComponent>()->EquippedWeapon=Weapon;
    const auto BashHandle=ASC->GetSpecFromAbilityTag(Bash)->Handle;
    Player->SetIsUsingBlock(false);
    TestFalse(TEXT("Bash requires existing guard"),BashAbility->CanActivateAbility(BashHandle,ASC->AbilityActorInfo.Get()));
    Player->SetIsUsingBlock(true);
    TestTrue(TEXT("Purchased bash with weapon and guard is ready"),BashAbility->CanActivateAbility(BashHandle,ASC->AbilityActorInfo.Get()));
    BashAbility->DamageTarget(Enemy);
    TestTrue(TEXT("Shield bash applies real configured damage"),Attributes->GetHealth()<1000);
    TestTrue(TEXT("Susceptible enemy receives stagger tag"),EnemyASC->HasMatchingGameplayTag(T.Debuff_Stun));
    auto* Immune=World->SpawnActor<AREnemy>(); Immune->Tags.AddUnique(TEXT("enemy")); Immune->SetActorLocation(FVector(180,0,0));
    Immune->GetAbilitySystemComponent()->InitAbilityActorInfo(Immune,Immune);
    Immune->ElementalResistanceProfile=NewObject<URElementalResistanceProfile>(); Immune->ElementalResistanceProfile->bImmuneToControl=true;
    auto* ImmuneAttributes=CastChecked<URAttributeSet>(Immune->GetAttributeSet()); Immune->GetAbilitySystemComponent()->AddAttributeSetSubobject(ImmuneAttributes); ImmuneAttributes->SetMaxHealth(1000); ImmuneAttributes->SetHealth(1000);
    BashAbility->DamageTarget(Immune);
    TestTrue(TEXT("Control immunity retains physical damage"),ImmuneAttributes->GetHealth()<1000);
    TestFalse(TEXT("Control-immune enemy resists bash stun"),Immune->GetAbilitySystemComponent()->HasMatchingGameplayTag(T.Debuff_Stun));
    PS->PlayerInventory->SetCurrency(500); Player->SetIsUsingBlock(false);
    ASC->ServerRespecBranch(ERSkillDiscipline::Melee);
    TestEqual(TEXT("Respec preserves automatic character-level light alternatives"),Melee->GetUnlockedAttackCount(),4);
    TestEqual(TEXT("Respec removes counter learned status"),ASC->GetLearnedSkillRank(Counter),0);
    World->DestroyWorld(false); return true;
}
#endif
