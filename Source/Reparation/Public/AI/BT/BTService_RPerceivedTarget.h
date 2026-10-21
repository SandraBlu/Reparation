#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_RPerceivedTarget.generated.h"

/** Mirrors the perception-selected target into legacy combat blackboard keys. */
UCLASS()
class REPARATION_API UBTService_RPerceivedTarget : public UBTService
{
    GENERATED_BODY()
public:
    UBTService_RPerceivedTarget();
protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
