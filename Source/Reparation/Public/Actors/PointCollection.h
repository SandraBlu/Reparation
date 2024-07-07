// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointCollection.generated.h"

UCLASS()
class REPARATION_API APointCollection : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APointCollection();
	
	UFUNCTION(BlueprintPure)
	TArray<USceneComponent*> GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride = 0.f);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<USceneComponent*> ImmutablePts;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_0;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_1;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_2;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_3;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_4;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_5;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_6;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_7;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_8;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_9;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* Pt_10;
};
