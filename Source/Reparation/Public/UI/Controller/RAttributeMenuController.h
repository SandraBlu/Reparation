// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Controller/RWidgetController.h"
#include "RAttributeMenuController.generated.h"

struct FAttributeData;
class URAttributeData;
struct FGameplayTag;
struct FGameplayAttribute;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfo, const FAttributeData&, Info);
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REPARATION_API URAttributeMenuController : public URWidgetController
{
	GENERATED_BODY()

public:

	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FAttributeInfo AttributeInfoDelegate;

	UPROPERTY(BlueprintAssignable, Category = "XP")
	FOnPlayerStatChangeSignature AttributePtsChangeDelegate;

	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

protected:

	UPROPERTY(EditDefaultsOnly)
	URAttributeData* AttributeInfo;

	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
	
};
