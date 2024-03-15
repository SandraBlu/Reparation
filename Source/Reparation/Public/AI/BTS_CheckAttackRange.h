// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_CheckAttackRange.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UBTS_CheckAttackRange : public UBTService
{
	GENERATED_BODY()

public:

	UBTS_CheckAttackRange();

	protected:

	/* Max desired attack range of AI pawn */
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxAttackRange;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector AttackRangeKey;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
