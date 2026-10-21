#include "AI/BT/BTTask_RSearch.h"
#include "AI/RAIController.h"
#include "Characters/RCharacterBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_RSearch::UBTTask_RSearch()
{
    NodeName = TEXT("Search last known location");
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RSearch::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ARAIController* AI = Cast<ARAIController>(OwnerComp.GetAIOwner());
    if (!AI || !AI->bSearching || !AI->GetPawn()) return EBTNodeResult::Failed;
    AI->StartSearchMovement();
    return EBTNodeResult::InProgress;
}

void UBTTask_RSearch::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    ARAIController* AI = Cast<ARAIController>(OwnerComp.GetAIOwner());
    const ARCharacterBase* Pawn = AI ? Cast<ARCharacterBase>(AI->GetPawn()) : nullptr;
    if (!AI || !AI->GetPawn() || !AI->bSearching || (Pawn && Pawn->IsDead_Implementation()))
    {
        if (AI && AI->bSearching) AI->StopMovement();
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
    AI->TickSearchMovement(DeltaSeconds);
}

EBTNodeResult::Type UBTTask_RSearch::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ARAIController* AI = Cast<ARAIController>(OwnerComp.GetAIOwner()); AI && AI->bSearching) AI->StopMovement();
    return EBTNodeResult::Aborted;
}
