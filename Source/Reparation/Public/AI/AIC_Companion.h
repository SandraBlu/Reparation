// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_Companion.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API AAIC_Companion : public AAIController
{
	GENERATED_BODY()
	
public:
	
	AAIC_Companion(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void BeginPlay() override;
	
private:

	//Behavior Tree
	UPROPERTY()
	class UBehaviorTree* BT_Companion;
	
};
