// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOItemTooltip.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UAOItemTooltip : public UUserWidget
{
	GENERATED_BODY()

	public:
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip", meta = (ExposeOnSpawn = true))
	class UAOItemSlot* InventorySlot;
	
};
