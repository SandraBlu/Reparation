// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RAbilitySystemLibrary.generated.h"

class URAttributeMenuController;
class UROverlayWidgetController;
/**
 * 
 */
UCLASS()
class REPARATION_API URAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "AbilityBFL")
	static UROverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AbilityBFL")
	static URAttributeMenuController* GetAttributeMenuController(const UObject* WorldContextObject);
	
	
};
