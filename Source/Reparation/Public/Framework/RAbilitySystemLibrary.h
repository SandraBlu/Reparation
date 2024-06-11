// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Data/RCharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RAbilitySystemLibrary.generated.h"

class UAbilitySystemComponent;
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

	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController")
	static UROverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController")
	static URAttributeMenuController* GetAttributeMenuController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "RBFL|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "RBFL|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC);
	
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static URCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
};
