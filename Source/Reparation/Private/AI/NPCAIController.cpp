// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/NPCAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

ANPCAIController::ANPCAIController(const FObjectInitializer& ObjectInitializer)
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComp");
	check(Blackboard);
	NPCBTree = CreateDefaultSubobject<UBehaviorTree>("BehaviorTree");
	check(NPCBTree);
}

void ANPCAIController::BeginPlay()
{
	Super::BeginPlay();

	if (NPCBTree != nullptr)
	{
		RunBehaviorTree(NPCBTree);
	}
}
