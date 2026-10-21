#include "CompanionCombatEditorLibrary.h"
#include "UI/GAS/RSkillTree.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/PackageName.h"

namespace
{
    UWidgetBlueprint* Create(const TCHAR* Path, UClass* Parent)
    {
        return Cast<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(Parent, CreatePackage(Path),
            FName(*FPackageName::GetLongPackageAssetName(Path)), BPTYPE_Normal, UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass()));
    }
    template<class T> T* Widget(UWidgetTree* Tree, const TCHAR* Name)
    {
        T* Result = Tree->ConstructWidget<T>(T::StaticClass(), FName(Name)); Result->bIsVariable = true; return Result;
    }
    UTextBlock* Text(UWidgetTree* Tree, const TCHAR* Name, const TCHAR* Value, int32 Size)
    {
        UTextBlock* Result = Widget<UTextBlock>(Tree, Name); Result->SetText(FText::FromString(Value));
        FSlateFontInfo Font = Result->GetFont(); Font.Size = Size; Result->SetFont(Font);
        Result->SetColorAndOpacity(FLinearColor(.9f, .85f, .7f, 1.f));
        return Result;
    }
}
bool UCompanionCombatEditorLibrary::CreateSkillTreeDesignerAssets()
{
    const TCHAR* NodePath = TEXT("/Game/UI/Abilities/UI_SkillTreeNode");
    UWidgetBlueprint* NodeBP = LoadObject<UWidgetBlueprint>(nullptr, NodePath, nullptr, LOAD_NoWarn);
    if (!NodeBP)
    {
        NodeBP = Create(NodePath, URSkillTreeNode::StaticClass());
        if (!NodeBP) return false;
        UWidgetTree* Tree = NodeBP->WidgetTree;
        UVerticalBox* Root = Widget<UVerticalBox>(Tree, TEXT("NodeContent")); Tree->RootWidget = Root;
        USizeBox* Circle = Widget<USizeBox>(Tree, TEXT("CircleSize")); Circle->SetWidthOverride(74.f); Circle->SetHeightOverride(74.f);
        Root->AddChildToVerticalBox(Circle)->SetHorizontalAlignment(HAlign_Center);
        UButton* Button = Widget<UButton>(Tree, TEXT("Button")); Circle->AddChild(Button);
        FButtonStyle Style = Button->GetStyle();
        auto Brush = [](FSlateBrush& B)
        {
            B.DrawAs = ESlateBrushDrawType::RoundedBox;
            B.TintColor = FLinearColor(.1f, .15f, .19f, 1.f);
            B.OutlineSettings.CornerRadii = FVector4(37.f,37.f,37.f,37.f);
            B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
            B.OutlineSettings.Color = FLinearColor(.86f,.72f,.38f,1.f); B.OutlineSettings.Width = 2.f;
        };
        Brush(Style.Normal); Brush(Style.Hovered); Brush(Style.Pressed); Brush(Style.Disabled);
        Style.Hovered.OutlineSettings.Width = 4.f;
        Style.Pressed.OutlineSettings.Width = 3.f;
        Button->SetStyle(Style); Button->SetBackgroundColor(FLinearColor::White);
        USizeBox* IconSize = Widget<USizeBox>(Tree, TEXT("IconSize")); IconSize->SetWidthOverride(44.f); IconSize->SetHeightOverride(44.f);
        Button->AddChild(IconSize); IconSize->AddChild(Widget<UImage>(Tree, TEXT("SkillIcon")));
        UTextBlock* Label = Text(Tree, TEXT("LabelText"), TEXT("Technique"), 12); Label->SetAutoWrapText(true); Label->SetJustification(ETextJustify::Center);
        Root->AddChildToVerticalBox(Label)->SetPadding(FMargin(0.f,3.f,0.f,0.f));
        UTextBlock* Rank = Text(Tree, TEXT("RankText"), TEXT("0 / 3"), 10); Rank->SetJustification(ETextJustify::Center); Root->AddChildToVerticalBox(Rank);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(NodeBP); FKismetEditorUtilities::CompileBlueprint(NodeBP);
        auto* Defaults = Cast<URSkillTreeNode>(NodeBP->GeneratedClass->GetDefaultObject()); Defaults->bUseDesignerLayout = true; Defaults->bApplyMenuStyle = false;
        NodeBP->MarkPackageDirty();
    }
    if (!NodeBP->GeneratedClass || NodeBP->Status == BS_Error) return false;
    const TCHAR* TreePath = TEXT("/Game/UI/Abilities/UI_SkillTree");
    if (UWidgetBlueprint* Existing = LoadObject<UWidgetBlueprint>(nullptr, TreePath, nullptr, LOAD_NoWarn))
        return Existing->ParentClass->IsChildOf(URSkillTree::StaticClass());
    UWidgetBlueprint* BP = Create(TreePath, URSkillTree::StaticClass());
    if (!BP) return false;
    UWidgetTree* Tree = BP->WidgetTree;
    UBorder* Background = Widget<UBorder>(Tree, TEXT("Background")); Tree->RootWidget = Background;
    Background->SetBrushColor(FLinearColor(.012f,.025f,.035f,.96f)); Background->SetPadding(FMargin(18.f));
    UVerticalBox* Root = Widget<UVerticalBox>(Tree, TEXT("Content")); Background->AddChild(Root);
    UHorizontalBox* Header = Widget<UHorizontalBox>(Tree, TEXT("Header")); Root->AddChildToVerticalBox(Header)->SetPadding(FMargin(0,0,0,10));
    Header->AddChildToHorizontalBox(Text(Tree, TEXT("Points"), TEXT("CHOOSE YOUR PATH"), 19))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    Header->AddChildToHorizontalBox(Text(Tree, TEXT("MeleeElement"), TEXT("Melee: Physical"), 15));
    UHorizontalBox* Body = Widget<UHorizontalBox>(Tree, TEXT("Body")); Root->AddChildToVerticalBox(Body)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UScaleBox* Fit = Widget<UScaleBox>(Tree, TEXT("TreeScale")); Fit->SetStretch(EStretch::ScaleToFit);
    FSlateChildSize Left(ESlateSizeRule::Fill); Left.Value = .74f; Body->AddChildToHorizontalBox(Fit)->SetSize(Left);
    USizeBox* Bounds = Widget<USizeBox>(Tree, TEXT("TreeBounds")); Bounds->SetWidthOverride(1100.f); Bounds->SetHeightOverride(940.f); Fit->AddChild(Bounds);
    UCanvasPanel* Canvas = Widget<UCanvasPanel>(Tree, TEXT("TreeCanvas")); Bounds->AddChild(Canvas);
    UBorder* Panel = Widget<UBorder>(Tree, TEXT("DetailsPanel")); Panel->SetBrushColor(FLinearColor(.025f,.045f,.06f,1.f)); Panel->SetPadding(FMargin(16));
    FSlateChildSize Right(ESlateSizeRule::Fill); Right.Value = .26f;
    auto* DetailsSlot = Body->AddChildToHorizontalBox(Panel); DetailsSlot->SetSize(Right); DetailsSlot->SetPadding(FMargin(14,0,0,0));
    UVerticalBox* DetailsBox = Widget<UVerticalBox>(Tree, TEXT("DetailsContent")); Panel->AddChild(DetailsBox);
    UScrollBox* Scroll = Widget<UScrollBox>(Tree, TEXT("DescriptionScroll")); DetailsBox->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UTextBlock* Details = Text(Tree, TEXT("Details"), TEXT("Select an icon to see its story, ranks and requirements."), 15); Details->SetAutoWrapText(true); Scroll->AddChild(Details);
    UClass* ButtonClass = LoadClass<URSkillBrowserButton>(nullptr, TEXT("/Game/UI/Abilities/UI_SkillBrowserButton.UI_SkillBrowserButton_C"));
    if (!ButtonClass) return false;
    for (const auto& Item : TArray<TPair<FName,int32>>{{TEXT("PurchaseButton"),2},{TEXT("ElementButton"),3},{TEXT("PhysicalButton"),5}})
    {
        auto* Control = Tree->ConstructWidget<URSkillBrowserButton>(ButtonClass, Item.Key); Control->bIsVariable = true; Control->Action = Item.Value;
        Control->Label = FText::FromString(Item.Value == 2 ? TEXT("Learn / Upgrade") : (Item.Value == 3 ? TEXT("Attune melee") : TEXT("Physical melee")));
        DetailsBox->AddChildToVerticalBox(Control)->SetPadding(FMargin(0,8,0,0));
    }
    // Compatibility binding inherited from the optional list browser; the tree uses TreeCanvas.
    UVerticalBox* List = Widget<UVerticalBox>(Tree, TEXT("List")); List->SetVisibility(ESlateVisibility::Collapsed); Root->AddChildToVerticalBox(List);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); FKismetEditorUtilities::CompileBlueprint(BP);
    if (!BP->GeneratedClass || BP->Status == BS_Error) return false;
    auto* Defaults = Cast<URSkillTree>(BP->GeneratedClass->GetDefaultObject()); Defaults->bUseDesignerLayout = true;
    Defaults->NodeClass = NodeBP->GeneratedClass.Get();
    Defaults->LayoutAsset = TSoftObjectPtr<URSkillTreeLayout>(FSoftObjectPath(TEXT("/Game/UI/Style/DA_SkillTreeLayout.DA_SkillTreeLayout")));
    BP->MarkPackageDirty(); return true;
}
