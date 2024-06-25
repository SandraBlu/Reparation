// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/GAS/Controllers/RWidgetController.h"
#include "ROverlayWidgetController.generated.h"

struct FRAbilityInfo;
class URAbilitySystemComponent;
class UAbilityInfo;

USTRUCT(BlueprintType, Blueprintable)
struct FUIMessageRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class URUserWidget> MessageWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

class URUserWidget;
struct FOnAttributeChangeData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangeSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetSignature, FUIMessageRow, Row);


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REPARATION_API UROverlayWidgetController : public URWidgetController
{
	GENERATED_BODY()

public:

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangeSignature OnHealthChange;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangeSignature OnMaxHealthChange;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangeSignature OnStaminaChange;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangeSignature OnMaxStaminaChange;

	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FMessageWidgetSignature MessageWidgetDelegate;

	UPROPERTY(BlueprintAssignable, Category="GAS|XP")
	FOnAttributeChangeSignature OnXPPercentChangeDelegate;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Level")
	FOnPlayerStatChangeSignature OnLevelChangeDelegate;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	void OnXPChange(int32 NewXP);

	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,	const FGameplayTag& Slot, const FGameplayTag& PrevSlot) const;
	
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);
	
};

template <typename T>
T* UROverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	return DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
}
