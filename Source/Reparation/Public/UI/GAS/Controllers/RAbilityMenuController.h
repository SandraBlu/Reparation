// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RGameplayTags.h"
#include "UI/GAS/Controllers/RWidgetController.h"
#include "RAbilityMenuController.generated.h"

struct FSelectedAbility
{
	FGameplayTag Ability = FGameplayTag();
	FGameplayTag Status = FGameplayTag();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAbilitySelectedSignature, bool, bEnableSpendPointBtn, bool, bEnableEquipBtn, FString, DescriptionString, FString, NextLevelDescriptionString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitEquipSelectionSignature,  const FGameplayTag&, AbilityType);
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REPARATION_API URAbilityMenuController : public URWidgetController
{
	GENERATED_BODY()

public:
	
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies()override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangeSignature AbilityPointsChange;

	UPROPERTY(BlueprintAssignable)
	FAbilitySelectedSignature AbilitySelectedDelegate;

	UPROPERTY(BlueprintAssignable)
	FWaitEquipSelectionSignature WaitEquipDelegate;

	UFUNCTION(BlueprintCallable)
	void AbilitySelected(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();

	UFUNCTION(BlueprintCallable)
	void AbilityDeselect();

	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

private:

	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus,
		int32 AbilityPoints, bool& bEnableSpendPointButton, bool& bEnableEquipButton);

	FSelectedAbility SelectedAbility = { FRGameplayTags::Get().ability_none, FRGameplayTags::Get().ability_status_locked};
	int32 CurrentAbilityPoints = 0;

	bool bWaitingForEquipSelection = false;
	
};
