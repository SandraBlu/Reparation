// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AbilityStats/Controllers/RWidgetController.h"
#include "ROverlayController.generated.h"

struct FOnAttributeChangeData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealtChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealtChangedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedSignature, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChangedSignature, float, NewMaxStamina);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REPARATION_API UROverlayController : public URWidgetController
{
	GENERATED_BODY()

public:

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnHealtChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnHealtChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnMaxStaminaChangedSignature OnMaxStaminaChanged;

protected:

	//Callbacks
	void HealthChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;
	void StaminaChanged(const FOnAttributeChangeData& Data) const;
	void MaxStaminaChanged(const FOnAttributeChangeData& Data) const;
	
};
