#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "AI/RCompanion.h"
#include "AI/AIC_Companion.h"
#include "AI/REnemy.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Ability/RCompanionAttackAbility.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "Components/RCompanionPartyComponent.h"
#include "Components/SkeletalMeshComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCompanionCombatOrdersTest, "Reparation.Companion.CombatOrders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCompanionCombatOrdersTest::RunTest(const FString& Parameters)
{
	UClass* TomoeClass = LoadClass<ARCompanion>(nullptr, TEXT("/Game/AI/BP/Companion/BP_Tomoe.BP_Tomoe_C"));
	if (!TestNotNull(TEXT("Configured Tomoe Blueprint loads"), TomoeClass)) return false;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world"), World)) return false;
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARCompanion* Companion = World->SpawnActor<ARCompanion>(TomoeClass, FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
	if (!TestNotNull(TEXT("Tomoe spawns"), Companion)) { World->DestroyWorld(false); return false; }
	// This isolated world does not run InitializeActorsForPlay. Initialize the ASC
	// before BeginPlay so it discovers the native attribute-set subobject.
	Companion->InitializeComponents();
	// The editor-created world suppresses gameplay callbacks by default.
	Companion->DispatchBeginPlay(true);
	AAIC_Companion* AI = Cast<AAIC_Companion>(Companion->GetController());
	if (!AI)
	{
		AI = World->SpawnActor<AAIC_Companion>();
		AI->Possess(Companion);
	}
	const URAttributeSet* Attributes = Cast<URAttributeSet>(Companion->GetAttributeSet());
	AddInfo(FString::Printf(TEXT("Tomoe: authority=%d begun=%d attributes=%d health=%.1f max=%.1f abilities=%d"),
		Companion->HasAuthority(), Companion->HasActorBegunPlay(), Companion->GetAbilitySystemComponent()->GetSpawnedAttributes().Num(),
		Attributes ? Attributes->GetHealth() : -1.f, Attributes ? Attributes->GetMaxHealth() : -1.f,
		Companion->GetAbilitySystemComponent()->GetActivatableAbilities().Num()));
	AddInfo(FString::Printf(TEXT("Tomoe attack: dead=%d class=%s granted=%d configured=%d montageCount=%d"),
		Companion->IsDead_Implementation(), *GetNameSafe(Companion->AttackAbility.Get()),
		Companion->AttackAbility && Companion->GetAbilitySystemComponent()->FindAbilitySpecFromClass(Companion->AttackAbility) != nullptr,
		Companion->AttackAbility && Companion->AttackAbility->GetDefaultObject<URCompanionAttackAbility>()->IsConfiguredFor(Companion),
		IRCombatInterface::Execute_GetAttackMontages(Companion).Num()));
	TestTrue(TEXT("Companion health initializes above zero"), Attributes && Attributes->GetHealth() > 0.f);
	TestTrue(TEXT("Combat ability and montage are ready"), Companion->IsCombatReady());
	TestTrue(TEXT("Tomoe requires her assigned weapon"), Companion->bRequiresEquippedWeapon);
	TestTrue(TEXT("Assigned katana is equipped"), Companion->HasEquippedWeapon());
	TestEqual(TEXT("Right weapon uses requested socket"), Companion->RightWeaponSlot->GetAttachSocketName(), FName(TEXT("weapon_r")));
	TestTrue(TEXT("Left hand socket is available for a second blade"), Companion->GetMesh()->DoesSocketExist(TEXT("weapon_l")));
	USkeletalMesh* KatanaMesh = Companion->RightWeaponSlot->GetSkeletalMeshAsset();
	Companion->RightWeaponSlot->SetSkeletalMeshAsset(nullptr);
	Companion->RefreshWeaponAttachments();
	TestFalse(TEXT("Empty weapon slots prevent required-weapon combat"), Companion->IsCombatReady());
	TestFalse(TEXT("Attack ability rejects a missing required weapon"), Companion->AttackAbility->GetDefaultObject<URCompanionAttackAbility>()->IsConfiguredFor(Companion));
	Companion->LeftWeaponSlot->SetSkeletalMeshAsset(KatanaMesh);
	Companion->RefreshWeaponAttachments();
	TestTrue(TEXT("Left-only loadout is combat-ready"), Companion->IsCombatReady());
	TestEqual(TEXT("Left weapon attaches to weapon_l"), Companion->LeftWeaponSlot->GetAttachSocketName(), FName(TEXT("weapon_l")));
	Companion->RightWeaponSlot->SetSkeletalMeshAsset(KatanaMesh);
	Companion->RefreshWeaponAttachments();
	TestTrue(TEXT("Both blade slots can be equipped together"), Companion->RightWeaponSlot->IsVisible() && Companion->LeftWeaponSlot->IsVisible());
	Companion->LeftWeaponSlot->SetSkeletalMeshAsset(nullptr);
	Companion->RefreshWeaponAttachments();
	ACharacter* Leader = World->SpawnActor<ACharacter>(ACharacter::StaticClass(), FVector(1000.f, 0.f, 0.f), FRotator::ZeroRotator, Spawn);
	const bool OriginalPathAcceleration = Companion->GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths;
	TestTrue(TEXT("Follow starts"), AI->FollowLeader(Leader));
	TestTrue(TEXT("Following uses acceleration for smooth path movement"), Companion->GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths != 0);
	TWeakObjectPtr<ATargetPoint> MovingGoal;
	for (TActorIterator<ATargetPoint> It(World); It; ++It)
		if (It->GetOwner() == AI) MovingGoal = *It;
	TestTrue(TEXT("Follow creates a moving formation goal"), MovingGoal.IsValid());
	UCharacterMovementComponent* Movement = Companion->GetCharacterMovement();
	const float OriginalWalkSpeed = Movement->MaxWalkSpeed;
	const float OriginalCrouchSpeed = Movement->MaxWalkSpeedCrouched;
	Leader->GetCharacterMovement()->Velocity = FVector(100.f, 0.f, 0.f);
	Companion->SetActorLocation(Leader->GetActorLocation() + AI->FormationOffset - FVector(AI->ArrivalDistance + 50.f, 0.f, 0.f));
	AI->Tick(0.1f);
	if (MovingGoal.IsValid()) TestTrue(TEXT("Moving goal tracks leader formation"), MovingGoal->GetActorLocation().Equals(Leader->GetNavAgentLocation() + AI->FormationOffset));
	TestTrue(TEXT("Follow reduces speed near formation rather than rushing to a stop"), Movement->MaxWalkSpeed < OriginalWalkSpeed);
	TestTrue(TEXT("Follow also reduces crouched speed"), Movement->MaxWalkSpeedCrouched < OriginalCrouchSpeed);
	AI->Stay();
	TestFalse(TEXT("Stay destroys the transient formation goal"), MovingGoal.IsValid());
	TestEqual(TEXT("Stay restores path acceleration setting"), Companion->GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths != 0, OriginalPathAcceleration);
	TestEqual(TEXT("Stay restores configured walking speed"), Movement->MaxWalkSpeed, OriginalWalkSpeed);
	TestEqual(TEXT("Stay restores configured crouched speed"), Movement->MaxWalkSpeedCrouched, OriginalCrouchSpeed);
	Companion->SetActorLocation(FVector::ZeroVector);
	Leader->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	TestTrue(TEXT("Follow can be reissued after speed restoration"), AI->FollowLeader(Leader));
	TestFalse(TEXT("Player cannot be an attack target"), AI->Attack(Leader));
	TestEqual(TEXT("Rejected attack preserves Follow"), AI->CurrentOrder, ERCompanionOrder::Follow);
	AREnemy* Enemy = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, Spawn);
	Companion->RightWeaponSlot->SetVisibility(false);
	Companion->RightWeaponSlot->SetHiddenInGame(true);
	TestTrue(TEXT("Hidden assigned weapon does not block acceptance of combat orders"), Companion->IsCombatReady());
	TestFalse(TEXT("Hidden weapon is not already equipped"), Companion->HasEquippedWeapon());
	TestTrue(TEXT("Living enemy is accepted"), AI->Attack(Enemy));
	TestTrue(TEXT("Accepting Attack equips and reveals assigned weapon"), Companion->HasEquippedWeapon());
	TestEqual(TEXT("Attack interrupts Follow"), AI->CurrentOrder, ERCompanionOrder::Attack);
	Enemy->Destroy();
	AI->Tick(0.25f);
	TestEqual(TEXT("Lost target restores Follow"), AI->CurrentOrder, ERCompanionOrder::Follow);
	Enemy = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, Spawn);
	TestTrue(TEXT("Second attack starts"), AI->Attack(Enemy));
	AI->Stay();
	Enemy->Destroy();
	AI->Tick(0.25f);
	TestEqual(TEXT("Stay cancels the attack and discards the old Follow"), AI->CurrentOrder, ERCompanionOrder::Stay);
	TestNull(TEXT("Canceled attack clears the combat target"), Companion->GetCombatTarget_Implementation());
	URCompanionPartyComponent* Party = NewObject<URCompanionPartyComponent>(Leader);
	Party->RegisterComponent();
	UAbilitySystemComponent* LeaderASC = NewObject<UAbilitySystemComponent>(Leader);
	LeaderASC->RegisterComponent();
	TestTrue(TEXT("Companion registers for party assistance"), Party->AddCompanion(Companion));
	Enemy = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), Leader->GetActorLocation() + FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, Spawn);
	TestFalse(TEXT("Idle enemies do not provoke automatic attacks"), AI->TryAutomaticAssist(Enemy, Leader));
	Enemy->SetCombatTarget_Implementation(Leader);
	Leader->bIsCrouched = true;
	TestFalse(TEXT("Sneaking suppresses even an enemy targeting the player"), AI->TryAutomaticAssist(Enemy, Leader));
	URDamageAbility* PlayerAttack = NewObject<URDamageAbility>();
	PlayerAttack->bSilentAttack = true;
	LeaderASC->AbilityActivatedCallbacks.Broadcast(PlayerAttack);
	TestFalse(TEXT("Silent ability activation preserves sneak suppression"), Party->CanAssistNow());
	PlayerAttack->bSilentAttack = false;
	LeaderASC->AbilityActivatedCallbacks.Broadcast(PlayerAttack);
	TestTrue(TEXT("Normal attack activation releases sneak suppression"), Party->CanAssistNow());
	Party->TickComponent(Party->CombatMemoryDuration + 1.f, LEVELTICK_All, nullptr);
	URCompanionPartyComponent::NotifyReceivedDamage(Companion, Enemy);
	TestTrue(TEXT("Damage to a party companion releases sneak suppression"), Party->CanAssistNow());
	TestTrue(TEXT("Nearby threat starts automatic assistance from Stay"), AI->TryAutomaticAssist(Enemy, Leader));
	TestTrue(TEXT("Automatic combat is distinguishable from explicit Attack"), AI->bAssistingAutomatically);
	Enemy->SetActorLocation(Leader->GetActorLocation() + FVector(Companion->AssistanceRadius + 500.f, 0.f, 0.f));
	AI->Tick(0.1f);
	TestEqual(TEXT("Threat leaving assistance radius restores Stay"), AI->CurrentOrder, ERCompanionOrder::Stay);
	TestFalse(TEXT("Assistance flag clears when prior order resumes"), AI->bAssistingAutomatically);
	AI->Tick(3.f);
	Party->TickComponent(Party->CombatMemoryDuration + 1.f, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Sneaking suppression returns after combat memory expires"), Party->CanAssistNow());
	Leader->bIsCrouched = false;
	Enemy->SetActorLocation(Leader->GetActorLocation() + FVector(100.f, 0.f, 0.f));
	Companion->SetActorLocation(Leader->GetActorLocation() + FVector(Companion->AssistanceRadius + 100.f, 0.f, 0.f));
	TestFalse(TEXT("Distant companion keeps its assignment"), AI->TryAutomaticAssist(Enemy, Leader));
	Companion->SetActorLocation(Leader->GetActorLocation());
	AI->CurrentOrder = ERCompanionOrder::GoToLocation;
	TestFalse(TEXT("Travel work is not interrupted by assistance"), AI->TryAutomaticAssist(Enemy, Leader));
	AI->Stay();
	Companion->bAutomaticAssistance = false;
	TestFalse(TEXT("Per-companion assistance can be disabled"), AI->TryAutomaticAssist(Enemy, Leader));
	Companion->bAutomaticAssistance = true;
	TestTrue(TEXT("Follow resumes before assistance"), AI->FollowLeader(Leader));
	Party->TickComponent(0.25f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Party scan automatically interrupts Follow for a nearby threat"), AI->bAssistingAutomatically);
	TestEqual(TEXT("Automatic party scan selects Attack"), AI->CurrentOrder, ERCompanionOrder::Attack);
	// Hold a committed swing active so this navigation-free fixture can exercise
	// the real controller's disengagement rules without starting another montage.
	FGameplayAbilitySpec* AttackSpec = Companion->GetAbilitySystemComponent()->FindAbilitySpecFromClass(Companion->AttackAbility);
	if (TestNotNull(TEXT("Attack spec for grace-period checks"), AttackSpec))
	{
		AttackSpec->ActiveCount = 1;
		Leader->bIsCrouched = true;
		TestFalse(TEXT("Crouching still blocks initiation of new assistance"), Party->CanAssistNow());
		AI->Tick(0.1f);
		TestEqual(TEXT("Player crouching does not cancel combat already underway"), AI->CurrentOrder, ERCompanionOrder::Attack);
		Enemy->SetActorLocation(Companion->GetActorLocation() - Companion->GetActorForwardVector() * 100.f);
		Enemy->SetCombatTarget_Implementation(nullptr);
		AI->Tick(1.f);
		TestEqual(TEXT("Enemy moving behind companion retains combat through the grace period while player crouches"), AI->CurrentOrder, ERCompanionOrder::Attack);
		Enemy->SetCombatTarget_Implementation(Leader);
		AI->Tick(0.1f);
		Enemy->SetCombatTarget_Implementation(nullptr);
		AI->Tick(2.f);
		TestEqual(TEXT("Renewed threat resets the grace period"), AI->CurrentOrder, ERCompanionOrder::Attack);
		AttackSpec->ActiveCount = 0;
		AI->Tick(1.1f);
		TestEqual(TEXT("Sustained quiet restores the previous Follow order"), AI->CurrentOrder, ERCompanionOrder::Follow);
		Leader->bIsCrouched = false;
		AI->Stay();
		AI->Tick(3.f);
		Enemy->SetCombatTarget_Implementation(Leader);
		TestTrue(TEXT("Assistance can start again for death check"), AI->TryAutomaticAssist(Enemy, Leader));
		AttackSpec->ActiveCount = 1;
		Enemy->SetCombatTarget_Implementation(nullptr);
		AI->Tick(0.1f);
		AttackSpec->ActiveCount = 0;
		Enemy->MulticastHandleDeath_Implementation(FVector::ZeroVector);
		AI->Tick(0.1f);
		TestEqual(TEXT("Enemy death bypasses the remaining combat grace period"), AI->CurrentOrder, ERCompanionOrder::Stay);
		TestFalse(TEXT("Enemy death ends automatic assistance immediately"), AI->bAssistingAutomatically);
		TestFalse(TEXT("Dead enemy cannot become an attack target again"), Companion->CanAttackTarget(Enemy));
		TestFalse(TEXT("Dead enemy is removed from party threats"), Party->IsActiveThreat(Enemy));
	}
	Enemy->Destroy();
	Leader->bIsCrouched = true;
	Enemy = World->SpawnActor<AREnemy>(AREnemy::StaticClass(), Companion->GetActorLocation() + FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, Spawn);
	Enemy->SetCombatTarget_Implementation(Leader);
	Party->TickComponent(Party->CombatMemoryDuration + 1.f, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Sneaking still suppresses ordinary assistance after combat memory expires"), Party->CanAssistNow());
	TestEqual(TEXT("Player-directed threat does not restart assistance while sneaking"), AI->CurrentOrder, ERCompanionOrder::Stay);
	Enemy->SetCombatTarget_Implementation(Companion);
	Companion->RightWeaponSlot->SetVisibility(false);
	Companion->RightWeaponSlot->SetHiddenInGame(true);
	Party->TickComponent(0.25f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Enemy targeting companion restarts defense without a Follow command while leader sneaks"), AI->bAssistingAutomatically);
	TestEqual(TEXT("Self defense interrupts Stay despite the post-combat retry delay"), AI->CurrentOrder, ERCompanionOrder::Attack);
	TestTrue(TEXT("Automatic self defense equips a hidden assigned weapon"), Companion->HasEquippedWeapon());
	Enemy->Destroy();
	AI->Tick(0.1f);
	TestEqual(TEXT("Self defense resumes previous Stay assignment after target destruction"), AI->CurrentOrder, ERCompanionOrder::Stay);
	Party->RemoveCompanion(Companion);
	TestFalse(TEXT("Removing a companion clears its party association"), Companion->RegisteredParty.IsValid());
	World->DestroyWorld(false);
	return true;
}
#endif
