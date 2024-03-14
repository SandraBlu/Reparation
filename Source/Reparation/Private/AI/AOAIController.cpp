// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AOAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

AAOAIController::AAOAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BBComp");
	check(Blackboard);
	BTComp = CreateDefaultSubobject<UBehaviorTreeComponent>("BTComp");
	check(BTComp);
}
