// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AOWidgetController.h"
#include "AttributeMenuController.generated.h"

struct FAttributeData;
class UAttributeInfo;
struct FGameplayTag;
struct FGameplayAttribute;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfo, const FAttributeData&, Info);


UCLASS(BlueprintType, Blueprintable)
class REPARATION_API UAttributeMenuController : public UAOWidgetController
{
	GENERATED_BODY()

public:

	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FAttributeInfo AttributeInfoDelegate;

protected:

	UPROPERTY(EditDefaultsOnly)
	UAttributeInfo* AttributeInfo;

	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
	
};
