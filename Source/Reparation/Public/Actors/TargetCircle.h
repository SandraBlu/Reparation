// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetCircle.generated.h"

UCLASS()
class REPARATION_API ATargetCircle : public AActor
{
	GENERATED_BODY()
	
public:	

	ATargetCircle();
	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UDecalComponent* TargetDecal;
	
protected:
	
	virtual void BeginPlay() override;

};
