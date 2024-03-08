// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFLAbilitySystem.generated.h"

class UOverlayWidgetController;
class UAttributeMenuController;

/**
 * 
 */
UCLASS()
class REPARATION_API UBFLAbilitySystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintPure, Category = "AOBFL")
	static UOverlayWidgetController* GetOverlayController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AOBFL")
	static UAttributeMenuController* GetAttributeMenuController(const UObject* WorldContextObject);
};
