// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RInventoryBFL.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URInventoryBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory", meta = (DefaultToSelf = "Target"))
	static class URInventoryComponent* GetInventoryComponentFromTarget(AActor* Target);

	
	//Sort the array of inventory items from a-z
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TArray<class URItem*> SortItemArrayAlphabetical(TArray<class URItem*> InItems, const bool bReverse);


	//Sort the array of inventory items using stack weight
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TArray<class URItem*> SortItemArrayWeight(TArray<class URItem*> InItems, const bool bReverse);
	
};
