#pragma once
#include "Blueprint/UserWidget.h"
#include "GAS/Data/AbilityInfo.h"
#include "UI/GAS/RSkillMenuStyle.h"
#include "RSkillBrowser.generated.h"
class URSkillBrowser;
class URAbilityMenuController;
class UVerticalBox;
class UTextBlock;
class UButton;

UCLASS()
class REPARATION_API URSkillBrowserButton : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Browser|Design") bool bUseDesignerLayout = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Browser|Design") bool bApplyMenuStyle = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Browser|Design") FLinearColor DesignerSelectedTint = FLinearColor(0.12f, 0.36f, 0.48f, 1.f);
    UPROPERTY() TObjectPtr<URSkillBrowser> Browser;
    UPROPERTY() TObjectPtr<URAbilityMenuController> MenuController;
    UPROPERTY() TObjectPtr<URSkillMenuStyle> MenuStyle;
    ERSkillMenuButtonRole StyleRole = ERSkillMenuButtonRole::Skill;
    FGameplayTag Skill;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Browser") ERSkillDiscipline Discipline = ERSkillDiscipline::Other;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Browser", meta=(ClampMin="0", ClampMax="7", ToolTip="0 discipline, 1 select skill, 2 purchase, 3 select element, 4 back to loadout, 5 physical, 6 loadout navigation, 7 equip/remove selected skill")) int32 Action = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Browser") FText Label;
    void SetSelected(bool bInSelected);
    UFUNCTION(BlueprintCallable, Category="Skill Browser") void SetLabelText(FText Text);
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativePreConstruct() override;
    UFUNCTION(BlueprintImplementableEvent, Category="Skill Browser") void SelectionChanged(bool bIsSelected);
    UFUNCTION() void Clicked();
protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UButton> Button;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UTextBlock> LabelText;
private:
    FLinearColor DesignerNormalTint = FLinearColor::White;
    void ApplyAppearance();
    bool bSelected = false;
};
UCLASS()
class REPARATION_API URSkillBrowser : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Browser|Design") bool bUseDesignerLayout = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Browser|Design") TSubclassOf<URSkillBrowserButton> SkillButtonClass;
    UPROPERTY() TObjectPtr<URAbilityMenuController> Controller;
    UPROPERTY() TObjectPtr<URSkillMenuStyle> MenuStyle;
    virtual void HandleAction(int32 Action, FGameplayTag Skill, ERSkillDiscipline Discipline);
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UVerticalBox> List;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UTextBlock> Details;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UTextBlock> Points;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UTextBlock> MeleeElement;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<URSkillBrowserButton> PurchaseButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<URSkillBrowserButton> ElementButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<URSkillBrowserButton> PhysicalButton;
protected:
    UPROPERTY() TArray<TObjectPtr<URSkillBrowserButton>> DisciplineButtons;
    ERSkillDiscipline SelectedDiscipline = ERSkillDiscipline::Melee;
    FGameplayTag SelectedSkill;
    float RefreshTime = 0.f;
    virtual void RebuildList();
    virtual void RefreshDetails();
    URSkillBrowserButton* MakeButton(FText Label, int32 Action, FGameplayTag Skill = FGameplayTag(), ERSkillDiscipline Discipline = ERSkillDiscipline::Other);
};
