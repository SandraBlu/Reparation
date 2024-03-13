// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "BFLAbilitySystem.generated.h"


class UOverlayWidgetController;
class UAttributeMenuController;
class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class REPARATION_API UBFLAbilitySystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintPure, Category = "AOBFL|Widget Controller")
	static UOverlayWidgetController* GetOverlayController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AOBFL| Widget Controller")
	static UAttributeMenuController* GetAttributeMenuController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "AOBFL|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC);
};
