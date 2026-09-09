// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "POI.generated.h"

UCLASS(Blueprintable)
class REPARATION_API APOI : public AActor
{
	GENERATED_BODY()

	friend class UNavigationComponent;
	
public:	

	APOI();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class USphereComponent* POISphere;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UPOINavMarker* NavigationMarker;

	/** The tag for this location, generally you'd just have one but more can technically be added */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Point Of Interest")
	FGameplayTagContainer POITags;

	/** The display name for this poi when we display it in the UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Point Of Interest", meta = (ExposeOnSpawn = "true"))
	FText POIDisplayName;

	/** The location in the world we'll fast travel the player to when the POI is selected in the world map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Point Of Interest", meta = (MakeEditWidget))
	FTransform FastTravelLocation;

protected:

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

	virtual void BeginPlay() override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual void SetDiscovered(const bool bDiscovered);

};
