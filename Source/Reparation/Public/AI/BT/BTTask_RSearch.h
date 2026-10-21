#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RSearch.generated.h"

/** Runs remembered-position navigation and scanning inside the enemy tree. */
UCLASS()
class REPARATION_API UBTTask_RSearch : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_RSearch();
protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
