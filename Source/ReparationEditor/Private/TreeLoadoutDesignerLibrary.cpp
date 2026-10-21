#include "CompanionCombatEditorLibrary.h"
#include "UI/GAS/RSkillTree.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintEditorUtils.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/ScrollBox.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Styling/CoreStyle.h"

namespace
{
    template<class T> T* Make(UWidgetTree* Tree, const TCHAR* Name)
    {
        T* Result = Tree->ConstructWidget<T>(T::StaticClass(), FName(Name)); Result->bIsVariable = true;
        if (auto* BP = Tree->GetTypedOuter<UWidgetBlueprint>())
        {
            if (!BP->WidgetVariableNameToGuidMap.Contains(FName(Name))) BP->OnVariableAdded(FName(Name));
        }
        return Result;
    }
    UTextBlock* Heading(UWidgetTree* Tree, const TCHAR* Name, const TCHAR* Value)
    {
        auto* Text = Make<UTextBlock>(Tree, Name); Text->SetText(FText::FromString(Value));
        auto Font = Text->GetFont(); Font.Size = 14; Text->SetFont(Font); Text->SetAutoWrapText(true);
        return Text;
    }
}

bool UCompanionCombatEditorLibrary::ConsolidateSkillTreeMenu()
{
    auto* BP = LoadObject<UWidgetBlueprint>(nullptr,TEXT("/Game/UI/Abilities/UI_SkillTree"));
    auto* Menu = LoadObject<UWidgetBlueprint>(nullptr,TEXT("/Game/UI/Abilities/UI_AbilityMenu"));
    auto* ButtonBP = LoadObject<UWidgetBlueprint>(nullptr,TEXT("/Game/UI/Abilities/UI_SkillBrowserButton"));
    if (!BP || !Menu || !ButtonBP) return false;
    auto* Details = Cast<UVerticalBox>(BP->WidgetTree->FindWidget(TEXT("DetailsContent")));
    auto* Passive = Cast<UVerticalBox>(BP->WidgetTree->FindWidget(TEXT("PassiveColumn")));
    if (!Details || !Passive) return false;
    // Repair attachment as well as existence: stale/detached optional bindings can hide actions.
    auto* Purchase = BP->WidgetTree->FindWidget(TEXT("PurchaseButton"));
    if (!Purchase) return false;
    auto* Equip = Cast<URSkillBrowserButton>(BP->WidgetTree->FindWidget(TEXT("EquipButton")));
    if (!Equip)
    {
        Equip = BP->WidgetTree->ConstructWidget<URSkillBrowserButton>(ButtonBP->GeneratedClass.Get(),TEXT("EquipButton"));
        Equip->bIsVariable = true;
        if (!BP->WidgetVariableNameToGuidMap.Contains(TEXT("EquipButton"))) BP->OnVariableAdded(TEXT("EquipButton"));
    }
    // A full-width vertical action remains readable on narrower screens and long labels.
    Purchase->RemoveFromParent(); Equip->RemoveFromParent();
    Details->AddChildToVerticalBox(Purchase)->SetPadding(FMargin(0,8,0,0));
    Details->AddChildToVerticalBox(Equip)->SetPadding(FMargin(0,8,0,0));
    if (UWidget* Old = BP->WidgetTree->FindWidget(TEXT("PurchaseAndEquip")))
        FWidgetBlueprintEditorUtils::DeleteWidgets(BP,{Old},FWidgetBlueprintEditorUtils::EDeleteWidgetWarningType::DeleteSilently);
    Equip->Action = 7; Equip->Label = FText::FromString(TEXT("Add to Loadout")); Equip->SetVisibility(ESlateVisibility::Visible);
    if (!BP->WidgetTree->FindWidget(TEXT("PassiveChoices")))
    {
        auto* HeadingWidget = Heading(BP->WidgetTree,TEXT("PassiveChoicesHeading"),TEXT("PASSIVE TECHNIQUES"));
        Passive->AddChildToVerticalBox(HeadingWidget)->SetPadding(FMargin(0,14,0,8));
        auto* Scroll = Make<UScrollBox>(BP->WidgetTree,TEXT("PassiveChoicesScroll"));
        Passive->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        Scroll->AddChild(Make<UVerticalBox>(BP->WidgetTree,TEXT("PassiveChoices")));
    }
    if (auto* Width = Cast<USizeBox>(BP->WidgetTree->FindWidget(TEXT("PassiveColumnWidth")))) Width->SetWidthOverride(165);
    // Delete the retired page, including its moved notice, while preserving the outer frame and initialization graph.
    if (UWidget* Old = Menu->WidgetTree->FindWidget(TEXT("SizeBox_root")))
        FWidgetBlueprintEditorUtils::DeleteWidgets(Menu,{Old},FWidgetBlueprintEditorUtils::EDeleteWidgetWarningType::DeleteSilently);
    for (UWidgetBlueprint* Asset : {BP,Menu})
    {
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Asset); FKismetEditorUtilities::CompileBlueprint(Asset);
        if (Asset->Status == BS_Error) return false;
        Asset->MarkPackageDirty();
    }
    return true;
}

