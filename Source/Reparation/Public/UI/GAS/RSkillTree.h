#pragma once
#include "UI/GAS/RSkillBrowser.h"
#include "RSkillTree.generated.h"

class UCanvasPanel;
class UImage;
class URichTextBlock;

UENUM(BlueprintType)
enum class ERSkillTreeBranch : uint8 { Stealth, Melee, Archer, Shared };

USTRUCT(BlueprintType)
struct FRSkillTreeEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag Skill;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Title;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true)) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TObjectPtr<UTexture2D> Icon;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ERSkillTreeBranch Branch = ERSkillTreeBranch::Melee;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector2D Position = FVector2D::ZeroVector;
    /** Visual connections only. Actual purchase requirements live in DA_AbilityInfo. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FName> Connections;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bPlanned = false;
    /** A presentation junction, not a purchasable or future ability. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bJunction = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.5", ClampMax="2.5")) float VisualScale = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bPassive = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0",ClampMax="5")) int32 Tier = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1",ClampMax="5")) int32 PointCost = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText TierName;
};

/** Positions, future nodes and artwork can change without changing gameplay requirements. */
UCLASS(BlueprintType)
class REPARATION_API URSkillTreeLayout : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector2D CanvasSize = FVector2D(1100.f, 940.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector2D NodeSize = FVector2D(130.f, 110.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FRSkillTreeEntry> Nodes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor LockedColor = FLinearColor(.23f, .29f, .34f, 1.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor AvailableColor = FLinearColor(.72f, .60f, .30f, 1.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor LearnedColor = FLinearColor(1.f, .77f, .25f, 1.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ConnectionThickness = 2.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Branch Colors") FLinearColor StealthColor = FLinearColor(.18f,.8f,.58f,1);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Branch Colors") FLinearColor MeleeColor = FLinearColor(.95f,.35f,.2f,1);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Branch Colors") FLinearColor ArcherColor = FLinearColor(.2f,.58f,1.f,1);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Branch Colors") FLinearColor SharedColor = FLinearColor(1.f,.77f,.25f,1);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Passive Appearance") FLinearColor PassiveColor = FLinearColor(.75f,.45f,1.f,1);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Passive Appearance", meta=(ClampMin="0.5",ClampMax="1.2")) float PassiveScale = .85f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Learned Highlight", meta=(ClampMin="1",ClampMax="8")) float LearnedRingWidth = 3.f;
    FLinearColor GetBranchColor(ERSkillTreeBranch Branch) const;
};

UCLASS()
class REPARATION_API URSkillTreeNode : public URSkillBrowserButton
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, Category="Skill Tree") FName NodeId;
    UPROPERTY(BlueprintReadOnly, Category="Skill Tree") bool bLearned = false;
    UPROPERTY(BlueprintReadOnly, Category="Skill Tree") bool bAvailable = false;
    UPROPERTY(BlueprintReadOnly, Category="Skill Tree") bool bPlanned = false;
    UPROPERTY(BlueprintReadOnly, Category="Skill Tree") bool bPassive = false;
    ERSkillTreeBranch Branch = ERSkillTreeBranch::Melee;
    FLinearColor LearnedHighlight = FLinearColor(1.f,.77f,.25f,1);
    float LearnedRingWidth = 3.f;
    void UpdateVisual(UTexture2D* Icon, FText Rank, FLinearColor Color, bool bSelected);
    FVector2D GetConnectionPoint() const;
    virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& Culling,
        FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool bEnabled) const override;
    virtual void NativePreConstruct() override;
    UFUNCTION() void SelectNode();
protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UImage> SkillIcon;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> RankText;
    bool bHasIcon = false;
};

UCLASS()
class REPARATION_API URSkillLoadoutSlot : public URSkillBrowserButton
{
    GENERATED_BODY()
public:
    FGameplayTag SlotTag;
    virtual void NativePreConstruct() override;
    UFUNCTION() void SelectSlot();
    void UpdateSlotVisual(UTexture2D* Icon, UMaterialInterface* Background, FText Name, bool bUnlocked, FLinearColor Accent);
protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UImage> SkillIcon;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UImage> AbilityBG;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UImage> SkillRing;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> SlotNameText;
    FSlateBrush EmptyBackground;
    bool bCapturedBackground = false;
};

UCLASS()
class REPARATION_API URSkillTree : public URSkillBrowser
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Tree") TSoftObjectPtr<URSkillTreeLayout> LayoutAsset;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Tree") TSubclassOf<URSkillTreeNode> NodeClass;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Tree|Loadout") TSubclassOf<URSkillLoadoutSlot> SlotWidgetClass;
    UFUNCTION(BlueprintCallable, Category="Skill Tree") void SelectTreeNode(FName Id);
    UFUNCTION(BlueprintCallable, Category="Skill Tree") void SelectLearnedSkill(FGameplayTag Skill);
    UFUNCTION(BlueprintCallable, Category="Skill Tree") void SelectLoadoutSlot(FGameplayTag SlotTag);
    virtual void HandleAction(int32 Action, FGameplayTag Skill, ERSkillDiscipline Discipline) override;
    virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UCanvasPanel> TreeCanvas;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UVerticalBox> ActiveSlots;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UVerticalBox> PassiveSlots;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UVerticalBox> PassiveChoices;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<URSkillBrowserButton> EquipButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<URSkillBrowserButton> RespecButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<URichTextBlock> RichDetails;
    virtual void RebuildList() override;
    virtual void RefreshDetails() override;
private:
    UPROPERTY() TObjectPtr<URSkillTreeLayout> Layout;
    UPROPERTY() TArray<TObjectPtr<URSkillTreeNode>> TreeNodes;
    FName SelectedNode;
    bool bChoosingSlot = false;
    bool bConfirmRespec = false;
    UPROPERTY() TArray<TObjectPtr<URSkillLoadoutSlot>> LoadoutSlots;
    void RefreshLoadout();
};
