// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavMarkerComponent.h"
#include "POINavMarker.generated.h"

/**
 * Special Navigation marker component intended for points of interest. Includes some fast travel functionality and colour changing upon discovery 
 */
UCLASS()
class REPARATION_API UPOINavMarker : public UNavMarkerComponent
{
	GENERATED_BODY()

	UPOINavMarker();

	virtual bool CanInteract_Implementation(class UNavigationComponent* Selector, class APlayerController* SelectorOwner) const override;

public:

	void SetDiscovered(const bool bDiscovered);
	
};
