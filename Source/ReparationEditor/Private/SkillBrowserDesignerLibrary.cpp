#include "CompanionCombatEditorLibrary.h"
#include "UI/GAS/RSkillBrowser.h"
#include "UI/GAS/RSkillMenuStyle.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/PackageName.h"

namespace
{
    UWidgetBlueprint* NewDesignerBlueprint(const TCHAR* Path, UClass* Parent)
    {
        UPackage* Package = CreatePackage(Path);
        UWidgetBlueprint* BP = Cast<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(Parent, Package,
            FName(*FPackageName::GetLongPackageAssetName(Path)), BPTYPE_Normal,
            UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass()));
        return BP;
    }
    template<class T> T* Named(UWidgetTree* Tree, const TCHAR* Name)
    {
        T* Widget = Tree->ConstructWidget<T>(T::StaticClass(), FName(Name));
        Widget->bIsVariable = true;
        return Widget;
    }
    void Font(UTextBlock* Text, int32 Size)
    {
        FSlateFontInfo Info = Text->GetFont(); Info.Size = Size; Text->SetFont(Info);
    }
}

bool UCompanionCombatEditorLibrary::CreateSkillBrowserDesignerAssets()
{
    const TCHAR* ButtonPath = TEXT("/Game/UI/Abilities/UI_SkillBrowserButton");
    const TCHAR* BrowserPath = TEXT("/Game/UI/Abilities/UI_SkillBrowser");
    UWidgetBlueprint* ButtonBP = LoadObject<UWidgetBlueprint>(nullptr, ButtonPath, nullptr, LOAD_NoWarn);
    if (!ButtonBP)
    {
        ButtonBP = NewDesignerBlueprint(ButtonPath, URSkillBrowserButton::StaticClass());
        if (!ButtonBP) return false;
        UButton* Button = Named<UButton>(ButtonBP->WidgetTree, TEXT("Button"));
        Button->SetBackgroundColor(FLinearColor(0.06f, 0.13f, 0.22f, 1.f));
        UTextBlock* Text = Named<UTextBlock>(ButtonBP->WidgetTree, TEXT("LabelText"));
        Text->SetText(FText::FromString(TEXT("Technique")));
        Text->SetMargin(FMargin(14.f, 9.f)); Text->SetAutoWrapText(true); Font(Text, 16);
        Button->AddChild(Text); ButtonBP->WidgetTree->RootWidget = Button;
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(ButtonBP);
        FKismetEditorUtilities::CompileBlueprint(ButtonBP);
        URSkillBrowserButton* Defaults = Cast<URSkillBrowserButton>(ButtonBP->GeneratedClass->GetDefaultObject());
        Defaults->bUseDesignerLayout = true; Defaults->bApplyMenuStyle = false;
        Defaults->Label = FText::FromString(TEXT("Technique"));
        ButtonBP->MarkPackageDirty();
    }
    if (!ButtonBP->GeneratedClass || ButtonBP->Status == BS_Error) return false;
    UWidgetBlueprint* BrowserBP = LoadObject<UWidgetBlueprint>(nullptr, BrowserPath, nullptr, LOAD_NoWarn);
    if (BrowserBP) return BrowserBP->ParentClass->IsChildOf(URSkillBrowser::StaticClass());
    BrowserBP = NewDesignerBlueprint(BrowserPath, URSkillBrowser::StaticClass());
    if (!BrowserBP) return false;
    UWidgetTree* Tree = BrowserBP->WidgetTree;
    auto Control = [&](const TCHAR* Name, const TCHAR* Label, int32 Action, ERSkillDiscipline Discipline = ERSkillDiscipline::Other)
    {
        URSkillBrowserButton* Result = Tree->ConstructWidget<URSkillBrowserButton>(ButtonBP->GeneratedClass.Get(), FName(Name));
        Result->bIsVariable = true; Result->Label = FText::FromString(Label); Result->Action = Action; Result->Discipline = Discipline;
        return Result;
    };
    UBorder* Background = Named<UBorder>(Tree, TEXT("Background"));
    Background->SetBrushColor(FLinearColor(0.015f, 0.028f, 0.055f, 1.f)); Background->SetPadding(FMargin(20.f));
    Tree->RootWidget = Background;
    UVerticalBox* Root = Named<UVerticalBox>(Tree, TEXT("Content")); Background->AddChild(Root);
    UHorizontalBox* Header = Named<UHorizontalBox>(Tree, TEXT("Header"));
    Root->AddChildToVerticalBox(Header)->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
    UTextBlock* Points = Named<UTextBlock>(Tree, TEXT("Points")); Points->SetText(FText::FromString(TEXT("Skill points: 30"))); Font(Points, 20);
    Header->AddChildToHorizontalBox(Points)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UTextBlock* Element = Named<UTextBlock>(Tree, TEXT("MeleeElement")); Element->SetText(FText::FromString(TEXT("Melee: Physical"))); Font(Element, 17);
    Header->AddChildToHorizontalBox(Element);
    UWrapBox* Tabs = Named<UWrapBox>(Tree, TEXT("DisciplineTabs")); Tabs->SetInnerSlotPadding(FVector2D(6.f));
    Root->AddChildToVerticalBox(Tabs)->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
    for (ERSkillDiscipline Discipline : {ERSkillDiscipline::Melee, ERSkillDiscipline::Ranged, ERSkillDiscipline::Mobility, ERSkillDiscipline::Stealth, ERSkillDiscipline::Elemental, ERSkillDiscipline::Other})
    {
        const FString Label = StaticEnum<ERSkillDiscipline>()->GetNameStringByValue(static_cast<int64>(Discipline));
        Tabs->AddChildToWrapBox(Control(*(TEXT("Tab") + Label), *Label, 0, Discipline));
    }
    UHorizontalBox* Body = Named<UHorizontalBox>(Tree, TEXT("Body"));
    Root->AddChildToVerticalBox(Body)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UScrollBox* SkillScroll = Named<UScrollBox>(Tree, TEXT("SkillScroll"));
    FSlateChildSize LeftSize(ESlateSizeRule::Fill); LeftSize.Value = 0.38f;
    UHorizontalBoxSlot* LeftSlot = Body->AddChildToHorizontalBox(SkillScroll); LeftSlot->SetSize(LeftSize); LeftSlot->SetPadding(FMargin(0.f, 0.f, 20.f, 0.f));
    SkillScroll->AddChild(Named<UVerticalBox>(Tree, TEXT("List")));
    UBorder* Panel = Named<UBorder>(Tree, TEXT("DetailsPanel")); Panel->SetBrushColor(FLinearColor(0.025f, 0.05f, 0.09f, 1.f)); Panel->SetPadding(FMargin(18.f));
    FSlateChildSize RightSize(ESlateSizeRule::Fill); RightSize.Value = 0.62f; Body->AddChildToHorizontalBox(Panel)->SetSize(RightSize);
    UVerticalBox* Right = Named<UVerticalBox>(Tree, TEXT("DetailsContent")); Panel->AddChild(Right);
    UScrollBox* DescriptionScroll = Named<UScrollBox>(Tree, TEXT("DescriptionScroll")); Right->AddChildToVerticalBox(DescriptionScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UTextBlock* Details = Named<UTextBlock>(Tree, TEXT("Details")); Details->SetAutoWrapText(true); Font(Details, 17);
    Details->SetText(FText::FromString(TEXT("Select a technique to see its ranks, cost and requirements."))); DescriptionScroll->AddChild(Details);
    Right->AddChildToVerticalBox(Control(TEXT("PurchaseButton"), TEXT("Learn / Upgrade"), 2))->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
    Right->AddChildToVerticalBox(Control(TEXT("ElementButton"), TEXT("Use selected element for melee"), 3))->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
    Right->AddChildToVerticalBox(Control(TEXT("PhysicalButton"), TEXT("Use physical melee"), 5))->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BrowserBP);
    FKismetEditorUtilities::CompileBlueprint(BrowserBP);
    if (!BrowserBP->GeneratedClass || BrowserBP->Status == BS_Error) return false;
    URSkillBrowser* Defaults = Cast<URSkillBrowser>(BrowserBP->GeneratedClass->GetDefaultObject());
    Defaults->bUseDesignerLayout = true; Defaults->SkillButtonClass = ButtonBP->GeneratedClass.Get();
    BrowserBP->MarkPackageDirty();
    return true;
}