bool UCompanionCombatEditorLibrary::AddSkillRespecControl()
{
    auto* BP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_SkillTree"));
    auto* ButtonBP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_SkillBrowserButton"));
    if (!BP || !ButtonBP) return false;
    auto* Details = Cast<UVerticalBox>(BP->WidgetTree->FindWidget(TEXT("DetailsContent")));
    if (!Details) return false;
    if (!BP->WidgetTree->FindWidget(TEXT("RespecButton")))
    {
        auto* Button = BP->WidgetTree->ConstructWidget<URSkillBrowserButton>(ButtonBP->GeneratedClass.Get(), TEXT("RespecButton"));
        Button->bIsVariable = true; Button->Action = 8; Button->Label = FText::FromString(TEXT("Reset selected branch"));
        if (!BP->WidgetVariableNameToGuidMap.Contains(TEXT("RespecButton"))) BP->OnVariableAdded(TEXT("RespecButton"));
        Details->AddChildToVerticalBox(Button)->SetPadding(FMargin(0,8,0,0));
    }
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); FKismetEditorUtilities::CompileBlueprint(BP);
    BP->MarkPackageDirty(); return BP->Status != BS_Error;
}

bool UCompanionCombatEditorLibrary::StyleTreeLoadoutIcons()
{
    auto* TreeBP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_SkillTree"));
    auto* SlotBP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_SkillLoadoutSlot"));
    auto* HUD = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/AbilityGlobes/UI_AbilityGlobe"));
    auto* Equipped = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/AbilityGlobes/UI_EquippedBtn"));
    if (!TreeBP || !SlotBP || !HUD || !Equipped) return false;
    // Apply the requested art migration once; preserve later Designer edits on repeat runs.
    if (!SlotBP->WidgetTree->FindWidget(TEXT("SkillIcon")))
    {
        auto* OriginalButton = Cast<UButton>(Equipped->WidgetTree->FindWidget(TEXT("ButtonRing")));
        if (!OriginalButton) return false;
        SlotBP->WidgetTree->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional);
        SlotBP->WidgetTree = NewObject<UWidgetTree>(SlotBP, TEXT("WidgetTree"), RF_Transactional);
        UWidgetTree* Tree = SlotBP->WidgetTree;
        auto* Root = Make<UVerticalBox>(Tree, TEXT("SlotContent")); Tree->RootWidget = Root;
        auto* Input = Heading(Tree, TEXT("LabelText"), TEXT("LMB")); Input->SetJustification(ETextJustify::Center);
        Root->AddChildToVerticalBox(Input)->SetPadding(FMargin(0,0,0,4));
        auto* Size = Make<USizeBox>(Tree, TEXT("CircleSize")); Size->SetWidthOverride(92); Size->SetHeightOverride(92);
        Root->AddChildToVerticalBox(Size)->SetHorizontalAlignment(HAlign_Center);
        auto* Button = Make<UButton>(Tree, TEXT("Button")); Button->SetStyle(OriginalButton->GetStyle());
        Button->SetBackgroundColor(FLinearColor::White); Size->AddChild(Button);
        auto* Layers = Make<UOverlay>(Tree, TEXT("HudIconLayers")); Button->AddChild(Layers);
        CastChecked<UButtonSlot>(Layers->Slot)->SetPadding(FMargin(0));
        for (const TCHAR* SourceName : {TEXT("AbilityBG"), TEXT("AbilityIcon"), TEXT("Glass"), TEXT("SkillRing")})
        {
            auto* Source = Cast<UImage>(HUD->WidgetTree->FindWidget(FName(SourceName)));
            if (!Source) return false;
            const TCHAR* Name = FCString::Strcmp(SourceName, TEXT("AbilityIcon")) == 0 ? TEXT("SkillIcon") : SourceName;
            auto* Image = Make<UImage>(Tree, Name); Image->SetBrush(Source->GetBrush()); Image->SetColorAndOpacity(Source->GetColorAndOpacity());
            Image->SetVisibility(ESlateVisibility::HitTestInvisible);
            auto* Layer = Layers->AddChildToOverlay(Image);
            if (auto* OriginalSlot = Cast<UOverlaySlot>(Source->Slot))
            {
                Layer->SetPadding(OriginalSlot->GetPadding());
                Layer->SetHorizontalAlignment(OriginalSlot->GetHorizontalAlignment()); Layer->SetVerticalAlignment(OriginalSlot->GetVerticalAlignment());
            }
            if (FCString::Strcmp(SourceName, TEXT("AbilityBG")) == 0)
                if (auto* Brush = FindFProperty<FStructProperty>(HUD->GeneratedClass.Get(), TEXT("AbilityBrush")))
                    if (Brush->Struct == FSlateBrush::StaticStruct()) Image->SetBrush(*Brush->ContainerPtrToValuePtr<FSlateBrush>(HUD->GeneratedClass->GetDefaultObject()));
        }
        auto* Name = Heading(Tree, TEXT("SlotNameText"), TEXT("Empty")); auto Font = Name->GetFont(); Font.Size = 11; Name->SetFont(Font); Name->SetJustification(ETextJustify::Center);
        Root->AddChildToVerticalBox(Name)->SetPadding(FMargin(0,4,0,0));
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(SlotBP); FKismetEditorUtilities::CompileBlueprint(SlotBP);
        if (SlotBP->Status == BS_Error) return false;
        auto* Defaults = CastChecked<URSkillLoadoutSlot>(SlotBP->GeneratedClass->GetDefaultObject());
        Defaults->bUseDesignerLayout = true; Defaults->bApplyMenuStyle = false; Defaults->DesignerSelectedTint = FLinearColor(1.f,.84f,.3f,1);
        SlotBP->MarkPackageDirty();
    }
    TSet<UWidget*> Removed;
    for (const TCHAR* Name : {TEXT("LearnedHeading"), TEXT("LearnedScroll")})
        if (UWidget* Widget = TreeBP->WidgetTree->FindWidget(FName(Name))) Removed.Add(Widget);
    FWidgetBlueprintEditorUtils::DeleteWidgets(TreeBP, Removed, FWidgetBlueprintEditorUtils::EDeleteWidgetWarningType::DeleteSilently);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(TreeBP); FKismetEditorUtilities::CompileBlueprint(TreeBP);
    return TreeBP->Status != BS_Error && SlotBP->Status != BS_Error;
}

