// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIC_Companion.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AAIC_Companion::AAIC_Companion(const FObjectInitializer& ObjectInitializer)
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComp");
	check(Blackboard);
	BT_Companion = CreateDefaultSubobject<UBehaviorTree>("BehaviorTree");
	check(BT_Companion);
}

void AAIC_Companion::BeginPlay()
{
	Super::BeginPlay();

	if (BT_Companion != nullptr)
	{
		RunBehaviorTree(BT_Companion);
	}
}
