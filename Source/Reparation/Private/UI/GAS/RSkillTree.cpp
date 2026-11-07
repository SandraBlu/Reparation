#include "UI/GAS/RSkillTree.h"
#include "UI/GAS/Controllers/RAbilityMenuController.h"
#include "GAS/RAbilitySystemComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/OverlaySlot.h"
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
    // Active/passive columns may stretch their rows; keep the layered icon itself square.
    if (WidgetTree)
    {
        if (auto* Circle = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("CircleSize"))))
        {
            const float Side = Circle->IsWidthOverride() ? FMath::Max(1.f, Circle->GetWidthOverride()) : 92.f;
            Circle->SetWidthOverride(Side); Circle->SetHeightOverride(Side);
            Circle->SetMinDesiredWidth(Side); Circle->SetMaxDesiredWidth(Side);
            Circle->SetMinDesiredHeight(Side); Circle->SetMaxDesiredHeight(Side);
            Circle->SetMinAspectRatio(1.f); Circle->SetMaxAspectRatio(1.f);
            if (auto* CircleSlot = Cast<UVerticalBoxSlot>(Circle->Slot))
            {
                CircleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
                CircleSlot->SetHorizontalAlignment(HAlign_Center);
                CircleSlot->SetVerticalAlignment(VAlign_Center);
            }
        }
        if (Button)
        {
            auto Style = Button->GetStyle();
            Style.NormalPadding = FMargin(0.f); Style.PressedPadding = FMargin(0.f);
            Button->SetStyle(Style);
        }
        if (auto* Layers = WidgetTree->FindWidget(TEXT("HudIconLayers")))
            if (auto* LayerContentSlot = Cast<UButtonSlot>(Layers->Slot))
            {
                LayerContentSlot->SetPadding(FMargin(0.f));
                LayerContentSlot->SetHorizontalAlignment(HAlign_Fill); LayerContentSlot->SetVerticalAlignment(VAlign_Fill);
            }
        for (const TCHAR* Name : {TEXT("AbilityBG"), TEXT("SkillIcon"), TEXT("Glass"), TEXT("SkillRing")})
            if (auto* Layer = WidgetTree->FindWidget(FName(Name)))
                if (auto* ImageSlot = Cast<UOverlaySlot>(Layer->Slot))
                {
                    const FMargin ImagePadding = ImageSlot->GetPadding();
                    const float Inset = FMath::Max(FMath::Max(ImagePadding.Left, ImagePadding.Right), FMath::Max(ImagePadding.Top, ImagePadding.Bottom));
                    ImageSlot->SetPadding(FMargin(Inset));
                    ImageSlot->SetHorizontalAlignment(HAlign_Fill); ImageSlot->SetVerticalAlignment(VAlign_Fill);
                }
    }
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

