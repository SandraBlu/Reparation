// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AOAIController.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

UCLASS()
class REPARATION_API AAOAIController : public AAIController
{
	GENERATED_BODY()

public:
		AAOAIController();

protected:

	UPROPERTY()
	UBehaviorTreeComponent* BTComp;

};
