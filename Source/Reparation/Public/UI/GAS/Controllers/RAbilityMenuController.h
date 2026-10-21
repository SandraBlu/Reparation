// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "GameplayTagContainer.h"
#include "RGameplayTags.h"
#include "GAS/Data/AbilityInfo.h"
#include "UI/GAS/Controllers/RWidgetController.h"
#include "RAbilityMenuController.generated.h"

struct FSelectedAbility
{
	FGameplayTag Ability = FGameplayTag();
	FGameplayTag Status = FGameplayTag();
};
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAbilitySelectedSignature, bool, bEnableSpendPointBtn, bool, bEnableEquipBtn, FString, DescriptionString, FString, NextLevelDescriptionString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitEquipSelectionSignature,  const FGameplayTag&, AbilityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityReassignedSignature,  const FGameplayTag&, AbilityTag);
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REPARATION_API URAbilityMenuController : public URWidgetController
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skills|Appearance")
    TSoftClassPtr<class URSkillBrowser> SkillBrowserClass = TSoftClassPtr<URSkillBrowser>(FSoftObjectPath(TEXT("/Game/UI/Abilities/UI_SkillBrowser.UI_SkillBrowser_C")));
	/** Shared native-menu theme. Change this in BP_AbilityMenuController Class Defaults to use a duplicate theme. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skills|Appearance")
	TSoftObjectPtr<class URSkillMenuStyle> SkillMenuStyle = TSoftObjectPtr<URSkillMenuStyle>(FSoftObjectPath(TEXT("/Game/UI/Style/DA_SkillMenuStyle.DA_SkillMenuStyle")));
	class URSkillMenuStyle* GetSkillMenuStyle() const;
	UPROPERTY() TObjectPtr<UUserWidget> SkillMenuHost;
	UPROPERTY() TObjectPtr<class UCanvasPanel> SkillMenuCanvas;
	UPROPERTY() TObjectPtr<class URSkillBrowser> SkillBrowser;
	UPROPERTY() TArray<TObjectPtr<class UWidget>> LegacySkillMenuWidgets;
	TArray<ESlateVisibility> LegacySkillMenuVisibilities;
	UPROPERTY() TObjectPtr<class URSkillBrowserButton> LearnViewButton;
	UPROPERTY() TObjectPtr<class URSkillBrowserButton> LoadoutViewButton;
	UFUNCTION(BlueprintPure, Category="Skills") TArray<FRAbilityInfo> GetSkillsForDiscipline(ERSkillDiscipline Discipline) const;
	UFUNCTION(BlueprintPure, Category="Skills") FRAbilityInfo GetSkillDefinition(FGameplayTag Skill) const;
	URAbilitySystemComponent* GetSkillASC() { return GetRASC(); }
	int32 GetSkillPoints();
	UFUNCTION(BlueprintCallable, Category="Skills") void OpenSkillBrowser();
	UFUNCTION(BlueprintCallable, Category="Skills") void CloseSkillBrowser();
	
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies()override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangeSignature AbilityPointsChange;

	UPROPERTY(BlueprintAssignable)
	FAbilitySelectedSignature AbilitySelectedDelegate;

	UPROPERTY(BlueprintAssignable)
	FWaitEquipSelectionSignature WaitEquipDelegate;

	UPROPERTY(BlueprintAssignable)
	FWaitEquipSelectionSignature StopWaitEquipDelegate;

	UPROPERTY(BlueprintAssignable)
	FAbilityReassignedSignature AbilityReassignedDelegate;

	UFUNCTION(BlueprintCallable)
	void AbilitySelected(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable, meta=(DeprecatedFunction, DeprecationMessage="Purchasing now belongs to the skill tree. This compatibility function does nothing."))
	void SpendPointButtonPressed();

	UFUNCTION(BlueprintCallable)
	void AbilityDeselect();

	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

	UFUNCTION(BlueprintCallable)
	void EquippedRowInputPressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType);
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot);
	
private:

	void ShouldEnableButtons(const FGameplayTag& AbilityStatus,
	int32 AbilityPoints, bool& bEnableSpendPointButton, bool& bEnableEquipButton);
	FSelectedAbility SelectedAbility = { FRGameplayTags::Get().ability_none, FRGameplayTags::Get().ability_status_locked};
	int32 CurrentAbilityPoints = 0;
	bool bWaitingForEquipSelection = false;
	FGameplayTag SelectedSlot;
	
};
