// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RCommonUI_BFL.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URCommonUI_BFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	//Add a notification to the Player HUD, provided one has been created 
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Narrative Common UI", meta = (DisplayName = "Show Player HUD Notification", WorldContext = "WorldContextObject"))
	static void PushHUDNotification(const UObject* WorldContextObject, FText Message, const float Duration = 5.f);
	
};
