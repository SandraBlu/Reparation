// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/GAS/Controllers/RWidgetController.h"
#include "RAbilityMenuController.generated.h"

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
	
};
