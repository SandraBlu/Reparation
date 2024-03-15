// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTS_CheckAttackRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTS_CheckAttackRange::UBTS_CheckAttackRange()
{
	MaxAttackRange = 2000.f;
}

void UBTS_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//Check Distance between AI Pawn and Target Actor

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (BBComp)
	{
		AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject("TargetActor"));
		if (TargetActor)
		{
			AAIController* AIController = OwnerComp.GetAIOwner();
			if (IsValid(AIController))
			{
				APawn* AIPawn = AIController->GetPawn();
				if (IsValid(AIPawn))
				{
					float DistanceTo = FVector::Distance(TargetActor->GetActorLocation(), AIPawn->GetActorLocation());
					bool bWithinRange = DistanceTo < 1500.f;
					bool bHasLOS = false;
					//if within range, check Line Of Sight
					if (bWithinRange)
					{
						bHasLOS = AIController->LineOfSightTo(TargetActor);
					}
					BBComp->SetValueAsBool(AttackRangeKey.SelectedKeyName, (bWithinRange && bHasLOS));
				}
			}
		}
	}
}
