#include "UI/GAS/RSkillBrowser.h"
#include "UI/GAS/Controllers/RAbilityMenuController.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Ability/RElementalMastery.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"

namespace
{
    void StyleText(UTextBlock* Text, const FSlateFontInfo& Font, const FLinearColor& Color)
    {
        Text->SetFont(Font);
        Text->SetColorAndOpacity(FSlateColor(Color));
    }
}

TSharedRef<SWidget> URSkillBrowserButton::RebuildWidget()
{
    if (bUseDesignerLayout) return Super::RebuildWidget();
    Button = WidgetTree->ConstructWidget<UButton>();
    LabelText = WidgetTree->ConstructWidget<UTextBlock>();
    LabelText->SetText(Label);
    LabelText->SetAutoWrapText(true);
    Button->AddChild(LabelText);
    ApplyAppearance();
    Button->OnClicked.AddDynamic(this, &URSkillBrowserButton::Clicked);
    WidgetTree->RootWidget = Button;
    return Super::RebuildWidget();
}
void URSkillBrowserButton::NativePreConstruct()
{
    Super::NativePreConstruct();
    if (bUseDesignerLayout && Button)
    {
        DesignerNormalTint = Button->GetBackgroundColor();
        Button->OnClicked.AddUniqueDynamic(this, &URSkillBrowserButton::Clicked);
    }
    ApplyAppearance();
}
void URSkillBrowserButton::SetSelected(bool bInSelected)
{
    bSelected = bInSelected;
    ApplyAppearance();
    SelectionChanged(bSelected);
}
void URSkillBrowserButton::SetLabelText(FText Text)
{
    Label = Text;
    ApplyAppearance();
}
void URSkillBrowserButton::ApplyAppearance()
{
    if (!Button || !LabelText) return;
    LabelText->SetText(Label);
    if (!bApplyMenuStyle)
    {
        Button->SetBackgroundColor(bSelected ? DesignerSelectedTint : DesignerNormalTint);
        return;
    }
    const URSkillMenuStyle* Theme = MenuStyle ? MenuStyle.Get() : GetDefault<URSkillMenuStyle>();
    const FRSkillMenuButtonAppearance& Appearance = Theme->GetButtonAppearance(StyleRole);
    Button->SetStyle(bSelected ? Appearance.SelectedStyle : Appearance.Style);
    Button->SetBackgroundColor(bSelected ? Appearance.SelectedTint : Appearance.Tint);
    StyleText(LabelText, Appearance.Font, bSelected ? Appearance.SelectedTextColor : Appearance.TextColor);
    LabelText->SetMargin(Appearance.TextPadding);
}
void URSkillBrowserButton::Clicked()
{
    if (Browser) Browser->HandleAction(Action, Skill, Discipline);
    else if (MenuController)
    {
        if (Action == 6) MenuController->CloseSkillBrowser();
        else MenuController->OpenSkillBrowser();
    }
}
URSkillBrowserButton* URSkillBrowser::MakeButton(FText Label, int32 Action, FGameplayTag Skill, ERSkillDiscipline Discipline)
{
    URSkillBrowserButton* Result = CreateWidget<URSkillBrowserButton>(GetOwningPlayer(), SkillButtonClass ? SkillButtonClass.Get() : URSkillBrowserButton::StaticClass());
    Result->Browser = this; Result->Label = Label; Result->Action = Action; Result->Skill = Skill; Result->Discipline = Discipline;
    Result->MenuStyle = MenuStyle;
    Result->StyleRole = Action == 0 ? ERSkillMenuButtonRole::Navigation : (Action == 1 ? ERSkillMenuButtonRole::Skill : ERSkillMenuButtonRole::Action);
    return Result;
}
TSharedRef<SWidget> URSkillBrowser::RebuildWidget()
{
    if (bUseDesignerLayout) return Super::RebuildWidget();
    const URSkillMenuStyle* Theme = MenuStyle ? MenuStyle.Get() : GetDefault<URSkillMenuStyle>();
    UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
    Background->SetBrush(Theme->BackgroundBrush);
    Background->SetBrushColor(Theme->BackgroundTint);
    Background->SetPadding(Theme->ContentPadding);
    UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
    Background->AddChild(Root);
    WidgetTree->RootWidget = Background;

    UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>();
    Root->AddChildToVerticalBox(Header)->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
    Points = WidgetTree->ConstructWidget<UTextBlock>();
    StyleText(Points, Theme->HeaderFont, Theme->HeaderColor);
    Header->AddChildToHorizontalBox(Points)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    MeleeElement = WidgetTree->ConstructWidget<UTextBlock>();
    StyleText(MeleeElement, Theme->BodyFont, Theme->BodyColor);
    Header->AddChildToHorizontalBox(MeleeElement)->SetVerticalAlignment(VAlign_Center);

    UWrapBox* Tabs = WidgetTree->ConstructWidget<UWrapBox>();
    Tabs->SetInnerSlotPadding(FVector2D(Theme->ControlSpacing));
    Root->AddChildToVerticalBox(Tabs)->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
    DisciplineButtons.Reset();
    for (ERSkillDiscipline Discipline : {ERSkillDiscipline::Melee, ERSkillDiscipline::Ranged, ERSkillDiscipline::Mobility, ERSkillDiscipline::Stealth, ERSkillDiscipline::Elemental, ERSkillDiscipline::Other})
    {
        URSkillBrowserButton* Tab = MakeButton(FText::FromString(StaticEnum<ERSkillDiscipline>()->GetNameStringByValue(static_cast<int64>(Discipline))), 0, FGameplayTag(), Discipline);
        Tabs->AddChildToWrapBox(Tab);
        DisciplineButtons.Add(Tab);
    }
    UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>();
    Root->AddChildToVerticalBox(Body)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
    UHorizontalBoxSlot* ListSlot = Body->AddChildToHorizontalBox(Scroll);
    const float ListFraction = FMath::Clamp(Theme->SkillListWidthFraction, 0.2f, 0.6f);
    FSlateChildSize ListSize(ESlateSizeRule::Fill); ListSize.Value = ListFraction;
    ListSlot->SetSize(ListSize);
    ListSlot->SetPadding(FMargin(0.f, 0.f, Theme->ColumnSpacing, 0.f));
    List = WidgetTree->ConstructWidget<UVerticalBox>();
    Scroll->AddChild(List);

    UBorder* DetailPanel = WidgetTree->ConstructWidget<UBorder>();
    DetailPanel->SetBrush(Theme->DetailsBrush);
    DetailPanel->SetBrushColor(Theme->DetailsTint);
    DetailPanel->SetPadding(Theme->DetailsPadding);
    FSlateChildSize DetailSize(ESlateSizeRule::Fill); DetailSize.Value = 1.f - ListFraction;
    Body->AddChildToHorizontalBox(DetailPanel)->SetSize(DetailSize);
    UVerticalBox* Right = WidgetTree->ConstructWidget<UVerticalBox>();
    DetailPanel->AddChild(Right);
    UScrollBox* DetailScroll = WidgetTree->ConstructWidget<UScrollBox>();
    Right->AddChildToVerticalBox(DetailScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    Details = WidgetTree->ConstructWidget<UTextBlock>();
    StyleText(Details, Theme->BodyFont, Theme->BodyColor);
    Details->SetAutoWrapText(true);
    DetailScroll->AddChild(Details);
    PurchaseButton = MakeButton(FText::FromString(TEXT("Learn / Upgrade")), 2);
    Right->AddChildToVerticalBox(PurchaseButton)->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
    ElementButton = MakeButton(FText::FromString(TEXT("Use selected element for melee")), 3);
    Right->AddChildToVerticalBox(ElementButton)->SetPadding(FMargin(0.f, Theme->ControlSpacing, 0.f, 0.f));
    PhysicalButton = MakeButton(FText::FromString(TEXT("Use physical melee")), 5);
    Right->AddChildToVerticalBox(PhysicalButton)->SetPadding(FMargin(0.f, Theme->ControlSpacing, 0.f, 0.f));
    RebuildList(); RefreshDetails();
    return Super::RebuildWidget();
}
void URSkillBrowser::NativeConstruct()
{
    Super::NativeConstruct();
    if (bUseDesignerLayout && WidgetTree)
    {
        DisciplineButtons.Reset();
        WidgetTree->ForEachWidget([this](UWidget* Widget)
        {
            if (URSkillBrowserButton* Control = Cast<URSkillBrowserButton>(Widget))
            {
                Control->Browser = this;
                Control->MenuStyle = MenuStyle;
                if (Control->Action == 0) DisciplineButtons.Add(Control);
            }
        });
    }
    RebuildList();
    RefreshDetails();
}
void URSkillBrowser::HandleAction(int32 Action, FGameplayTag Skill, ERSkillDiscipline Discipline)
{
    if (!Controller) return;
    URAbilitySystemComponent* ASC = Controller->GetSkillASC();
    if (!ASC) return;
    if (Action == 0) { SelectedDiscipline = Discipline; SelectedSkill = FGameplayTag(); RebuildList(); }
    if (Action == 1) { SelectedSkill = Skill; RebuildList(); }
    if (Action == 2 && SelectedSkill.IsValid()) ASC->ServerSpendAbilityPoint(SelectedSkill);
    if (Action == 3 && SelectedSkill.IsValid())
    {
        for (const TCHAR* Type : {TEXT("damage.elemental.fire"), TEXT("damage.elemental.ice"), TEXT("damage.elemental.electric"), TEXT("damage.poison"), TEXT("damage.dark")})
        {
            const FGameplayTag Damage = FGameplayTag::RequestGameplayTag(FName(Type));
            if (URElementalMastery::GetSkillForDamage(Damage) == SelectedSkill) ASC->ServerSelectMeleeElement(Damage);
        }
    }
    if (Action == 4) Controller->CloseSkillBrowser();
    if (Action == 5) ASC->ServerSelectMeleeElement(FGameplayTag());
    RefreshDetails();
}
void URSkillBrowser::RebuildList()
{
    if (!Controller || !List) return;
    for (URSkillBrowserButton* Tab : DisciplineButtons) Tab->SetSelected(Tab->Discipline == SelectedDiscipline);
    List->ClearChildren();
    for (const FRAbilityInfo& Entry : Controller->GetSkillsForDiscipline(SelectedDiscipline))
    {
        URSkillBrowserButton* Row = MakeButton(Entry.DisplayName.IsEmpty() ? FText::FromString(Entry.AbilityTag.ToString()) : Entry.DisplayName, 1, Entry.AbilityTag);
        const URSkillMenuStyle* Theme = MenuStyle ? MenuStyle.Get() : GetDefault<URSkillMenuStyle>();
        List->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 0.f, 0.f, Theme->ControlSpacing));
        Row->SetSelected(Entry.AbilityTag == SelectedSkill);
    }
    if (List->GetChildrenCount() == 0)
    {
        UTextBlock* Empty = WidgetTree->ConstructWidget<UTextBlock>();
        Empty->SetText(FText::FromString(TEXT("No techniques authored yet.")));
        const URSkillMenuStyle* Theme = MenuStyle ? MenuStyle.Get() : GetDefault<URSkillMenuStyle>();
        StyleText(Empty, Theme->BodyFont, Theme->BodyColor);
        List->AddChildToVerticalBox(Empty);
    }
}
void URSkillBrowser::RefreshDetails()
{
    if (!Controller || !Details || !Points || !MeleeElement || !PurchaseButton || !ElementButton || !PhysicalButton || !Controller->GetSkillASC()) return;
    URAbilitySystemComponent* ASC = Controller->GetSkillASC();
    Points->SetText(FText::FromString(FString::Printf(TEXT("Skill points: %d"), Controller->GetSkillPoints())));
    const FGameplayTag CurrentElement = URElementalMastery::GetSkillForDamage(ASC->SelectedMeleeElement);
    const FRAbilityInfo CurrentMastery = Controller->GetSkillDefinition(CurrentElement);
    const FString ElementName = CurrentElement.IsValid() ? CurrentMastery.DisplayName.ToString() : TEXT("Physical");
    MeleeElement->SetText(FText::FromString(TEXT("Melee: ") + ElementName));
    const FRAbilityInfo Entry = Controller->GetSkillDefinition(SelectedSkill);
    PurchaseButton->SetIsEnabled(Entry.AbilityTag.IsValid() && ASC->GetSkillUpgradeIssue(SelectedSkill).IsEmpty());
    const bool bElement = Entry.AbilityTag.IsValid() && Entry.Discipline == ERSkillDiscipline::Elemental;
    ElementButton->SetVisibility(bElement ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    ElementButton->SetIsEnabled(bElement && ASC->GetLearnedSkillRank(SelectedSkill) > 0 && CurrentElement != SelectedSkill);
    PhysicalButton->SetVisibility(SelectedDiscipline == ERSkillDiscipline::Elemental ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    PhysicalButton->SetIsEnabled(ASC->SelectedMeleeElement.IsValid());
    if (!Entry.AbilityTag.IsValid()) { Details->SetText(FText::FromString(TEXT("Select a technique to see its ranks, cost and requirements."))); return; }
    FString Current, Next;
    ASC->BuildSkillDescriptions(SelectedSkill, Current, Next);
    if (CurrentElement == SelectedSkill) Current += TEXT("\nSelected for melee.");
    Details->SetText(FText::FromString(Current + TEXT("\n\n") + Next));
}
void URSkillBrowser::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    RefreshTime += DeltaTime;
    if (RefreshTime >= 0.25f) { RefreshTime = 0.f; RefreshDetails(); }
}
