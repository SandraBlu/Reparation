// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomWaypoint.generated.h"

UCLASS()
class REPARATION_API ACustomWaypoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ACustomWaypoint();

	//Scene root 
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	class USceneComponent* Root;

	//The map marker for the custom waypoint 
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	class UCustomWaypointMarker* WaypointMarker;

};
