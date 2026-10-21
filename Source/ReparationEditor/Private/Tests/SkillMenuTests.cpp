#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Framework/RPlayerController.h"
#include "Framework/RPlayerState.h"
#include "UI/GAS/RUserWidget.h"
#include "UI/GAS/RSkillBrowser.h"
#include "UI/GAS/RSkillTree.h"
#include "UI/GAS/Controllers/RAbilityMenuController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "GAS/Data/AbilityInfo.h"
#include "GAS/RAbilitySystemComponent.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillMenuViewsTest, "Reparation.Combat.SkillMenuViews",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillMenuViewsTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    World->SetGameInstance(NewObject<UGameInstance>());
    ARPlayerController* PC = World->SpawnActor<ARPlayerController>();
    World->AddController(PC);
    PC->SetPlayer(NewObject<ULocalPlayer>(GEngine));
    ARPlayerState* PS = World->SpawnActor<ARPlayerState>();
    CastChecked<URAbilitySystemComponent>(PS->GetAbilitySystemComponent())->SkillDefinitions = LoadObject<UAbilityInfo>(nullptr, TEXT("/Game/GAS/Data/DA_AbilityInfo.DA_AbilityInfo"));
    UClass* MenuClass = LoadClass<URUserWidget>(nullptr, TEXT("/Game/UI/Abilities/UI_AbilityMenu.UI_AbilityMenu_C"));
    UClass* ControllerClass = LoadClass<URAbilityMenuController>(nullptr, TEXT("/Game/UI/Controllers/BP_AbilityMenuController.BP_AbilityMenuController_C"));
    if (!TestNotNull(TEXT("Saved menu class"), MenuClass) || !TestNotNull(TEXT("Saved controller class"), ControllerClass)) { World->DestroyWorld(false); return false; }
    URAbilityMenuController* Controller = NewObject<URAbilityMenuController>(PC, ControllerClass);
    Controller->SetWidgetControllerParams(FWidgetControllerParams(PC, PS, PS->GetAbilitySystemComponent(), PS->GetAttributeSet()));
    URUserWidget* Menu = CreateWidget<URUserWidget>(PC, MenuClass);
    if (!TestNotNull(TEXT("Saved UMG instance"), Menu)) { World->DestroyWorld(false); return false; }
    // Match the original parent menu's exposed-on-spawn controller assignment.
    FObjectPropertyBase* AuthoredController = FindFProperty<FObjectPropertyBase>(MenuClass, TEXT("AbilityMenuController"));
    if (!TestNotNull(TEXT("Authored controller input"), AuthoredController)) { World->DestroyWorld(false); return false; }
    AuthoredController->SetObjectPropertyValue_InContainer(Menu, Controller);
    Menu->SetWidgetController(Controller);
    const TSharedRef<SWidget> MenuSlate = Menu->TakeWidget(); // Retain the Slate tree for visibility checks.
    UWidget* Legacy = Menu->WidgetTree->FindWidget(TEXT("SizeBox_root"));
    TestNull(TEXT("Retired Loadout page is deleted"), Legacy);
    if (!TestNotNull(TEXT("Embedded browser"), Controller->SkillBrowser.Get())) { World->DestroyWorld(false); return false; }
    TestNull(TEXT("No Loadout navigation remains"), Controller->LoadoutViewButton.Get());
    TestTrue(TEXT("Original frame remains visible"), Menu->WidgetTree->FindWidget(TEXT("Border_Img"))->IsVisible());
    TestNull(TEXT("Old purchasing header deleted"), Menu->WidgetTree->FindWidget(TEXT("TitleBox")));
    TestNull(TEXT("Old next-rank panel deleted"), Menu->WidgetTree->FindWidget(TEXT("RT_NextLevelDescription")));
    TestNull(TEXT("Old equipment picker removed"), Menu->WidgetTree->FindWidget(TEXT("UI_ActiveAbilityTree")));
    TestNull(TEXT("Old equipped slots moved to tree"), Menu->WidgetTree->FindWidget(TEXT("UI_EquippedSkills")));
    TestEqual(TEXT("Browser belongs to the menu"), Controller->SkillBrowser->GetParent(), static_cast<UPanelWidget*>(Controller->SkillMenuCanvas));
    const UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Controller->SkillBrowser->Slot);
    TestEqual(TEXT("Browser stretches with menu width"), Slot->GetAnchors().Maximum.X, 1.0);
    TestEqual(TEXT("Browser stretches with menu height"), Slot->GetAnchors().Maximum.Y, 1.0);
    TestTrue(TEXT("Removed tab bar no longer reserves space"), Slot->GetOffsets().Top < 64.f);
    URSkillBrowser* First = Controller->SkillBrowser;
    TestEqual(TEXT("Editable Blueprint browser is selected"), First->GetClass()->GetName(), FString(TEXT("UI_SkillTree_C")));
    TestTrue(TEXT("Designer layout is active"), First->bUseDesignerLayout);
    TestEqual(TEXT("Designer root was preserved"), First->GetRootWidget()->GetName(), FString(TEXT("Background")));
    URSkillTree* Tree = Cast<URSkillTree>(First);
    if (TestNotNull(TEXT("Icon tree is active"), Tree))
    {
        auto* Active = Cast<UVerticalBox>(Tree->WidgetTree->FindWidget(TEXT("ActiveSlots")));
        auto* Passive = Cast<UVerticalBox>(Tree->WidgetTree->FindWidget(TEXT("PassiveSlots")));
        auto* Choices = Cast<UVerticalBox>(Tree->WidgetTree->FindWidget(TEXT("LearnedSkills")));
        auto* PassiveChoices = Cast<UVerticalBox>(Tree->WidgetTree->FindWidget(TEXT("PassiveChoices")));
        TestTrue(TEXT("Passive choices share a single column"), PassiveChoices && PassiveChoices->GetChildrenCount() >= 2);
        TestTrue(TEXT("Four labeled active inputs on tree"), Active && Active->GetChildrenCount() == 4);
        TestTrue(TEXT("Passive slots on tree"), Passive && Passive->GetChildrenCount() == 4);
        TestNull(TEXT("Separate learned skills list removed"), Choices);
        URAbilitySystemComponent* ASC = Controller->GetSkillASC();
        ASC->InitAbilityActorInfo(PS, PS);
        const FGameplayTag Katana = FGameplayTag::RequestGameplayTag(TEXT("ability.melee"));
        FGameplayAbilitySpec Available(Controller->GetSkillDefinition(FGameplayTag::RequestGameplayTag(TEXT("ability.grenade.stun"))).Ability, 1);
        Available.GetDynamicSpecSourceTags().AddTag(FRGameplayTags::Get().ability_status_available);
        ASC->GiveAbility(Available);
        Tree->NativeTick(FGeometry(), .3f);
        FGameplayAbilitySpec Learned(Controller->GetSkillDefinition(Katana).Ability, 1);
        Learned.GetDynamicSpecSourceTags().AddTag(FRGameplayTags::Get().ability_status_unlocked);
        ASC->GiveAbility(Learned);
        Tree->SelectTreeNode(TEXT("MeleeRoot"));
        Tree->HandleAction(7, FGameplayTag(), ERSkillDiscipline::Other);
        Tree->SelectLoadoutSlot(FRGameplayTags::Get().InputTag_LMB);
        TestEqual(TEXT("Tree assigns learned skill to input"), ASC->GetInputTagFromAbilityTag(Katana), FRGameplayTags::Get().InputTag_LMB);
        auto* IconSlot = Active ? Cast<URSkillLoadoutSlot>(Active->GetChildAt(0)) : nullptr;
        auto* SlotIcon = IconSlot ? Cast<UImage>(IconSlot->WidgetTree->FindWidget(TEXT("SkillIcon"))) : nullptr;
        if (TestNotNull(TEXT("HUD-style skill icon exists"), SlotIcon))
            TestEqual(TEXT("Slot uses the same skill texture as the HUD"), SlotIcon->GetBrush().GetResourceObject(), static_cast<UObject*>(Controller->GetSkillDefinition(Katana).Icon));
        Tree->HandleAction(7, FGameplayTag(), ERSkillDiscipline::Other);
        TestFalse(TEXT("Tree removes assignment without forgetting skill"), ASC->GetInputTagFromAbilityTag(Katana).IsValid());
        TestEqual(TEXT("Removing retains learned rank"), ASC->GetLearnedSkillRank(Katana), 1);
        UCanvasPanel* Nodes = Cast<UCanvasPanel>(Tree->WidgetTree->FindWidget(TEXT("TreeCanvas")));
        TestTrue(TEXT("Tree includes working and planned skills"), Nodes && Nodes->GetChildrenCount() >= 20);
        TSet<FName> Roots;
        if (Nodes) for (UWidget* Child : Nodes->GetAllChildren())
        {
            if (URSkillTreeNode* Node = Cast<URSkillTreeNode>(Child))
            {
                if (Node->NodeId == TEXT("StealthHideinShadows") || Node->NodeId == TEXT("MeleeRoot") || Node->NodeId == TEXT("ArcherRoot")) Roots.Add(Node->NodeId);
                if (Node->NodeId == TEXT("MeleeRoot"))
                {
                    TestTrue(TEXT("Learned tree node remains highlighted after unequip"), Node->bLearned);
                    auto* Rank = Cast<UTextBlock>(Node->WidgetTree->FindWidget(TEXT("RankText")));
                    TestTrue(TEXT("Learned rank has an explicit label"), Rank && Rank->GetText().ToString().Contains(TEXT("LEARNED")));
                    Node->SelectNode();
                    UTextBlock* Description = Cast<UTextBlock>(Tree->WidgetTree->FindWidget(TEXT("Details")));
                    TestTrue(TEXT("Icon selection shows katana requirements"), Description->GetText().ToString().Contains(TEXT("Katana")));
                }
                if (Node->NodeId == TEXT("Grenade")) TestFalse(TEXT("Available but unlearned node has no learned highlight"), Node->bLearned);
                if (Node->NodeId == TEXT("Sneaking"))
                {
                    Node->SelectNode();
                    TestFalse(TEXT("Future techniques cannot be purchased"), Tree->WidgetTree->FindWidget(TEXT("PurchaseButton"))->GetIsEnabled());
                }
            }
        }
        TestEqual(TEXT("Three lower branches are visible"), Roots.Num(), 3);
        const FGameplayTag Counter = FGameplayTag::RequestGameplayTag(TEXT("ability.technique.counterstrike"));
        FGameplayAbilitySpec CounterSpec(Controller->GetSkillDefinition(Counter).Ability,1);
        CounterSpec.GetDynamicSpecSourceTags().AddTag(FRGameplayTags::Get().ability_status_unlocked); ASC->GiveAbility(CounterSpec);
        Tree->SelectTreeNode(TEXT("MeleeCounterstrike"));
        auto* Equip = Cast<URSkillBrowserButton>(Tree->WidgetTree->FindWidget(TEXT("EquipButton")));
        TestTrue(TEXT("Learned counter has a visible usable equip action"), Equip && Equip->IsVisible() && Equip->GetIsEnabled());
        TestEqual(TEXT("Equip action is directly attached to visible details"), Equip ? Equip->GetParent() : nullptr, Cast<UPanelWidget>(Tree->WidgetTree->FindWidget(TEXT("DetailsContent"))));
        if (Equip) Equip->Clicked();
        Tree->SelectLoadoutSlot(FRGameplayTags::Get().InputTag_1);
        TestEqual(TEXT("Actual equip button assigns newly learned counter"), ASC->GetInputTagFromAbilityTag(Counter), FRGameplayTags::Get().InputTag_1);
    }
    PS->AddToAbilityPts(10);
    Controller->AbilitySelected(FGameplayTag::RequestGameplayTag(TEXT("ability.melee")));
    Controller->SpendPointButtonPressed();
    TestEqual(TEXT("Obsolete Loadout handler cannot spend points"), PS->GetAbilityPts(), 10);
    TestNull(TEXT("Obsolete purchase control deleted"), Menu->WidgetTree->FindWidget(TEXT("SpendPointBtn")));
    Controller->OpenSkillBrowser();
    TestEqual(TEXT("Repeated opening reuses one browser"), Controller->SkillBrowser.Get(), First);
    TestEqual(TEXT("Only the skill page layer remains"), Controller->SkillMenuCanvas->GetChildrenCount(), 1);
    Controller->CloseSkillBrowser();
    TestTrue(TEXT("Compatibility close cannot restore a retired page"), First->IsVisible());
    Controller->OpenSkillBrowser();
    Menu->SetWidgetController(Controller);
    TestEqual(TEXT("Repeated setup adds no layers"), Controller->SkillMenuCanvas->GetChildrenCount(), 1);
    World->DestroyWorld(false);
    return true;
}
#endif