void URSkillTreeNode::SetIconTexture(UTexture2D* NewIcon)
{
    IconTexture = NewIcon;
    ApplyIconAppearance();
}
void URSkillTreeNode::SetIconSize(float NewSize)
{
    IconPixelSize = FMath::Max(16.f, NewSize);
    ApplyIconAppearance();
}
void URSkillTreeNode::SetCircleSize(float NewSize)
{
    CircleDiameter = FMath::Max(32.f, NewSize);
    ApplyIconAppearance();
}
void URSkillTreeNode::SynchronizeProperties()
{
    Super::SynchronizeProperties();
    ApplyIconAppearance();
}
void URSkillTreeNode::ApplyIconAppearance()
{
    if (WidgetTree)
    {
        if (auto* Size = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("CircleSize"))))
        { Size->SetWidthOverride(CircleDiameter); Size->SetHeightOverride(CircleDiameter); }
        if (auto* Size = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("IconSize"))))
        { Size->SetWidthOverride(IconPixelSize); Size->SetHeightOverride(IconPixelSize); }
    }
    UTexture2D* DisplayIcon = IconTexture ? IconTexture.Get() : DataIcon.Get();
    bHasIcon = DisplayIcon != nullptr;
    if (SkillIcon)
    {
        SkillIcon->SetBrushFromTexture(DisplayIcon);
        SkillIcon->SetVisibility(DisplayIcon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
}
void URSkillTreeNode::NativePreConstruct()
{
    Super::NativePreConstruct();
    ApplyIconAppearance();
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
    DataIcon = Icon;
    if (IconTexture) Icon = IconTexture;
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
    TreeNodes.Reset();
    TMap<FName, URSkillTreeNode*> DesignerNodes;
    WidgetTree->ForEachWidget([&DesignerNodes](UWidget* Widget)
    {
        if (auto* Node = Cast<URSkillTreeNode>(Widget)) DesignerNodes.Add(Node->NodeId, Node);
    });
    ResolvedEntries = Layout->Nodes;
    // Blueprint owns which nodes exist. Layout data must never recreate deleted widgets.
    ResolvedEntries.RemoveAll([&DesignerNodes](const FRSkillTreeEntry& Entry)
    {
        return !DesignerNodes.Contains(Entry.Id);
    });
    // A Designer-authored node can call its skill without a duplicate layout entry.
    for (const auto& Pair : DesignerNodes)
        if (!Pair.Key.IsNone() && (Pair.Value->SkillTag.IsValid() || SkillLinks.Contains(Pair.Key))
            && !ResolvedEntries.ContainsByPredicate([&Pair](const FRSkillTreeEntry& Entry){ return Entry.Id == Pair.Key; }))
        {
            FRSkillTreeEntry Entry;
            Entry.Id = Pair.Key;
            Entry.Skill = Pair.Value->SkillTag;
            Entry.Branch = Pair.Value->Branch;
            ResolvedEntries.Add(Entry);
        }
    for (FRSkillTreeEntry& Entry : ResolvedEntries)
    {
        const URSkillTreeNode* DesignerNode = DesignerNodes.FindRef(Entry.Id);
        const FGameplayTag* Link = DesignerNode && DesignerNode->SkillTag.IsValid() ? &DesignerNode->SkillTag : SkillLinks.Find(Entry.Id);
        if (Link)
        {
            Entry.Skill = *Link;
            if (Link->IsValid())
            {
                Entry.bPlanned = false;
                Entry.bJunction = false;
                const FRAbilityInfo Info = Controller->GetSkillDefinition(*Link);
                if (!Info.DisplayName.IsEmpty()) Entry.Title = Info.DisplayName;
                Entry.Description = Info.Summary;
            }
        }
    }
    if (USizeBox* Bounds = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("TreeBounds"))))
    {
        Bounds->SetWidthOverride(Layout->CanvasSize.X);
        Bounds->SetHeightOverride(Layout->CanvasSize.Y);
    }
    // Branch titles are editable Designer widgets below the tree, not runtime tier overlays.
    for (const FRSkillTreeEntry& Entry : ResolvedEntries)
    {
        URSkillTreeNode* Node = DesignerNodes.FindRef(Entry.Id);
        if (!Node) continue;
        Node->Browser = this; Node->NodeId = Entry.Id; Node->Skill = Entry.Skill; Node->Label = Entry.Title;
        Node->bPlanned = Entry.bPlanned; Node->Branch = Entry.Branch;
        Node->LearnedHighlight = Layout->LearnedColor; Node->LearnedRingWidth = Layout->LearnedRingWidth;
        Node->bApplyMenuStyle = false;
        const FRAbilityInfo Info = Controller->GetSkillDefinition(Entry.Skill);
        Node->bPassive = Entry.bPassive || Info.AbilityType == FRGameplayTags::Get().ability_type_passive;
        TreeNodes.Add(Node);
    }
}
void URSkillTree::SelectTreeNode(FName Id)
{
    if (!Layout) return;
    if (const FRSkillTreeEntry* Node = ResolvedEntries.FindByPredicate([Id](const FRSkillTreeEntry& Entry) { return Entry.Id == Id; }))
    {
        bChoosingSlot = false; bConfirmRespec = false; SelectedNode = Id; SelectedSkill = Node->Skill;
        RefreshDetails();
    }
}
void URSkillTree::SetSkillLink(FName NodeId, FGameplayTag SkillTag)
{
    if (NodeId.IsNone()) return;
    SkillLinks.Add(NodeId, SkillTag);
    RefreshSkillLinks();
}
void URSkillTree::ClearSkillLink(FName NodeId)
{
    if (SkillLinks.Remove(NodeId) == 0) return;
    RefreshSkillLinks();
}
void URSkillTree::RefreshSkillLinks()
{
    RebuildList();
    if (!SelectedNode.IsNone()) SelectTreeNode(SelectedNode);
    else RefreshDetails();
}
void URSkillTreeNode::SetSkillTag(FGameplayTag NewSkillTag)
{
    if (SkillTag == NewSkillTag) return;
    SkillTag = NewSkillTag;
    if (auto* Tree = Cast<URSkillTree>(Browser)) Tree->RefreshSkillLinks();
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
        const FRSkillTreeEntry* Entry = ResolvedEntries.FindByPredicate([Node](const FRSkillTreeEntry& Item) { return Item.Id == Node->NodeId; });
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
        Details->SetText(FText::FromString(TEXT("CHOOSE YOUR PATH\n\nMelee and Bow share the central Stealth path. Advance through both skills in each combat tier; Stealth requires its preceding skill.\n\nEither Master leads to Death Mark, then four separate elements: Fire, Ice, Electric and Poison. Dark requires all four.\n\nSelect an icon to inspect its path and current purchase requirements. Planned skills and Master rewards still require gameplay authoring.")));
        PurchaseButton->SetIsEnabled(false);
        return;
    }
    const FRSkillTreeEntry* Entry = ResolvedEntries.FindByPredicate([this](const FRSkillTreeEntry& Item) { return Item.Id == SelectedNode; });
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
            + TEXT("\n\nSelect a connected skill to inspect its requirements.")));
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
                Entry->Tier > 1 ? TEXT("See the path requirements above.") : TEXT("Foundation tier.")) : TEXT(""))
            + TEXT("\n\nFuture technique. Not available to purchase yet.")));
        PurchaseButton->SetIsEnabled(false);
        ElementButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        FString Current, Next;
        ASC->GetDescriptionByAbilityTag(SelectedSkill, Current, Next);
        const FText PurchaseIssue=ASC->GetSkillUpgradeIssue(SelectedSkill);
        FString Text = TEXT("<Title>") + Entry->Title.ToString() + TEXT("</>\n")
            + (PurchaseIssue.IsEmpty() ? FString() : PurchaseIssue.ToString()+TEXT("\n\n")) + Entry->Description.ToString()
            + TEXT("\n\n<Title>Current Skill</>\n") + Current + TEXT("\n\n<Title>Next Upgrade</>\n") + Next;
        // Older authored descriptions may contain escaped newlines.
        Text.ReplaceInline(TEXT("\\n"), TEXT("\n"));
        Text += FString::Printf(TEXT("\n\n<Title>Requirements</>\nCharacter level <Level>%d</>"), Info.LevelRequirement);
        if (ASC->GetLearnedSkillRank(SelectedSkill)>0 && Info.AbilityType == FGameplayTag::RequestGameplayTag(TEXT("ability.type.technique")))
            Text += TEXT("\n\n<Title>Already in use</>\nThis technique improves an existing action or uses its fixed control. It does not need an active loadout slot.");
        if (Info.BranchTier > 0) Text += FString::Printf(TEXT("\n%s / Tier %d / %d point(s) per rank\n%s"), *Entry->TierName.ToString(), Info.BranchTier, FMath::Clamp(Info.SkillPointCost,1,5),
            Info.BranchTier > 1 ? TEXT("Requires learned skills in the preceding branch tier.") : TEXT("Foundation tier."));
        if (Info.MinimumCorePointsSpent > 0) Text += FString::Printf(TEXT("\n%d points invested across any branches (%d / %d)"), Info.MinimumCorePointsSpent, ASC->CoreSkillPointsSpent, Info.MinimumCorePointsSpent);
        for (const FRSkillPrerequisite& Required : Info.Prerequisites)
            if (Required.RequiredRank > 0)
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
    for (const FRSkillTreeEntry& Entry : ResolvedEntries)
        for (FName Parent : Entry.Connections)
            if (const FRSkillTreeEntry* Other = ResolvedEntries.FindByPredicate([Parent](const FRSkillTreeEntry& Node) { return Node.Id == Parent; }))
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
        const FRSkillTreeEntry* Entry = ResolvedEntries.FindByPredicate([this](const FRSkillTreeEntry& E) { return E.Id == SelectedNode; });
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
        const FRSkillTreeEntry* Entry = ResolvedEntries.FindByPredicate([this](const FRSkillTreeEntry& E) { return E.Id == SelectedNode; });
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
