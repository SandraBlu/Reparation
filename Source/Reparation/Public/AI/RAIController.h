// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RAIController.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API ARAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	
	ARAIController();
	
protected:
	
	UPROPERTY()
	UBehaviorTree* BTree;
	
};
