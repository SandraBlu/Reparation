#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "GAS/Ability/RDamageAbility.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "AI/REnemy.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAttackSelectionTest, "Reparation.Combat.TargetStanceMontages",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnemyAttackSelectionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world"), World)) return false;
	ACharacter* Target = World->SpawnActor<ACharacter>();
	FTaggedMontage Low, Across, General;
	Low.Montage = NewObject<UAnimMontage>();
	Low.TargetStance = EAttackTargetStance::Crouching;
	Across.Montage = NewObject<UAnimMontage>();
	Across.TargetStance = EAttackTargetStance::Standing;
	General.Montage = NewObject<UAnimMontage>();
	Target->bIsCrouched = true;
	TestTrue(TEXT("Crouched target selects low attack over high sweep or general fallback"), URDamageAbility::SelectMontageForTarget({Low, Across, General}, Target).Montage == Low.Montage);
	TestNull(TEXT("High-only attack list does not attack a crouched target"), URDamageAbility::SelectMontageForTarget({Across}, Target).Montage);
	Target->bIsCrouched = false;
	TestTrue(TEXT("Standing target selects standing attack"), URDamageAbility::SelectMontageForTarget({Low, Across, General}, Target).Montage == Across.Montage);
	TestTrue(TEXT("Legacy unrestricted attacks remain compatible"), URDamageAbility::SelectMontageForTarget({General}, Target).Montage == General.Montage);
	TestNull(TEXT("Missing target does not select a stance-restricted attack"), URDamageAbility::SelectMontageForTarget({Low, Across}, nullptr).Montage);
	AREnemy* Enemy = World->SpawnActor<AREnemy>();
	float Min, Preferred, Max;
	Target->bIsCrouched = true;
	Enemy->GetMeleeRangeForTarget(Target, Min, Preferred, Max);
	TestEqual(TEXT("Crouched spacing keeps its established minimum"), Min, 150.f);
	TestEqual(TEXT("Crouched spacing keeps its established preferred range"), Preferred, 190.f);
	TestEqual(TEXT("Crouched spacing keeps its established maximum"), Max, 230.f);
	Target->bIsCrouched = false;
	Enemy->GetMeleeRangeForTarget(Target, Min, Preferred, Max);
	TestEqual(TEXT("Standing attack stays closer"), Preferred, 120.f);
	TestTrue(TEXT("Standing maximum is closer than crouched minimum"), Max < Enemy->MinMeleeDistance);
	Enemy->StandingMaxMeleeDistance = 50.f;
	Enemy->StandingPreferredMeleeDistance = 200.f;
	Enemy->GetMeleeRangeForTarget(Target, Min, Preferred, Max);
	TestTrue(TEXT("Edited spacing limits normalize to a valid ordered range"), Min <= Preferred && Preferred <= Max);
	World->DestroyWorld(false);
	return true;
}
#endif
