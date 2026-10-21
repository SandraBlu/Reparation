#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Framework/RPIETestSettings.h"
#include "Framework/RPlayerState.h"
#include "GAS/Data/LevelUpInfo.h"
#include "Engine/World.h"
#include "UI/GAS/Controllers/ROverlayWidgetController.h"
#include "Tests/ProgressionHUDProbe.h"
#include "Components/RInventoryComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPIETestSettingsTest, "Reparation.Combat.PIETestOverrides",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPIETestSettingsTest::RunTest(const FString& Parameters)
{
    URPIETestSettings* Settings = GetMutableDefault<URPIETestSettings>();
    TGuardValue<bool> Enabled(Settings->bEnableTestOverrides, true);
    TGuardValue<int32> Level(Settings->StartingLevel, 25);
    TGuardValue<int32> Points(Settings->StartingSkillPoints, 30);
    TGuardValue<int32> Attributes(Settings->StartingAttributePoints, 10);
    TGuardValue<int32> Gold(Settings->StartingGold, 500);
    ULevelUpInfo* Data = NewObject<ULevelUpInfo>();
    Data->LevelUpInfo.SetNum(41);
    for (int32 Index = 0; Index < 41; ++Index) Data->LevelUpInfo[Index].LevelUpRequirement = Index * 100;

    UWorld* Game = UWorld::CreateWorld(EWorldType::Game, false);
    ARPlayerState* Normal = Game->SpawnActor<ARPlayerState>();
    Normal->LevelUpInfo = Data;
    Normal->ApplyPIETestOverrides();
    TestEqual(TEXT("Normal game ignores enabled overrides"), Normal->GetPlayerLevel(), 1);
    TestEqual(TEXT("Normal game receives no test points"), Normal->GetAbilityPts(), 0);
    TestEqual(TEXT("Normal game receives no test gold"), Normal->PlayerInventory->GetCurrency(), 0);

    UWorld* PIE = UWorld::CreateWorld(EWorldType::PIE, false);
    ARPlayerState* Player = PIE->SpawnActor<ARPlayerState>();
    Player->LevelUpInfo = Data;
    Settings->bEnableTestOverrides = false;
    Player->ApplyPIETestOverrides();
    TestEqual(TEXT("Disabled PIE uses normal level"), Player->GetPlayerLevel(), 1);
    TestEqual(TEXT("Disabled PIE uses normal points"), Player->GetAbilityPts(), 0);
    Settings->bEnableTestOverrides = true;
    Player->ApplyPIETestOverrides();
    TestEqual(TEXT("PIE receives test level"), Player->GetPlayerLevel(), 25);
    TestEqual(TEXT("XP matches test level"), Data->FindLevelForXP(Player->GetXP()), 25);
    TestEqual(TEXT("Exact skill point budget without intermediate rewards"), Player->GetAbilityPts(), 30);
    TestEqual(TEXT("Attribute budget"), Player->GetAttributePts(), 10);
    TestEqual(TEXT("PIE receives temporary respec gold"), Player->PlayerInventory->GetCurrency(), 500);
    UROverlayWidgetController* HUD = NewObject<UROverlayWidgetController>();
    HUD->SetWidgetControllerParams(FWidgetControllerParams(nullptr, Player, Player->GetAbilitySystemComponent(), Player->GetAttributeSet()));
    UProgressionHUDProbe* Probe = NewObject<UProgressionHUDProbe>();
    HUD->OnLevelChangeDelegate.AddDynamic(Probe, &UProgressionHUDProbe::ReceiveLevel);
    HUD->OnXPPercentChangeDelegate.AddDynamic(Probe, &UProgressionHUDProbe::ReceiveXP);
    HUD->BroadcastInitialValues();
    TestEqual(TEXT("HUD created after overrides receives starting level"), Probe->Level, 25);
    TestEqual(TEXT("HUD receives initial XP progress"), Probe->XPProgress, 0.f);
    Player->AddToAbilityPts(-1);
    Player->PlayerInventory->AddCurrency(-100);
    Player->ApplyPIETestOverrides();
    TestEqual(TEXT("Repeated possession does not refill spent points"), Player->GetAbilityPts(), 29);
    TestEqual(TEXT("Repeated possession does not refill spent gold"), Player->PlayerInventory->GetCurrency(), 400);

    Settings->StartingLevel = 100;
    Settings->StartingSkillPoints = -1;
    ARPlayerState* Capped = PIE->SpawnActor<ARPlayerState>();
    Capped->LevelUpInfo = Data;
    Capped->ApplyPIETestOverrides();
    TestEqual(TEXT("Test level respects character cap"), Capped->GetPlayerLevel(), 40);
    TestEqual(TEXT("Negative points are clamped"), Capped->GetAbilityPts(), 0);
    PIE->DestroyWorld(false);
    Game->DestroyWorld(false);
    return true;
}
#endif
