#include "UI/GAS/RSkillTree.h"
#include "UI/GAS/Controllers/RAbilityMenuController.h"
#include "GAS/RAbilitySystemComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/RichTextBlock.h"
#include "Misc/ScopeExit.h"
#include "Rendering/DrawElements.h"

FLinearColor URSkillTreeLayout::GetBranchColor(ERSkillTreeBranch Branch) const
{
    switch (Branch)
    {
        case ERSkillTreeBranch::Stealth: return StealthColor;
        case ERSkillTreeBranch::Melee: return MeleeColor;
        case ERSkillTreeBranch::Archer: return ArcherColor;
        default: return SharedColor;
    }
}

void URSkillLoadoutSlot::NativePreConstruct()
{
    Super::NativePreConstruct();
    if (Button) { Button->OnClicked.RemoveDynamic(this, &URSkillBrowserButton::Clicked); Button->OnClicked.AddUniqueDynamic(this, &URSkillLoadoutSlot::SelectSlot); }
}
void URSkillLoadoutSlot::SelectSlot()
{
    if (URSkillTree* Tree = Cast<URSkillTree>(Browser)) Tree->SelectLoadoutSlot(SlotTag);
}
void URSkillLoadoutSlot::UpdateSlotVisual(UTexture2D* Icon, UMaterialInterface* Background, FText Name, bool bUnlocked, FLinearColor Accent)
{
    if (SkillIcon)
    {
        SkillIcon->SetBrushFromTexture(Icon);
        SkillIcon->SetVisibility(Icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
    if (AbilityBG)
    {
        if (!bCapturedBackground) { EmptyBackground = AbilityBG->GetBrush(); bCapturedBackground = true; }
        if (Background && Icon) AbilityBG->SetBrushFromMaterial(Background);
        else AbilityBG->SetBrush(EmptyBackground);
        AbilityBG->SetOpacity(bUnlocked ? 1.f : .3f);
    }
    if (SkillRing) SkillRing->SetColorAndOpacity(Accent);
    if (SlotNameText) SlotNameText->SetText(Name);
}

void URSkillTreeNode::NativePreConstruct()
{
    Super::NativePreConstruct();
    if (Button)
    {
        Button->OnClicked.RemoveDynamic(this, &URSkillBrowserButton::Clicked);
        Button->OnClicked.AddUniqueDynamic(this, &URSkillTreeNode::SelectNode);
    }
}
void URSkillTreeNode::SelectNode()
{
    if (URSkillTree* Tree = Cast<URSkillTree>(Browser)) Tree->SelectTreeNode(NodeId);
}
FVector2D URSkillTreeNode::GetConnectionPoint() const
{
    const FGeometry& IconGeometry = Button ? Button->GetCachedGeometry() : GetCachedGeometry();
    return IconGeometry.LocalToAbsolute(IconGeometry.GetLocalSize() * .5f);
}
void URSkillTreeNode::UpdateVisual(UTexture2D* Icon, FText Rank, FLinearColor Color, bool bIsSelected)
{
    bHasIcon = Icon != nullptr;
    SetSelected(bIsSelected);
    if (SkillIcon)
    {
        SkillIcon->SetBrushFromTexture(Icon);
        SkillIcon->SetVisibility(Icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        SkillIcon->SetOpacity(bLearned ? 1.f : (bAvailable ? .65f : .3f));
    }
    if (RankText) { RankText->SetText(Rank); RankText->SetColorAndOpacity(bLearned ? LearnedHighlight : FLinearColor(.7f,.73f,.78f,1)); }
    if (Button) Button->SetBackgroundColor(bIsSelected ? FLinearColor(1.f, .94f, .65f, 1.f) : Color);
}
int32 URSkillTreeNode::NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& Culling,
    FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool bEnabled) const
{
    const int32 Result = Super::NativePaint(Args, Geometry, Culling, Elements, Layer, Style, bEnabled);
    if (!Button) return Result;
    const FGeometry& Circle = Button->GetCachedGeometry();
    if (bLearned)
    {
        TArray<FVector2D> Halo;
        for (int32 Step = 0; Step <= 64; ++Step)
        {
            const float Angle = Step * 2.f * PI / 64.f;
            const FVector2D Point = FVector2D(.5f,.5f) + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * .55f;
            Halo.Add(Geometry.AbsoluteToLocal(Circle.LocalToAbsolute(Point * Circle.GetLocalSize())));
        }
        FSlateDrawElement::MakeLines(Elements, Result + 1, Geometry.ToPaintGeometry(), Halo, ESlateDrawEffect::None, LearnedHighlight, true, LearnedRingWidth);
    }
    if (bHasIcon) return Result + (bLearned ? 1 : 0);
    auto Stroke = [&](std::initializer_list<FVector2D> Points)
    {
        TArray<FVector2D> Local;
        for (FVector2D Point : Points) Local.Add(Geometry.AbsoluteToLocal(Circle.LocalToAbsolute(Point * Circle.GetLocalSize())));
        FSlateDrawElement::MakeLines(Elements, Result + 1, Geometry.ToPaintGeometry(), Local, ESlateDrawEffect::None, FLinearColor(.95f,.9f,.72f,1), true, 2.f);
    };
    // Small vector emblems keep unillustrated future nodes readable; authored textures take precedence.
    if (NodeId == TEXT("Master"))
    {
        Stroke({{.22,.35},{.32,.7},{.68,.7},{.78,.35},{.6,.5},{.5,.25},{.4,.5},{.22,.35}});
        Stroke({{.32,.78},{.68,.78}});
    }
    else if (Branch == ERSkillTreeBranch::Stealth)
    {
        Stroke({{.2,.5},{.35,.35},{.5,.3},{.65,.35},{.8,.5},{.65,.65},{.5,.7},{.35,.65},{.2,.5}});
        Stroke({{.5,.4},{.6,.5},{.5,.6},{.4,.5},{.5,.4}});
    }
    else if (Branch == ERSkillTreeBranch::Archer)
    {
        Stroke({{.35,.2},{.65,.5},{.35,.8},{.35,.2}});
        Stroke({{.2,.5},{.8,.5},{.7,.4}}); Stroke({{.8,.5},{.7,.6}});
    }
    else if (Branch == ERSkillTreeBranch::Melee)
    {
        Stroke({{.3,.75},{.7,.25},{.72,.42},{.42,.7}}); Stroke({{.25,.55},{.5,.8}});
    }
    else if (NodeId == TEXT("Poison"))
    {
        Stroke({{.4,.2},{.6,.2},{.6,.4},{.78,.7},{.7,.8},{.3,.8},{.22,.7},{.4,.4},{.4,.2}});
        Stroke({{.3,.6},{.7,.6}});
    }
    else if (NodeId == TEXT("Dark"))
        Stroke({{.65,.2},{.4,.25},{.25,.5},{.4,.75},{.65,.8},{.5,.6},{.5,.4},{.65,.2}});
    else Stroke({{.6,.2},{.3,.55},{.5,.55},{.4,.8},{.7,.4},{.5,.4},{.6,.2}});
    return Result + 1;
}
void URSkillTree::RebuildList()
{
    if (!Controller || !TreeCanvas) return;
    Layout = LayoutAsset.LoadSynchronous();
    if (!Layout) return;
    if (USizeBox* Bounds = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("TreeBounds"))))
    {
        Bounds->SetWidthOverride(Layout->CanvasSize.X);
        Bounds->SetHeightOverride(Layout->CanvasSize.Y);
    }
    TreeCanvas->ClearChildren(); TreeNodes.Reset();
    if (PassiveChoices) PassiveChoices->ClearChildren();
    for (ERSkillTreeBranch Branch : {ERSkillTreeBranch::Stealth,ERSkillTreeBranch::Melee,ERSkillTreeBranch::Archer})
        for (int32 Tier=1; Tier<=5; ++Tier)
        {
            FVector2D Center = FVector2D::ZeroVector; int32 Count=0; FText TierName;
            for (const FRSkillTreeEntry& Entry : Layout->Nodes) if (Entry.Branch == Branch && Entry.Tier == Tier
                && !(PassiveChoices && (Entry.bPassive || Controller->GetSkillDefinition(Entry.Skill).AbilityType == FRGameplayTags::Get().ability_type_passive)))
            { Center += Entry.Position; ++Count; TierName = Entry.TierName; }
            if (!Count) continue;
            Center /= Count;
            auto* Heading = WidgetTree->ConstructWidget<UTextBlock>();
            const TCHAR* BranchName = Branch == ERSkillTreeBranch::Stealth ? TEXT("STEALTH") : Branch == ERSkillTreeBranch::Melee ? TEXT("MELEE") : TEXT("RANGED");
            Heading->SetText(FText::FromString(FString::Printf(TEXT("%s / %s / %d POINT(S)"), BranchName, *TierName.ToString().ToUpper(), Tier)));
            Heading->SetJustification(ETextJustify::Center); Heading->SetColorAndOpacity(Layout->GetBranchColor(Branch));
            auto Font = Heading->GetFont(); Font.Size=16; Heading->SetFont(Font);
            Heading->SetVisibility(ESlateVisibility::HitTestInvisible);
            auto* HeadingSlot = TreeCanvas->AddChildToCanvas(Heading);
            HeadingSlot->SetPosition(Center + FVector2D(Layout->NodeSize.X*.5f-240,-32)); HeadingSlot->SetSize(FVector2D(480,28));
        }
    for (const FRSkillTreeEntry& Entry : Layout->Nodes)
    {
        URSkillTreeNode* Node = CreateWidget<URSkillTreeNode>(GetOwningPlayer(), NodeClass ? NodeClass.Get() : URSkillTreeNode::StaticClass());
        Node->Browser = this; Node->NodeId = Entry.Id; Node->Skill = Entry.Skill; Node->Label = Entry.Title;
        Node->bPlanned = Entry.bPlanned; Node->Branch = Entry.Branch;
        Node->LearnedHighlight = Layout->LearnedColor; Node->LearnedRingWidth = Layout->LearnedRingWidth;
        Node->bApplyMenuStyle = false;
        const FRAbilityInfo Info = Controller->GetSkillDefinition(Entry.Skill);
        Node->bPassive = Entry.bPassive || Info.AbilityType == FRGameplayTags::Get().ability_type_passive;
        if (Node->bPassive && PassiveChoices)
            PassiveChoices->AddChildToVerticalBox(Node)->SetPadding(FMargin(0,0,0,10));
        else
        {
            UCanvasPanelSlot* NodeSlot = TreeCanvas->AddChildToCanvas(Node);
            NodeSlot->SetPosition(Entry.Position); NodeSlot->SetSize(Layout->NodeSize); NodeSlot->SetZOrder(1);
        }
        Node->SetRenderScale(FVector2D(FMath::Clamp(Entry.VisualScale * (Node->bPassive ? Layout->PassiveScale : 1.f), .5f, 2.5f)));
        TreeNodes.Add(Node);
    }
}
void URSkillTree::SelectTreeNode(FName Id)
{
    if (!Layout) return;
    if (const FRSkillTreeEntry* Node = Layout->Nodes.FindByPredicate([Id](const FRSkillTreeEntry& Entry) { return Entry.Id == Id; }))
    {
        bChoosingSlot = false; bConfirmRespec = false; SelectedNode = Id; SelectedSkill = Node->Skill;
        RefreshDetails();
    }
}
void URSkillTree::RefreshDetails()
{
    Super::RefreshDetails();
    if (!Layout || !Controller || !Controller->GetSkillASC()) return;
    URAbilitySystemComponent* ASC = Controller->GetSkillASC();
    ON_SCOPE_EXIT
    {
        if (RichDetails) RichDetails->SetText(Details->GetText());
        RefreshLoadout();
    };
    Points->SetText(FText::FromString(FString::Printf(TEXT("%d POINTS  /  %d INVESTED"), Controller->GetSkillPoints(), ASC->CoreSkillPointsSpent)));
    for (URSkillTreeNode* Node : TreeNodes)
    {
        const FRSkillTreeEntry* Entry = Layout->Nodes.FindByPredicate([Node](const FRSkillTreeEntry& Item) { return Item.Id == Node->NodeId; });
        if (!Entry) continue;
        const FRAbilityInfo Info = Controller->GetSkillDefinition(Entry->Skill);
        const int32 Rank = ASC->GetLearnedSkillRank(Entry->Skill);
        Node->bLearned = Rank > 0;
        Node->bAvailable = !Entry->bPlanned && Entry->Skill.IsValid() && ASC->GetSkillUpgradeIssue(Entry->Skill).IsEmpty();
        const FLinearColor Base = Node->bPassive ? Layout->PassiveColor : Layout->GetBranchColor(Entry->Branch);
        const FLinearColor Color = Base * (Node->bLearned ? 1.f : (Node->bAvailable ? .72f : .38f));
        const FString Status = Entry->bJunction ? FString::Printf(TEXT("%d INVESTED"), ASC->CoreSkillPointsSpent)
            : (Entry->bPlanned ? TEXT("PLANNED") : FString::Printf(TEXT("%s%s %d / %d"), Node->bLearned ? TEXT("LEARNED\n") : TEXT(""), Node->bPassive ? TEXT("PASSIVE") : TEXT("RANK"), Rank, FMath::Max(1, Info.MaximumAbilityLevel)));
        Node->UpdateVisual(Entry->Icon ? Entry->Icon.Get() : Info.Icon, FText::FromString(Status), Color, SelectedNode == Entry->Id);
        Node->SetToolTipText(FText::FromString(Entry->Title.ToString() + (Entry->Tier > 0 ? FString::Printf(TEXT("\n%s | %d point(s)"), *Entry->TierName.ToString(), Entry->PointCost) : TEXT(""))));
    }
    if (SelectedNode.IsNone() && !SelectedSkill.IsValid())
    {
        Details->SetText(FText::FromString(TEXT("CHOOSE YOUR PATH\n\nStealth, Melee or Archer. Invest in any branch to reach the shared disciplines above.\n\nSelect an icon to inspect ranks and requirements. Grey nodes remain visible so you can plan ahead.\n\nPoison comes first. Advanced Projectiles and Dark require their own teaching quests.")));
        PurchaseButton->SetIsEnabled(false);
        return;
    }
    const FRSkillTreeEntry* Entry = Layout->Nodes.FindByPredicate([this](const FRSkillTreeEntry& Item) { return Item.Id == SelectedNode; });
    if (!Entry)
    {
        FString Current, Next;
        ASC->GetDescriptionByAbilityTag(SelectedSkill, Current, Next);
        Details->SetText(FText::FromString(Current + TEXT("\n\n<Title>Next Upgrade</>\n") + Next));
        return;
    }
    const FRAbilityInfo Info = Controller->GetSkillDefinition(Entry->Skill);
    if (Entry->bJunction)
    {
        Details->SetText(FText::FromString(Entry->Title.ToString() + TEXT("\n\n") + Entry->Description.ToString()
            + FString::Printf(TEXT("\n\n%d points invested across the lower branches.\n\nSelect Poison, Advanced Projectiles or Dark to see its investment and quest requirements."), ASC->CoreSkillPointsSpent)));
        PurchaseButton->SetIsEnabled(false);
        ElementButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    else if (const UAbilityInfo* RewardData = ASC->GetSkillData(); RewardData && Entry->Skill.IsValid() && Entry->Skill == RewardData->AutomaticTierReward)
    {
        Details->SetText(FText::FromString(Entry->Title.ToString() + TEXT("\n\n") + Entry->Description.ToString()
            + FString::Printf(TEXT("\n\nAutomatically granted after learning every tier-%d skill in %s. No skill point or active slot is needed.\n%s"),RewardData->RewardTier,
                RewardData->bRewardRequiresAllBranches ? TEXT("all three branches") : TEXT("any one branch"),
                ASC->GetLearnedSkillRank(Entry->Skill)>0 ? TEXT("UNLOCKED") : TEXT("Not yet earned. Planned tier skills must be implemented and learned too."))));
        PurchaseButton->SetIsEnabled(false);
        ElementButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    else if (Entry->bPlanned || !Info.Ability)
    {
        Details->SetText(FText::FromString(Entry->Title.ToString() + TEXT("\n\n") + Entry->Description.ToString()
            + (Entry->Tier > 0 ? FString::Printf(TEXT("\n\n%s / Tier %d\nPlanned cost: %d point(s).\n%s"), *Entry->TierName.ToString(), Entry->Tier, Entry->PointCost,
                Entry->Tier > 1 ? TEXT("Learn a skill in the previous tier to unlock this tier.") : TEXT("Foundation tier.")) : TEXT(""))
            + TEXT("\n\nFuture technique. Not available to purchase yet.")));
        PurchaseButton->SetIsEnabled(false);
        ElementButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        FString Current, Next;
        ASC->GetDescriptionByAbilityTag(SelectedSkill, Current, Next);
        FString Text = TEXT("<Title>Current Skill</>\n") + Current + TEXT("\n\n<Title>Next Upgrade</>\n") + Next;
        // Older authored descriptions may contain escaped newlines.
        Text.ReplaceInline(TEXT("\\n"), TEXT("\n"));
        Text += FString::Printf(TEXT("\n\n<Title>Requirements</>\nCharacter level <Level>%d</>"), Info.LevelRequirement);
        if (ASC->GetLearnedSkillRank(SelectedSkill)>0 && Info.AbilityType == FGameplayTag::RequestGameplayTag(TEXT("ability.type.technique")))
            Text += TEXT("\n\n<Title>Already in use</>\nThis technique improves an existing action or uses its fixed control. It does not need an active loadout slot.");
        if (Info.BranchTier > 0) Text += FString::Printf(TEXT("\n%s / Tier %d / %d point(s) per rank\n%s"), *Entry->TierName.ToString(), Info.BranchTier, FMath::Clamp(Info.SkillPointCost,1,5),
            Info.BranchTier > 1 ? TEXT("Requires learned skills in the preceding branch tier.") : TEXT("Foundation tier."));
        if (Info.MinimumCorePointsSpent > 0) Text += FString::Printf(TEXT("\n%d points invested across any branches (%d / %d)"), Info.MinimumCorePointsSpent, ASC->CoreSkillPointsSpent, Info.MinimumCorePointsSpent);
        for (const FRSkillPrerequisite& Required : Info.Prerequisites)
            Text += FString::Printf(TEXT("\n%s rank %d"), *Controller->GetSkillDefinition(Required.AbilityTag).DisplayName.ToString(), Required.RequiredRank);
        if (!Info.RequiredProgressionUnlock.IsNone())
        {
            if (const UAbilityInfo* Data = ASC->GetSkillData()) for (const FRSkillQuestUnlock& Quest : Data->QuestUnlocks)
                if (Quest.Id == Info.RequiredProgressionUnlock)
                    Text += FString::Printf(TEXT("\n%s: %s\nQuest offered at level %d OR %d branch points invested."), *Quest.DisplayName.ToString(),
                        ASC->ProgressionUnlocks.Contains(Quest.Id) ? TEXT("completed") : (ASC->IsSkillQuestEligible(Quest.Id) ? TEXT("eligible; quest content coming later") : TEXT("not yet eligible")), Quest.OfferAtLevel, Quest.OfferAtCorePointsSpent);
        }
        Details->SetText(FText::FromString(Text));
    }
    const bool bElement = Entry->Skill.IsValid() && Info.Discipline == ERSkillDiscipline::Elemental;
    PhysicalButton->SetVisibility(bElement ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
int32 URSkillTree::NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& Culling,
    FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool bEnabled) const
{
    const int32 Result = Super::NativePaint(Args, Geometry, Culling, Elements, Layer, Style, bEnabled);
    if (!Layout || !TreeCanvas) return Result;
    for (const FRSkillTreeEntry& Entry : Layout->Nodes)
        for (FName Parent : Entry.Connections)
            if (const FRSkillTreeEntry* Other = Layout->Nodes.FindByPredicate([Parent](const FRSkillTreeEntry& Node) { return Node.Id == Parent; }))
            {
                // Passive choices live in their own column; never draw tree lines across the UI to them.
                auto IsCanvasNode = [this](FName Id)
                {
                    const auto* Found = TreeNodes.FindByPredicate([Id](const URSkillTreeNode* Node) { return Node->NodeId == Id; });
                    return Found && (*Found)->GetParent() == TreeCanvas;
                };
                if (!IsCanvasNode(Entry.Id) || !IsCanvasNode(Other->Id)) continue;
                auto Point = [&](FName Id)
                {
                    const auto* Node = TreeNodes.FindByPredicate([Id](const URSkillTreeNode* Item) { return Item->NodeId == Id; });
                    return Node ? Geometry.AbsoluteToLocal((*Node)->GetConnectionPoint()) : FVector2D::ZeroVector;
                };
                const auto* Learned = TreeNodes.FindByPredicate([&](const URSkillTreeNode* Node) { return Node->NodeId == Entry.Id; });
                const FLinearColor Color = Layout->GetBranchColor(Entry.Branch) * (Learned && (*Learned)->bLearned ? .85f : .35f);
                const TArray<FVector2D> Line{Point(Other->Id), Point(Entry.Id)};
                FSlateDrawElement::MakeLines(Elements, Layer + 1, Geometry.ToPaintGeometry(), Line, ESlateDrawEffect::None, Color, true, Layout->ConnectionThickness);
            }
    return FMath::Max(Result, Layer + 1);
}

void URSkillTree::SelectLearnedSkill(FGameplayTag Skill)
{
    if (!Controller || Controller->GetSkillASC()->GetLearnedSkillRank(Skill) <= 0) return;
    SelectedSkill = Skill; SelectedNode = NAME_None; bChoosingSlot = false;
    RefreshDetails();
}

void URSkillTree::HandleAction(int32 Action, FGameplayTag Skill, ERSkillDiscipline Discipline)
{
    if (Action == 8)
    {
        if (!Controller || !Layout) return;
        const FRSkillTreeEntry* Entry = Layout->Nodes.FindByPredicate([this](const FRSkillTreeEntry& E) { return E.Id == SelectedNode; });
        const ERSkillDiscipline Branch = Entry ? (Entry->Branch == ERSkillTreeBranch::Melee ? ERSkillDiscipline::Melee : Entry->Branch == ERSkillTreeBranch::Stealth ? ERSkillDiscipline::Stealth : Entry->Branch == ERSkillTreeBranch::Archer ? ERSkillDiscipline::Ranged : Controller->GetSkillDefinition(Entry->Skill).Discipline) : Controller->GetSkillDefinition(SelectedSkill).Discipline;
        if (!Controller->GetSkillASC()->GetBranchRespecIssue(Branch).IsEmpty()) return;
        if (bConfirmRespec) { bConfirmRespec = false; Controller->GetSkillASC()->ServerRespecBranch(Branch); }
        else bConfirmRespec = true;
        RefreshDetails(); return;
    }
    if (Action == 1) { SelectLearnedSkill(Skill); return; }
    if (Action != 7) { Super::HandleAction(Action, Skill, Discipline); return; }
    if (!Controller || !SelectedSkill.IsValid()) return;
    URAbilitySystemComponent* ASC = Controller->GetSkillASC();
    if (ASC->GetLearnedSkillRank(SelectedSkill) <= 0) return;
    if (ASC->GetInputTagFromAbilityTag(SelectedSkill).IsValid()) ASC->ServerUnequipAbility(SelectedSkill);
    else bChoosingSlot = !bChoosingSlot;
    RefreshDetails();
}

void URSkillTree::SelectLoadoutSlot(FGameplayTag SlotTag)
{
    if (!Controller) return;
    URAbilitySystemComponent* ASC = Controller->GetSkillASC();
    if (bChoosingSlot)
    {
        if (!ASC->GetEquipIssue(SelectedSkill, SlotTag).IsEmpty()) return;
        ASC->ServerEquipAbility(SelectedSkill, SlotTag); bChoosingSlot = false;
    }
    else if (const FGameplayAbilitySpec* Spec = ASC->GetSpecWithInputSlot(SlotTag)) SelectLearnedSkill(ASC->GetAbilityTagFromSpec(*Spec));
    RefreshDetails();
}

void URSkillTree::RefreshLoadout()
{
    if (!Controller || !Layout) return;
    URAbilitySystemComponent* ASC = Controller->GetSkillASC();
    const auto& T = FRGameplayTags::Get();
    if (RespecButton)
    {
        const FRSkillTreeEntry* Entry = Layout->Nodes.FindByPredicate([this](const FRSkillTreeEntry& E) { return E.Id == SelectedNode; });
        const ERSkillDiscipline Branch = Entry ? (Entry->Branch == ERSkillTreeBranch::Melee ? ERSkillDiscipline::Melee : Entry->Branch == ERSkillTreeBranch::Stealth ? ERSkillDiscipline::Stealth : Entry->Branch == ERSkillTreeBranch::Archer ? ERSkillDiscipline::Ranged : Controller->GetSkillDefinition(Entry->Skill).Discipline) : Controller->GetSkillDefinition(SelectedSkill).Discipline;
        const UAbilityInfo* Data = ASC->GetSkillData();
        const FText Issue = ASC->GetBranchRespecIssue(Branch);
        RespecButton->Browser = this; RespecButton->Action = 8;
        RespecButton->SetIsEnabled(Issue.IsEmpty());
        const FString Fee = !Data ? TEXT("") : Data->RespecPayment == ERRespecPayment::QuestItem ? TEXT("1 quest item") : FString::Printf(TEXT("%d %s"), FMath::Max(1,Data->RespecCost), Data->RespecPayment == ERRespecPayment::Gold ? TEXT("gold") : TEXT("skill point(s)"));
        RespecButton->SetLabelText(FText::FromString(FString::Printf(TEXT("%s branch | Refund %d | Cost %s"), bConfirmRespec ? TEXT("CONFIRM reset") : TEXT("Reset"), ASC->GetBranchRefund(Branch), *Fee)));
        RespecButton->SetToolTipText(Issue.IsEmpty() ? FText::FromString(TEXT("Reset purchased ranks in the selected branch. Starting/free ranks and learned shared knowledge remain. Click again to confirm. Changing selection cancels.")) : Issue);
    }
    if (LoadoutSlots.IsEmpty() && ActiveSlots && PassiveSlots)
    {
        const TArray<FGameplayTag> Slots{T.InputTag_LMB,T.InputTag_RMB,T.InputTag_1,T.InputTag_2,T.InputTag_Passive01,T.InputTag_Passive02,T.InputTag_Passive03,T.InputTag_Passive04};
        for (int32 Index = 0; Index < Slots.Num(); ++Index)
        {
            auto* Button = CreateWidget<URSkillLoadoutSlot>(GetOwningPlayer(), SlotWidgetClass ? SlotWidgetClass.Get() : URSkillLoadoutSlot::StaticClass());
            Button->Browser = this; Button->SlotTag = Slots[Index]; Button->MenuStyle = MenuStyle;
            Button->StyleRole = ERSkillMenuButtonRole::Skill;
            (Index < 4 ? ActiveSlots : PassiveSlots)->AddChildToVerticalBox(Button)->SetPadding(FMargin(0,0,0,8));
            LoadoutSlots.Add(Button);
        }
    }
    for (int32 Index = 0; Index < LoadoutSlots.Num(); ++Index)
    {
        URSkillLoadoutSlot* Button = LoadoutSlots[Index];
        const int32 UnlockLevel = Index < 4 ? 1 : ASC->GetPassiveSlotUnlockLevel(Index - 4);
        Button->SetVisibility(UnlockLevel > 40 ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
        const bool bUnlocked = ASC->IsLoadoutSlotUnlocked(Button->SlotTag);
        const FGameplayAbilitySpec* Spec = ASC->GetSpecWithInputSlot(Button->SlotTag);
        const FGameplayTag Skill = Spec ? ASC->GetAbilityTagFromSpec(*Spec) : FGameplayTag();
        const FString Name = Skill.IsValid() ? Controller->GetSkillDefinition(Skill).DisplayName.ToString() : TEXT("Empty");
        const FString Input = Index == 0 ? TEXT("LMB") : Index == 1 ? TEXT("RMB") : Index == 2 ? TEXT("1") : Index == 3 ? TEXT("2") : FString::Printf(TEXT("PASSIVE %d"), Index-3);
        const FRAbilityInfo Info = Controller->GetSkillDefinition(Skill);
        Button->SetLabelText(FText::FromString(Input));
        Button->UpdateSlotVisual(Info.Icon, Info.BGMaterial, FText::FromString(bUnlocked ? Name : FString::Printf(TEXT("Level %d"), UnlockLevel)), bUnlocked,
            Index < 4 ? FLinearColor::White : Layout->PassiveColor);
        Button->SetToolTipText(FText::FromString(Input + TEXT(" — ") + (bUnlocked ? Name : FString::Printf(TEXT("Unlocks at level %d"), UnlockLevel))));
        Button->SetIsEnabled(bUnlocked && (!bChoosingSlot || ASC->GetEquipIssue(SelectedSkill, Button->SlotTag).IsEmpty()));
        Button->SetSelected(bChoosingSlot && Button->GetIsEnabled());
    }
    if (EquipButton)
    {
        EquipButton->Browser = this; EquipButton->Action = 7;
        const FRAbilityInfo Info = Controller->GetSkillDefinition(SelectedSkill);
        const bool bEquippable = (Info.AbilityType == T.ability_type_dynamic || Info.AbilityType == T.ability_type_passive)
            && ASC->GetLearnedSkillRank(SelectedSkill) > 0;
        EquipButton->SetIsEnabled(bEquippable);
        EquipButton->SetVisibility(bEquippable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        EquipButton->SetLabelText(FText::FromString(bChoosingSlot ? TEXT("Choose a slot / Cancel") : ASC->GetInputTagFromAbilityTag(SelectedSkill).IsValid()
            ? TEXT("Remove from Loadout") : Info.AbilityType == T.ability_type_passive ? TEXT("Activate Passive") : TEXT("Add to Loadout")));
    }
}
