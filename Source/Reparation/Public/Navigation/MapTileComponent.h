// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavMarkerComponent.h"
#include "MapTileComponent.generated.h"

USTRUCT(BlueprintType)
struct FMapTileLayer
{
	GENERATED_BODY()

	FMapTileLayer() 
	{
	
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Tiles")
	FGameplayTag LayerTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Tiles")
	class UTexture2D* MapTileTexture;

};
/**
 * 
 */
UCLASS(ClassGroup=(Reparation), Blueprintable, DisplayName = "Map Tile Marker", meta=(BlueprintSpawnableComponent))
class REPARATION_API UMapTileComponent : public UNavMarkerComponent
{
	GENERATED_BODY()

	UMapTileComponent();

	virtual bool CanInteract_Implementation(class UNavigationComponent* Selector, class APlayerController* SelectorOwner) const;

public:

	//The map tile layers. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Tiles")
	TMap<FGameplayTag, UTexture2D*> MapTileLayers;

	//Set the layer of this map tile for the given domains 
	UFUNCTION(BlueprintCallable, Category = "Map Layers")
	bool SetMapLayer(UPARAM(meta = (Categories = "Navigator.MapLayer"))const FGameplayTag& NewLayer, UPARAM(meta = (Categories = "Navigator.NavigatorTypes"))const FGameplayTagContainer& Domains);
	
};
