#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/RCoordinationTestAbility.h"
#include "GAS/Ability/RElementalMastery.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/AbilityInfo.h"
#include "GAS/Data/LevelUpInfo.h"
#include "GAS/Data/RElementalResistanceProfile.h"
#include "Framework/RPlayerState.h"
#include "Characters/RPlayer.h"
#include "AI/RProtectionComponent.h"
#include "AI/RAIController.h"
#include "Stealth/RSneakArea.h"
#include "Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AI/REnemy.h"
#include "GAS/RAttributeSet.h"
#include "RAbilityTypes.h"
#include "GameplayEffect.h"
#include "TimerManager.h"
#include "AI/RCompanion.h"
#include "AI/AIC_Companion.h"
#include "AI/RCombatCoordinator.h"
#include "GAS/Ability/RCompanionInterrupt.h"
#include "GAS/Ability/RMeleeAbility.h"
#include "Tests/ProgressionHUDProbe.h"
#include "Actors/RWeapon.h"
#include "Components/REquipmentComponent.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKatanaProgressionTest, "Reparation.Combat.SavedKatanaProgression", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKatanaProgressionTest::RunTest(const FString& Parameters)
{
    // This isolated world never begins play; allow the reflected weapon interface getter.
    TGuardValue<bool> AllowActorEvents(GAllowActorScriptExecutionInEditor, true);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    ARPlayer* Player = World->SpawnActor<ARPlayer>();
    ARPlayerState* PS = World->SpawnActor<ARPlayerState>();
    Player->SetPlayerState(PS);
    PS->LevelUpInfo = LoadObject<ULevelUpInfo>(nullptr, TEXT("/Game/GAS/Data/DA_XPInfo.DA_XPInfo"));
    PS->SetLevel(25); PS->AddToAbilityPts(30);
    URAbilitySystemComponent* ASC = CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent());
    ASC->InitAbilityActorInfo(PS, Player);
    ASC->SkillDefinitions = LoadObject<UAbilityInfo>(nullptr, TEXT("/Game/GAS/Data/DA_AbilityInfo.DA_AbilityInfo"));
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("ability.melee"));
    const FRAbilityInfo Info = ASC->SkillDefinitions->FindAbilityInfoForTag(Tag);
    if (!TestNotNull(TEXT("Saved katana ability"), Info.Ability.Get())) { World->DestroyWorld(false); return false; }
    FGameplayAbilitySpec Spec(Info.Ability, 1);
    Spec.GetDynamicSpecSourceTags().AddTag(FGameplayTag::RequestGameplayTag(TEXT("ability.status.equipped")));
    const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
    ASC->UpdateAbilityStatus(25);
    for (int32 Level = 1; Level <= 5; ++Level)
    {
        PS->SetLevel(Level);
        FGameplayAbilitySpec* Actual = ASC->FindAbilitySpecFromHandle(Handle);
        TestEqual(TEXT("Character unlocks never increase Light Strike ability rank"), Actual->Level, 1);
        URMeleeAbility* Instance = Cast<URMeleeAbility>(Actual->GetPrimaryInstance());
        if (TestNotNull(TEXT("Actual instanced katana ability"), Instance))
            TestEqual(TEXT("Saved light alternatives unlock by character levels one through four"), Instance->GetUnlockedAttackCount(), FMath::Min(Level,4));
    }
    TestEqual(TEXT("Automatic light alternatives cost no points"), PS->GetAbilityPts(), 30);
    ARWeapon* Weapon = World->SpawnActor<ARWeapon>();
    Weapon->IsEquipped = true; Weapon->CombatType = ECombatType::ECT_LightSword;
    Player->FindComponentByClass<UREquipmentComponent>()->EquippedWeapon = Weapon;
    FGameplayAbilitySpec ComboSpec(URComboProgressionTestAbility::StaticClass(), 3);
    ComboSpec.GetDynamicSpecSourceTags().AddTag(Tag);
    const FGameplayAbilitySpecHandle ComboHandle = ASC->GiveAbility(ComboSpec);
    URComboProgressionTestAbility* Combo = CastChecked<URComboProgressionTestAbility>(ASC->FindAbilitySpecFromHandle(ComboHandle)->GetPrimaryInstance());
    Combo->Configure();
    TestTrue(TEXT("Combo test ability activates"), ASC->TryActivateAbility(ComboHandle));
    TestTrue(TEXT("Combo remains active"), Combo->IsActive());
    TestEqual(TEXT("Test combo has three attacks"), Combo->GetUnlockedAttackCount(), 3);
    TestNotNull(TEXT("Test combo sees the equipped weapon"), Combo->GetMeleeWeapon());
    TestTrue(TEXT("Combo starts with equipped weapon"), Combo->BeginMeleeAttack());
    ASC->AbilityInputTagPressed(Tag);
    TestFalse(TEXT("Input outside window cannot advance combo"), Combo->AdvanceCombo());
    for (int32 Index = 1; Index <= 2; ++Index)
    {
        Combo->SetDamageWindowOpen(true);
        ASC->AbilityInputTagPressed(Tag);
        Combo->SetDamageWindowOpen(false);
        TestTrue(TEXT("Repeated attack input buffers next unlocked swing"), Combo->AdvanceCombo());
    }
    Combo->SetDamageWindowOpen(true);
    ASC->AbilityInputTagPressed(Tag);
    TestFalse(TEXT("Combo stops at final unlocked swing"), Combo->AdvanceCombo());
    ASC->CancelAbilityHandle(ComboHandle);
    TestFalse(TEXT("Cancellation clears combo state"), Combo->AdvanceCombo());
    Combo->ConfigureRandomLights();
    for (int32 Level=1; Level<=4; ++Level)
    {
        PS->SetLevel(Level);
        for (int32 Attempt=0; Attempt<12; ++Attempt)
        {
            TestTrue(TEXT("Random light activation succeeds"),ASC->TryActivateAbility(ComboHandle));
            TestTrue(TEXT("Random light starts with equipped weapon"),Combo->BeginMeleeAttack());
            const int32 Index=Combo->GetAvailableAttackIndices().IndexOfByPredicate([Combo](int32 Value)
            {
                // The saved current montage must be one of the unlocked alternatives.
                return Value==Combo->GetCurrentAttackIndexForTest();
            });
            TestTrue(TEXT("Random selection always stays within unlocked alternatives"),Index!=INDEX_NONE);
            Combo->SetDamageWindowOpen(true);
            TestFalse(TEXT("Light alternatives cannot queue a free full combo"),Combo->QueueComboAttack());
            TestFalse(TEXT("Light alternatives cannot advance into another montage"),Combo->AdvanceCombo());
            ASC->CancelAbilityHandle(ComboHandle);
        }
    }
    // The full-montage chain opts into new hit history only after a window closes.
    PS->SetLevel(25);
    TestTrue(TEXT("Contact-window probe activates"), ASC->TryActivateAbility(ComboHandle));
    TestTrue(TEXT("Contact-window probe starts"), Combo->BeginMeleeAttack());
    auto* ContactTarget = World->SpawnActor<AREnemy>();
    Combo->SetDamageWindowOpen(true);
    TestTrue(TEXT("First contact registers"), Combo->RegisterMeleeHit(ContactTarget));
    Combo->SetDamageWindowOpen(false); Combo->SetDamageWindowOpen(true);
    TestFalse(TEXT("Legacy attacks retain per-activation deduplication"), Combo->RegisterMeleeHit(ContactTarget));
    Combo->SetDamageWindowOpen(false);
    FindFProperty<FBoolProperty>(Combo->GetClass(),TEXT("bResetHitsPerDamageWindow"))->SetPropertyValue_InContainer(Combo,true);
    Combo->SetDamageWindowOpen(true);
    TestTrue(TEXT("Chain's next strike can hit the same target"), Combo->RegisterMeleeHit(ContactTarget));
    TestFalse(TEXT("One strike cannot hit the same target twice"), Combo->RegisterMeleeHit(ContactTarget));
    Combo->SetDamageWindowOpen(true);
    TestFalse(TEXT("Duplicate begin events cannot produce another hit"), Combo->RegisterMeleeHit(ContactTarget));
    ASC->CancelAbilityHandle(ComboHandle);
    TestFalse(TEXT("Cancelled chain windows cannot register a hit"), Combo->RegisterMeleeHit(ContactTarget));
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillRulesTest, "Reparation.Combat.SkillPrerequisitesAndElements", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSkillRulesTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ARPlayer* Player = World->SpawnActor<ARPlayer>(ARPlayer::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
    ARPlayerState* PS = World->SpawnActor<ARPlayerState>(); Player->SetPlayerState(PS);
    PS->LevelUpInfo = NewObject<ULevelUpInfo>(); PS->LevelUpInfo->LevelUpInfo.SetNum(41); PS->SetLevel(10); PS->AddToAbilityPts(10);
    URAbilitySystemComponent* ASC = CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent()); ASC->InitAbilityActorInfo(PS, Player);
    UAbilityInfo* Data = NewObject<UAbilityInfo>(); ASC->SkillDefinitions = Data;
    const FGameplayTag Base = FGameplayTag::RequestGameplayTag(TEXT("ability.attack.melee"));
    const FGameplayTag Fire = FGameplayTag::RequestGameplayTag(TEXT("ability.mastery.fire"));
    const FGameplayTag Available = FGameplayTag::RequestGameplayTag(TEXT("ability.status.available"));
    const FGameplayTag Unlocked = FGameplayTag::RequestGameplayTag(TEXT("ability.status.unlocked"));
    FRAbilityInfo Melee; Melee.AbilityTag = Base; Melee.Ability = URCoordinationTestAbility::StaticClass(); Melee.MaximumAbilityLevel = 3; Data->AbilityInfo.Add(Melee);
    FRAbilityInfo Element; Element.AbilityTag = Fire; Element.Ability = URFireMastery::StaticClass(); Element.MaximumAbilityLevel = 2; Element.SkillPointCost = 2;
    FRSkillPrerequisite Required; Required.AbilityTag = Base; Required.RequiredRank = 2; Element.Prerequisites.Add(Required); Data->AbilityInfo.Add(Element);
    FGameplayAbilitySpec BaseSpec(URCoordinationTestAbility::StaticClass(), 1); BaseSpec.GetDynamicSpecSourceTags().AddTag(Unlocked);
    const FGameplayAbilitySpecHandle BaseHandle = ASC->GiveAbility(BaseSpec);
    FGameplayAbilitySpec FireSpec(URFireMastery::StaticClass(), 1); FireSpec.GetDynamicSpecSourceTags().AddTag(Available);
    const FGameplayAbilitySpecHandle FireHandle = ASC->GiveAbility(FireSpec);
    ASC->ServerSpendAbilityPoint(Fire);
    TestEqual(TEXT("Unmet prerequisite prevents spending"), PS->GetAbilityPts(), 10);
    TestEqual(TEXT("Available rank one is not learned"), ASC->GetLearnedSkillRank(Fire), 0);
    ASC->ServerSelectMeleeElement(FGameplayTag::RequestGameplayTag(TEXT("damage.elemental.fire")));
    TestFalse(TEXT("Unlearned element cannot be selected"), ASC->SelectedMeleeElement.IsValid());
    ASC->ServerSpendAbilityPoint(Base);
    TestEqual(TEXT("Melee purchase advances the actual spec"), ASC->FindAbilitySpecFromHandle(BaseHandle)->Level, 2);
    ASC->ServerSpendAbilityPoint(Fire);
    TestEqual(TEXT("Learned element starts at rank one"), ASC->GetLearnedSkillRank(Fire), 1);
    TestEqual(TEXT("Authored point cost is enforced"), PS->GetAbilityPts(), 7);
    ASC->ServerSelectMeleeElement(FGameplayTag::RequestGameplayTag(TEXT("damage.elemental.fire")));
    TestTrue(TEXT("Learned element can be selected"), ASC->SelectedMeleeElement.IsValid());
    ASC->ServerSpendAbilityPoint(Fire); ASC->ServerSpendAbilityPoint(Fire);
    TestEqual(TEXT("Skill cap is separate from level forty"), ASC->FindAbilitySpecFromHandle(FireHandle)->Level, 2);
    TestEqual(TEXT("Cap does not spend extra points"), PS->GetAbilityPts(), 5);
    ASC->ServerSelectMeleeElement(FGameplayTag()); TestFalse(TEXT("Physical selection clears specialization"), ASC->SelectedMeleeElement.IsValid());
    URElementalResistanceProfile* Profile = NewObject<URElementalResistanceProfile>();
    const FGameplayTag Poison = FGameplayTag::RequestGameplayTag(TEXT("damage.poison"));
    const FGameplayTag Dark = FGameplayTag::RequestGameplayTag(TEXT("damage.dark"));
    Profile->DamageMultipliers.Add(Poison, 1.f);
    FRElementalResistanceBand Band; Band.MinimumEnemyLevel = 25; Band.DamageMultipliers.Add(Poison, 0.35f); Profile->LevelBands.Add(Band);
    Band.MinimumEnemyLevel = 35; Band.DamageMultipliers.Empty(); Band.DamageMultipliers.Add(Dark, 0.25f); Profile->LevelBands.Add(Band);
    TestEqual(TEXT("Poison remains full strength before authored band"), Profile->GetDamageMultiplier(Poison, 24), 1.f);
    TestEqual(TEXT("Poison resistance starts at exact band"), Profile->GetDamageMultiplier(Poison, 25), 0.35f);
    TestEqual(TEXT("Later dark band does not erase poison resistance"), Profile->GetDamageMultiplier(Poison, 40), 0.35f);
    TestEqual(TEXT("Dark is weaker at authored level thirty-five"), Profile->GetDamageMultiplier(Dark, 35), 0.25f);
    World->DestroyWorld(false); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuardWarningTest, "Reparation.Combat.GuardProtection", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGuardWarningTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false); World->CreateAISystem();
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ARCharacterBase* Guard = World->SpawnActor<ARCharacterBase>(ARCharacterBase::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
    ARCharacterBase* Target = World->SpawnActor<ARCharacterBase>(ARCharacterBase::StaticClass(), FVector(300.f,0.f,0.f), FRotator::ZeroRotator, Spawn);
    Target->Tags.Add(TEXT("player"));
    ARAIController* AI = World->SpawnActor<ARAIController>(); AI->Possess(Guard);
    UBlackboardData* Blackboard = NewObject<UBlackboardData>(AI);
    FBlackboardEntry TargetEntry; TargetEntry.EntryName = TEXT("TargetActor"); TargetEntry.KeyType = NewObject<UBlackboardKeyType_Object>(Blackboard);
    Blackboard->Keys.Add(TargetEntry); AI->GetBlackboardComponent()->InitializeBlackboard(*Blackboard);
    Guard->Protection->WorldRole = ERWorldRole::Guard; Guard->Protection->ProtectionRadius = 600.f; Guard->Protection->SetProtectionCenter(FVector::ZeroVector);
    AI->HandleSightUpdate(Target, true, Target->GetActorLocation());
    TestNull(TEXT("Guard cannot attack before warning grace"), AI->GetSightTarget());
    Guard->Protection->TickComponent(0.25f, LEVELTICK_All, nullptr);
    TestFalse(TEXT("First warning does not immediately permit combat"), Guard->Protection->CanEngage(Target));
    Guard->Protection->WarningGraceSeconds = 0.f;
    AI->HandleSightUpdate(Target, true, Target->GetActorLocation());
    TestTrue(TEXT("Visible intruder can be engaged after grace"), AI->GetSightTarget() == Target);
    Target->SetActorLocation(FVector(900.f,0.f,0.f)); TestFalse(TEXT("Unprovoked guard respects protected radius"), Guard->Protection->CanEngage(Target));
    Guard->Protection->NotifyProvoked(Target); TestTrue(TEXT("Attacked guard may defend itself outside warning rules"), Guard->Protection->CanEngage(Target));
    ARSneakArea* Area = World->SpawnActor<ARSneakArea>(); Area->SetActorLocation(Target->GetActorLocation()); Target->bIsCrouched = true;
    AI->HandleSightUpdate(Target, true, Target->GetActorLocation()); TestTrue(TEXT("Provocation still does not reveal concealed target"), AI->bSearching);
    World->DestroyWorld(false); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPoisonTimingTest, "Reparation.Combat.PoisonStackTiming", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPoisonTimingTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AREnemy* Source = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
    AREnemy* Target = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), FVector(300.f,0.f,0.f), FRotator::ZeroRotator, Spawn);
    UAbilitySystemComponent* ASC = Target->GetAbilitySystemComponent(); ASC->InitAbilityActorInfo(Target, Target);
    Source->GetAbilitySystemComponent()->InitAbilityActorInfo(Source, Source);
    URAttributeSet* Attributes = CastChecked<URAttributeSet>(Target->GetAttributeSet()); ASC->AddAttributeSetSubobject(Attributes);
    Attributes->SetMaxHealth(1000.f); Attributes->SetHealth(1000.f);
    UGameplayEffect* Hit = NewObject<UGameplayEffect>();
    Hit->DurationPolicy = EGameplayEffectDurationType::Instant;
    FGameplayModifierInfo Modifier; Modifier.Attribute = URAttributeSet::GetDamageAttribute(); Modifier.ModifierOp = EGameplayModOp::Additive; Modifier.ModifierMagnitude = FScalableFloat(1.f); Hit->Modifiers.Add(Modifier);
    auto ApplyPoison = [&]()
    {
        FGameplayEffectContextHandle Context = Source->GetAbilitySystemComponent()->MakeEffectContext();
        FRGameplayEffectContext* Custom = static_cast<FRGameplayEffectContext*>(Context.Get());
        Custom->SetIsSuccessfulDebuff(true); Custom->SetDamageType(MakeShared<FGameplayTag>(FGameplayTag::RequestGameplayTag(TEXT("damage.poison"))));
        Custom->SetDebuffDamage(5.f); Custom->SetDebuffDuration(4.f); Custom->SetDebuffFrequency(1.f);
        FGameplayEffectSpec Spec(Hit, Context, 1.f); ASC->ApplyGameplayEffectSpecToSelf(Spec);
    };
    auto AdvanceTimers = [World](float Seconds) { ++GFrameCounter; World->GetTimerManager().Tick(Seconds); };
    // A fresh timer manager activates pending timers only after its first tick.
    AdvanceTimers(0.f);
    ApplyPoison(); AdvanceTimers(0.6f);
    TestEqual(TEXT("Poison waits for its first periodic tick"), Attributes->GetHealth(), 999.f);
    ApplyPoison(); AdvanceTimers(0.41f);
    TestEqual(TEXT("Repeated hits preserve the existing tick timing at stack two"), Attributes->GetHealth(), 990.5f);
    ApplyPoison(); ApplyPoison(); AdvanceTimers(1.f);
    TestEqual(TEXT("Additional hits refresh poison but cannot exceed three stacks"), Attributes->GetHealth(), 977.25f);
    TestTrue(TEXT("Poison has its own affliction tag"), ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("debuff.poison"))));
    TestFalse(TEXT("Poison does not apply physical knockout"), ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("debuff.knockout"))));
    World->DestroyWorld(false); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCompanionSignatureTest, "Reparation.Companion.SignatureInterrupt", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCompanionSignatureTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false); World->CreateAISystem();
    FActorSpawnParameters Spawn; Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ARCompanion* Companion = World->SpawnActor<ARCompanion>(ARCompanion::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
    AAIC_Companion* AI = World->SpawnActor<AAIC_Companion>(); AI->Possess(Companion);
    AREnemy* Enemy = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), FVector(200.f,0.f,0.f), FRotator::ZeroRotator, Spawn);
    UAbilitySystemComponent* EnemyASC = Enemy->GetAbilitySystemComponent(); EnemyASC->InitAbilityActorInfo(Enemy, Enemy);
    UAbilitySystemComponent* CompanionASC = Companion->GetAbilitySystemComponent(); CompanionASC->InitAbilityActorInfo(Companion, Companion);
    Enemy->SetCombatTarget_Implementation(Companion); Companion->SetCombatTarget_Implementation(Enemy);
    const FGameplayAbilitySpecHandle Attack = EnemyASC->GiveAbility(FGameplayAbilitySpec(URCoordinationTestAbility::StaticClass()));
    const FGameplayAbilitySpecHandle Counter = CompanionASC->GiveAbility(FGameplayAbilitySpec(URCompanionInterrupt::StaticClass()));
    TestFalse(TEXT("Counter waits for a real enemy attack"), CompanionASC->TryActivateAbility(Counter));
    TestTrue(TEXT("Enemy begins a committed swing"), EnemyASC->TryActivateAbility(Attack));
    Enemy->ElementalResistanceProfile = NewObject<URElementalResistanceProfile>(); Enemy->ElementalResistanceProfile->bImmuneToControl = true;
    TestFalse(TEXT("Control-immune enemies resist companion counter"), CompanionASC->TryActivateAbility(Counter));
    Enemy->ElementalResistanceProfile->bImmuneToControl = false;
    TestTrue(TEXT("Nearby attacking enemy can be countered"), CompanionASC->TryActivateAbility(Counter));
    TestFalse(TEXT("Counter cancels the actual enemy activation"), EnemyASC->FindAbilitySpecFromHandle(Attack)->IsActive());
    TestEqual(TEXT("Counter releases the committed attack slot"), World->GetSubsystem<URCombatCoordinator>()->GetAttackerCount(Companion), 0);
    TestTrue(TEXT("Counter applies short stun"), EnemyASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("debuff.stun"))));
    World->DestroyWorld(false); return true;
}
#endif
