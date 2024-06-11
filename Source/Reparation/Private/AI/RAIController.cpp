// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

ARAIController::ARAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComp");
	check(Blackboard);
	BTree = CreateDefaultSubobject<UBehaviorTree>("BehaviorTree");
	check(BTree);
}
