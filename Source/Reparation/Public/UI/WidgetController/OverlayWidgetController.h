// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AOWidgetController.h"
#include "GameplayEffectTypes.h"
#include "OverlayWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChange, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChange, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChange, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChange, float, NewMaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnergyChange, float, NewEnergy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxEnergyChange, float, NewMaxEnergy);



UCLASS(BlueprintType, Blueprintable)
class REPARATION_API UOverlayWidgetController : public UAOWidgetController
{
	GENERATED_BODY()

public:

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnHealthChange OnHealthChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnMaxHealthChange OnMaxHealthChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnHealthChange OnStaminaChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnMaxHealthChange OnMaxStaminaChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnHealthChange OnEnergyChange;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnMaxHealthChange OnMaxEnergyChange;

protected:

	void HandleHealthChanged(const FOnAttributeChangeData& Data) const;
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data) const;
	void HandleStaminaChanged(const FOnAttributeChangeData& Data) const;
	void HandleMaxStaminaChanged(const FOnAttributeChangeData& Data) const;
	void HandleEnergyChanged(const FOnAttributeChangeData& Data) const;
	void HandleMaxEnergyChanged(const FOnAttributeChangeData& Data) const;

};


