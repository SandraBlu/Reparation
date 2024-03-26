// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOItemSlot.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UAOItemSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Slot", meta = (ExposeOnSpawn = true))
	class UAOItem* Item;
	
};