bool UCompanionCombatEditorLibrary::SetupTreeLoadout()
{
    auto* BP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_SkillTree"));
    auto* Menu = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_AbilityMenu"));
    auto* ButtonBP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_SkillBrowserButton"));
    if (!BP || !Menu || !ButtonBP) return false;
    auto* Body = Cast<UHorizontalBox>(BP->WidgetTree->FindWidget(TEXT("Body")));
    auto* Details = Cast<UVerticalBox>(BP->WidgetTree->FindWidget(TEXT("DetailsContent")));
    auto* Scroll = Cast<UScrollBox>(BP->WidgetTree->FindWidget(TEXT("DescriptionScroll")));
    if (!Body || !Details || !Scroll) return false;
    // Idempotent: preserve the new Designer layout and any later user styling on subsequent runs.
    if (!BP->WidgetTree->FindWidget(TEXT("ActiveSlots")))
    {
        UWidgetTree* Tree = BP->WidgetTree;
        UWidget* TreeFit = Tree->FindWidget(TEXT("TreeScale"));
        UWidget* DetailPanel = Tree->FindWidget(TEXT("DetailsPanel"));
        if (!TreeFit || !DetailPanel) return false;
        // Retain existing child slot settings, recreating only this horizontal arrangement.
        const FSlateChildSize TreeSize = CastChecked<UHorizontalBoxSlot>(TreeFit->Slot)->GetSize();
        const FSlateChildSize DetailSize = CastChecked<UHorizontalBoxSlot>(DetailPanel->Slot)->GetSize();
        TreeFit->RemoveFromParent(); DetailPanel->RemoveFromParent();
        auto* ActiveWidth = Make<USizeBox>(Tree, TEXT("ActiveColumnWidth")); ActiveWidth->SetWidthOverride(165);
        auto* Active = Make<UVerticalBox>(Tree, TEXT("ActiveColumn")); ActiveWidth->AddChild(Active);
        Active->AddChildToVerticalBox(Heading(Tree, TEXT("ActiveHeading"), TEXT("ACTIVE SKILLS")))->SetPadding(FMargin(0,0,0,8));
        Active->AddChildToVerticalBox(Make<UVerticalBox>(Tree, TEXT("ActiveSlots")));
        Active->AddChildToVerticalBox(Heading(Tree, TEXT("LearnedHeading"), TEXT("LEARNED SKILLS")))->SetPadding(FMargin(0,12,0,8));
        auto* LearnedScroll = Make<UScrollBox>(Tree, TEXT("LearnedScroll"));
        Active->AddChildToVerticalBox(LearnedScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        LearnedScroll->AddChild(Make<UVerticalBox>(Tree, TEXT("LearnedSkills")));
        Body->AddChildToHorizontalBox(ActiveWidth)->SetPadding(FMargin(0,0,12,0));
        Body->AddChildToHorizontalBox(TreeFit)->SetSize(TreeSize);
        auto* PassiveWidth = Make<USizeBox>(Tree, TEXT("PassiveColumnWidth")); PassiveWidth->SetWidthOverride(145);
        auto* Passive = Make<UVerticalBox>(Tree, TEXT("PassiveColumn")); PassiveWidth->AddChild(Passive);
        auto* Label = Heading(Tree, TEXT("PassiveHeading"), TEXT("PASSIVE SKILLS")); Label->SetColorAndOpacity(FLinearColor(.75f,.45f,1.f,1));
        Passive->AddChildToVerticalBox(Label)->SetPadding(FMargin(0,0,0,8));
        Passive->AddChildToVerticalBox(Make<UVerticalBox>(Tree, TEXT("PassiveSlots")));
        Body->AddChildToHorizontalBox(PassiveWidth)->SetPadding(FMargin(12,0,0,0));
        auto* PanelSlot = Body->AddChildToHorizontalBox(DetailPanel); PanelSlot->SetSize(DetailSize); PanelSlot->SetPadding(FMargin(12,0,0,0));
        auto* Actions = Make<UHorizontalBox>(Tree, TEXT("PurchaseAndEquip"));
        UWidget* Purchase = Tree->FindWidget(TEXT("PurchaseButton"));
        const int32 Index = Details->GetChildIndex(Purchase); Purchase->RemoveFromParent();
        Details->InsertChildAt(Index, Actions);
        Actions->AddChildToHorizontalBox(Purchase)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        auto* Equip = Tree->ConstructWidget<URSkillBrowserButton>(ButtonBP->GeneratedClass.Get(), TEXT("EquipButton")); Equip->bIsVariable = true;
        BP->OnVariableAdded(TEXT("EquipButton"));
        Equip->Action = 7; Equip->Label = FText::FromString(TEXT("Add to Loadout"));
        auto* EquipSlot = Actions->AddChildToHorizontalBox(Equip); EquipSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); EquipSlot->SetPadding(FMargin(6,0,0,0));
    }
    // A reusable Blueprint exposes the new slot appearance in Designer.
    auto* SlotBP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_SkillLoadoutSlot"), nullptr, LOAD_NoWarn);
    if (!SlotBP)
    {
        SlotBP = Cast<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(URSkillLoadoutSlot::StaticClass(), CreatePackage(TEXT("/Game/UI/Abilities/UI_SkillLoadoutSlot")),
            TEXT("UI_SkillLoadoutSlot"), BPTYPE_Normal, UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass()));
        SlotBP->WidgetTree = DuplicateObject<UWidgetTree>(ButtonBP->WidgetTree, SlotBP, TEXT("WidgetTree"));
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(SlotBP); FKismetEditorUtilities::CompileBlueprint(SlotBP);
        CastChecked<URSkillLoadoutSlot>(SlotBP->GeneratedClass->GetDefaultObject())->bUseDesignerLayout = true;
        CastChecked<URSkillLoadoutSlot>(SlotBP->GeneratedClass->GetDefaultObject())->bApplyMenuStyle = false;
        SlotBP->MarkPackageDirty();
    }
    // Copy existing ability-description style rows, then add clear section headings.
    auto* Styles = LoadObject<UDataTable>(nullptr, TEXT("/Game/UI/Style/DT_SkillTreeTextStyles"), nullptr, LOAD_NoWarn);
    if (!Styles)
    {
        Styles = NewObject<UDataTable>(CreatePackage(TEXT("/Game/UI/Style/DT_SkillTreeTextStyles")), TEXT("DT_SkillTreeTextStyles"), RF_Public|RF_Standalone);
        Styles->RowStruct = FRichTextStyleRow::StaticStruct();
        auto* Original = Cast<URichTextBlock>(Menu->WidgetTree->FindWidget(TEXT("RT_Description")));
        if (Original && Original->GetTextStyleSet())
            for (const auto& Pair : Original->GetTextStyleSet()->GetRowMap()) Styles->AddRow(Pair.Key, *reinterpret_cast<const FRichTextStyleRow*>(Pair.Value));
        FRichTextStyleRow Base; Base.TextStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14)); Base.TextStyle.SetColorAndOpacity(FLinearColor(.88f,.9f,.94f,1));
        if (!Styles->GetRowMap().Contains(TEXT("Default"))) Styles->AddRow(TEXT("Default"), Base);
        Base.TextStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 17)); Base.TextStyle.SetColorAndOpacity(FLinearColor(1.f,.77f,.35f,1)); Styles->AddRow(TEXT("Title"), Base);
        Styles->MarkPackageDirty();
    }
    if (!BP->WidgetTree->FindWidget(TEXT("RichDetails")))
    {
        if (UWidget* Old = BP->WidgetTree->FindWidget(TEXT("Details"))) Old->SetVisibility(ESlateVisibility::Collapsed); // Retain inherited binding.
        auto* Rich = Make<URichTextBlock>(BP->WidgetTree, TEXT("RichDetails")); Rich->SetTextStyleSet(Styles); Rich->SetAutoWrapText(true); Scroll->AddChild(Rich);
    }
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); FKismetEditorUtilities::CompileBlueprint(BP);
    if (BP->Status == BS_Error || SlotBP->Status == BS_Error) return false;
    CastChecked<URSkillTree>(BP->GeneratedClass->GetDefaultObject())->SlotWidgetClass = SlotBP->GeneratedClass.Get(); BP->MarkPackageDirty();

    // Retire the original choice/equip UI while preserving menu controller initialization and its frame.
    if (Menu->WidgetTree->FindWidget(TEXT("UI_ActiveAbilityTree")))
    {
        TArray<UEdGraph*> Graphs; Menu->GetAllGraphs(Graphs);
        const TSet<FName> Keep{TEXT("K2Node_Event_1"),TEXT("K2Node_CallFunction_2"),TEXT("K2Node_DynamicCast_0"),TEXT("K2Node_VariableSet_0"),
            TEXT("K2Node_VariableGet_2"),TEXT("K2Node_CallFunction_1"),TEXT("K2Node_VariableGet_4"),TEXT("K2Node_CallFunction_4")};
        for (UEdGraph* Graph : Graphs)
        {
            if (Graph->GetFName() == TEXT("EventGraph"))
            {
                for (UEdGraphNode* Node : TArray<TObjectPtr<UEdGraphNode>>(Graph->Nodes)) if (!Keep.Contains(Node->GetFName())) FBlueprintEditorUtils::RemoveNode(Menu, Node, true);
                UEdGraphNode *Event = nullptr, *Cast = nullptr, *SetController = nullptr, *Broadcast = nullptr;
                for (UEdGraphNode* Node : Graph->Nodes)
                {
                    if (Node->GetFName() == TEXT("K2Node_Event_1")) Event = Node;
                    if (Node->GetFName() == TEXT("K2Node_DynamicCast_0")) Cast = Node;
                    if (Node->GetFName() == TEXT("K2Node_CallFunction_1")) SetController = Node;
                    if (Node->GetFName() == TEXT("K2Node_CallFunction_4")) Broadcast = Node;
                }
                if (!Event || !Cast || !SetController || !Broadcast) return false;
                Event->FindPinChecked(UEdGraphSchema_K2::PN_Then)->MakeLinkTo(Cast->FindPinChecked(UEdGraphSchema_K2::PN_Execute));
                SetController->FindPinChecked(UEdGraphSchema_K2::PN_Then)->MakeLinkTo(Broadcast->FindPinChecked(UEdGraphSchema_K2::PN_Execute));
            }
            else if (Menu->FunctionGraphs.Contains(Graph)) FBlueprintEditorUtils::RemoveGraph(Menu, Graph, EGraphRemoveFlags::Recompile);
        }
        if (UWidget* Content = Menu->WidgetTree->FindWidget(TEXT("Overlay_box")))
            FWidgetBlueprintEditorUtils::DeleteWidgets(Menu, {Content}, FWidgetBlueprintEditorUtils::EDeleteWidgetWarningType::DeleteSilently);
        auto* Bounds = Cast<USizeBox>(Menu->WidgetTree->FindWidget(TEXT("SizeBox_root")));
        if (!Bounds) return false;
        Bounds->AddChild(Heading(Menu->WidgetTree, TEXT("LoadoutMovedNotice"), TEXT("Manage active and passive skills beside the skill tree. Select a learned skill, then Add to Loadout and choose a slot.")));
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Menu); FKismetEditorUtilities::CompileBlueprint(Menu);
    }
    return Menu->Status != BS_Error;
}
