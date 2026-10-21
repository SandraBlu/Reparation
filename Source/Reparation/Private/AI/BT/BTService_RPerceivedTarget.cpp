#include "AI/BT/BTService_RPerceivedTarget.h"
#include "AI/RAIController.h"
#include "Characters/RCharacterBase.h"
#include "Stealth/RStealthComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_RPerceivedTarget::UBTService_RPerceivedTarget()
{
    NodeName = TEXT("Use perceived combat target");
    bNotifyTick = true;
    bCallTickOnSearchStart = true;
    Interval = 0.1f;
    RandomDeviation = 0.f;
}

void UBTService_RPerceivedTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    ARAIController* AI = Cast<ARAIController>(OwnerComp.GetAIOwner());
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!AI || !BB) return;
    AI->EnableBehaviorTreeSearch();
    AActor* Target = AI->bSearching ? nullptr : AI->GetSightTarget();
    const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(Target);
    if (!IsValid(Target) || !AI->IsWithinSightCone(Target->GetActorLocation()) ||
        (CharacterBase && (CharacterBase->IsDead_Implementation() || CharacterBase->Stealth->IsConcealed()))) Target = nullptr;
    if (Target)
    {
        BB->SetValueAsObject(TEXT("TargetActor"), Target);
        BB->SetValueAsFloat(TEXT("DistanceToTarget"), AI->GetPawn()->GetDistanceTo(Target));
        AI->SetFocus(Target);
    }
    else
    {
        BB->ClearValue(TEXT("TargetActor"));
        BB->ClearValue(TEXT("DistanceToTarget"));
        AI->ClearFocus(EAIFocusPriority::Gameplay);
    }
}
