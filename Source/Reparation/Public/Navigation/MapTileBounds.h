// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapTileBounds.generated.h"

UCLASS()
class REPARATION_API AMapTileBounds : public AActor
{
	GENERATED_BODY()
	
public:	

	AMapTileBounds();

	/** The width our map should be, in unreal units (cm)*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Map Bounds")
	float MapWidth;
	
	/*The number of 1024x1024 tiles that should make up the map. For smaller maps you could use a single tile - for large open world maps you'll want more
	or your map will become blurry and low resolution. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Map Bounds")
	int32 NumTiles;

	UPROPERTY(BlueprintReadOnly, Category = "Map Bounds")
	class UBoxComponent* MapTileBounds;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif 	
	virtual void UpdateDebugDraw();

	UPROPERTY()
	class UMapTileBoundsDebugDraw* DebugDraw;
};
