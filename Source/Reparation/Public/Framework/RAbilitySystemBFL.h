// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RAbilitySystemBFL.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URAbilitySystemBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category="RAbilitySystemLibrary|WidgetController")
	static URWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="RAbilitySystemLibrary|WidgetController")
	static URWidgetController* GetAttributeMenuController(const UObject* WorldContextObject);
	
};
