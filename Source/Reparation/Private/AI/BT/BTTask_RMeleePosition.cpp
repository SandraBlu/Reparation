#include "AI/BT/BTTask_RMeleePosition.h"
#include "AI/REnemy.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Characters/RCharacterBase.h"
#include "Components/Combat/REnemyMeleeSweepComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Stealth/RStealthComponent.h"

UBTTask_RMeleePosition::UBTTask_RMeleePosition()
{
	NodeName = TEXT("Position for melee attack");
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RMeleePosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Controller = OwnerComp.GetAIOwner();
	MoveId = FAIRequestID::InvalidRequest;
	Elapsed = 0.f;
	const EBTNodeResult::Type Result = UpdatePosition(OwnerComp);
	if (Result != EBTNodeResult::InProgress) StopOwnMove();
	return Result;
}

EBTNodeResult::Type UBTTask_RMeleePosition::UpdatePosition(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AI = Controller.Get();
	AREnemy* Enemy = AI ? Cast<AREnemy>(AI->GetPawn()) : nullptr;
	if (!Enemy || Enemy->IsDead_Implementation()) return EBTNodeResult::Failed;
	if (!Enemy->MeleeSweep || !Enemy->MeleeSweep->bUseHeadSweep) return EBTNodeResult::Succeeded;
	if (!OwnerComp.GetBlackboardComponent()) return EBTNodeResult::Failed;
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	if (!IsValid(Target)) return EBTNodeResult::Failed;
	if (const ARCharacterBase* TargetCharacter = Cast<ARCharacterBase>(Target); TargetCharacter &&
		(TargetCharacter->IsDead_Implementation() || TargetCharacter->Stealth->IsConcealed())) return EBTNodeResult::Failed;
	// Repeated tree evaluations must not reposition during an already committed
	// attack or hit reaction. Re-evaluate range after that montage ends.
	if (UAnimInstance* Anim = Enemy->GetMesh()->GetAnimInstance(); Anim && Anim->Montage_IsPlaying(nullptr)) return EBTNodeResult::InProgress;
	if (bWaitForAttackOnly) return EBTNodeResult::Succeeded;
	float Min, Preferred, Max;
	Enemy->GetMeleeRangeForTarget(Target, Min, Preferred, Max);
	const float Distance = FVector::Dist2D(Enemy->GetActorLocation(), Target->GetActorLocation());
	if (Distance >= Min && Distance <= Max) { StopOwnMove(); return EBTNodeResult::Succeeded; }
	// Backing away must keep the player in front of the sight cone. Temporarily
	// use controller-facing rotation while moving, then restore the pawn settings.
	if (!bFacingSaved)
	{
		FacingPawn = Enemy;
		PreviousFocus = AI->GetFocusActor();
		bSavedControllerYaw = Enemy->bUseControllerRotationYaw;
		bSavedOrientToMovement = Enemy->GetCharacterMovement()->bOrientRotationToMovement;
		bSavedDesiredRotation = Enemy->GetCharacterMovement()->bUseControllerDesiredRotation;
		bFacingSaved = true;
	}
	Enemy->bUseControllerRotationYaw = true;
	Enemy->GetCharacterMovement()->bOrientRotationToMovement = false;
	Enemy->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	AI->SetFocus(Target);
	const FRotator Facing(0.f, (Target->GetActorLocation() - Enemy->GetActorLocation()).Rotation().Yaw, 0.f);
	AI->SetControlRotation(Facing);
	Enemy->SetActorRotation(Facing);
	if (AI->GetMoveStatus() == EPathFollowingStatus::Moving && MoveId.IsValid()) return EBTNodeResult::InProgress;
	FVector Away = (Enemy->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
	if (Away.IsNearlyZero()) Away = -Enemy->GetActorForwardVector();
	const FVector Desired = Target->GetActorLocation() + Away * Preferred;
	UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Enemy->GetWorld());
	FNavLocation Projected;
	if (!Nav || !Nav->ProjectPointToNavigation(Desired, Projected, FVector(100.f, 100.f, 250.f))) return EBTNodeResult::Failed;
	FAIMoveRequest Request(Projected.Location);
	Request.SetAcceptanceRadius(15.f);
	Request.SetReachTestIncludesAgentRadius(false);
	Request.SetReachTestIncludesGoalRadius(false);
	Request.SetAllowPartialPath(false);
	Request.SetCanStrafe(true);
	const FPathFollowingRequestResult Result = AI->MoveTo(Request);
	MoveId = Result.MoveId;
	return Result.Code == EPathFollowingRequestResult::RequestSuccessful ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

void UBTTask_RMeleePosition::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Elapsed += DeltaSeconds;
	EBTNodeResult::Type Result = Elapsed > 6.f ? EBTNodeResult::Failed : UpdatePosition(OwnerComp);
	if (Result != EBTNodeResult::InProgress)
	{
		StopOwnMove();
		FinishLatentTask(OwnerComp, Result);
	}
}

void UBTTask_RMeleePosition::StopOwnMove()
{
	if (AAIController* AI = Controller.Get(); AI && MoveId.IsValid() && AI->GetPathFollowingComponent()->GetCurrentRequestId() == MoveId) AI->StopMovement();
	MoveId = FAIRequestID::InvalidRequest;
	if (bFacingSaved)
	{
		if (AREnemy* Enemy = FacingPawn.Get())
		{
			Enemy->bUseControllerRotationYaw = bSavedControllerYaw;
			Enemy->GetCharacterMovement()->bOrientRotationToMovement = bSavedOrientToMovement;
			Enemy->GetCharacterMovement()->bUseControllerDesiredRotation = bSavedDesiredRotation;
		}
		if (AAIController* AI = Controller.Get())
		{
			if (PreviousFocus.IsValid()) AI->SetFocus(PreviousFocus.Get());
			else AI->ClearFocus(EAIFocusPriority::Gameplay);
		}
		FacingPawn.Reset();
		PreviousFocus.Reset();
		bFacingSaved = false;
	}
}

EBTNodeResult::Type UBTTask_RMeleePosition::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	StopOwnMove();
	return EBTNodeResult::Aborted;
}
