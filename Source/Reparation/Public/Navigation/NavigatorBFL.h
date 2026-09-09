// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigatorBFL.generated.h"

struct FNavigationMarkerSettings;
class UNavMarkerComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API UNavigatorBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// Add a navigation marker to the given actor! Navigator will automatically destroy the marker when the Navigation marker or its owning actor are destroyed. 
	UFUNCTION(BlueprintCallable, Category="Navigation", meta=(UnsafeDuringActorConstruction = "true"))
	static UNavMarkerComponent* AddNavigationMarkerToActor(class AActor* ActorToMark, const FNavigationMarkerSettings& MarkerSettings, UPARAM(meta = (Categories = "Navigator.NavigatorTypes")) const FGameplayTagContainer& MarkerDomain);

	
};
