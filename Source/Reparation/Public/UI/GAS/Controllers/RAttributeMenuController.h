// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/GAS/Controllers/RWidgetController.h"
#include "RAttributeMenuController.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URAttributeMenuController : public URWidgetController
{
	GENERATED_BODY()

public:

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;
	
};
