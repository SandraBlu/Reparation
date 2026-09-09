// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NPCAIController.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API ANPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	
	ANPCAIController(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void BeginPlay() override;
	
private:

	//Behavior Tree
	UPROPERTY()
	class UBehaviorTree* NPCBTree;


};
