// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AOWidgetController.h"
#include "GameplayEffectTypes.h"
#include "OverlayWidgetController.generated.h"


USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UAOUserWidget> MessageWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;

};

class UAOUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChange, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature, FUIWidgetRow, Row);

UCLASS(BlueprintType, Blueprintable)
class REPARATION_API UOverlayWidgetController : public UAOWidgetController
{
	GENERATED_BODY()

public:

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChange OnHealthChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChange OnMaxHealthChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChange OnStaminaChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChange OnMaxStaminaChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChange OnEnergyChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChange OnMaxEnergyChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Messages")
	FMessageWidgetRowSignature WidgetMessageRow;

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Data")
	UDataTable* DTMessageUI;
	
	//Handles Messages on HUD
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);
};

template<typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	return DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));	 
}

