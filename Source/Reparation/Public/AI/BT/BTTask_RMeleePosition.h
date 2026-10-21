#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AITypes.h"
#include "BTTask_RMeleePosition.generated.h"

class AAIController;
class AREnemy;

/** Reach a useful weapon range before the existing attack task commits a montage. */
UCLASS()
class REPARATION_API UBTTask_RMeleePosition : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_RMeleePosition();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	UPROPERTY(EditAnywhere, Category="Melee")
	bool bWaitForAttackOnly = false;
private:
	TWeakObjectPtr<AAIController> Controller;
	FAIRequestID MoveId;
	float Elapsed = 0.f;
	TWeakObjectPtr<AREnemy> FacingPawn;
	TWeakObjectPtr<AActor> PreviousFocus;
	bool bFacingSaved = false;
	bool bSavedControllerYaw = false;
	bool bSavedOrientToMovement = false;
	bool bSavedDesiredRotation = false;
	EBTNodeResult::Type UpdatePosition(UBehaviorTreeComponent& OwnerComp);
	void StopOwnMove();
};
