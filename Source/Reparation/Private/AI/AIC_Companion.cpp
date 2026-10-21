// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIC_Companion.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/TargetPoint.h"
#include "Engine/World.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/RCompanion.h"
#include "GAS/Ability/RCompanionAttackAbility.h"
#include "Components/RCompanionPartyComponent.h"

AAIC_Companion::AAIC_Companion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.f;
}

void AAIC_Companion::CancelCurrentMove()
{
	// Abort callbacks must not finish a replacement order.
	ActiveMove = FAIRequestID::InvalidRequest;
	StopMovement();
}

bool AAIC_Companion::FollowLeader(ACharacter* Leader)
{
	if (!IsValid(Leader) || !GetPawn() || Leader == GetPawn()) return false;
	if (IRCombatInterface::Execute_IsDead(GetPawn())) return false;
	CancelCombat();
	CancelCurrentMove();
	FollowTarget = Leader;
	if (ACharacter* Companion = Cast<ACharacter>(GetPawn()))
	{
		NormalWalkSpeed = Companion->GetCharacterMovement()->MaxWalkSpeed;
		NormalCrouchSpeed = Companion->GetCharacterMovement()->MaxWalkSpeedCrouched;
		bNormalPathAcceleration = Companion->GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths;
		Companion->GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths = true;
		bFollowSpeedSaved = true;
	}
	FActorSpawnParameters Spawn;
	Spawn.Owner = this;
	Spawn.ObjectFlags |= RF_Transient;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FollowGoalActor = GetWorld()->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), Leader->GetNavAgentLocation(), FRotator::ZeroRotator, Spawn);
	if (!FollowGoalActor.IsValid()) { RestoreFollowSpeed(); return false; }
	FollowGoalActor->SetActorHiddenInGame(true);
	FollowGoalActor->SetActorEnableCollision(false);
	CurrentOrder = ERCompanionOrder::Follow;
	return true;
}

void AAIC_Companion::Stay()
{
	CancelCombat();
	CurrentOrder = ERCompanionOrder::Stay;
	CancelCurrentMove();
	// Retain the leader for nearby stay-state sneak mirroring.
	OnOrderFinished.Broadcast(CurrentOrder, true);
}

bool AAIC_Companion::GoToLocation(FVector Location)
{
	if (!GetPawn() || Location.ContainsNaN()) return false;
	if (IRCombatInterface::Execute_IsDead(GetPawn())) return false;
	CancelCombat();
	CancelCurrentMove();
	OrderDestination = Location;
	CurrentOrder = ERCompanionOrder::GoToLocation;
	FAIMoveRequest Request(Location);
	Request.SetAcceptanceRadius(ArrivalDistance);
	Request.SetAllowPartialPath(false);
	const FPathFollowingRequestResult Result = MoveTo(Request);
	if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		ActiveMove = Result.MoveId;
		return true;
	}
	CurrentOrder = ERCompanionOrder::Stay;
	const bool bArrived = Result.Code == EPathFollowingRequestResult::AlreadyAtGoal;
	if (!bArrived)
	{
		UE_LOG(LogTemp, Warning, TEXT("Companion %s cannot reach destination %s. Check NavMesh coverage, agent dimensions, and movement settings."),
			*GetNameSafe(GetPawn()), *Location.ToString());
	}
	OnOrderFinished.Broadcast(ERCompanionOrder::GoToLocation, bArrived);
	return bArrived;
}

void AAIC_Companion::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AssistanceRetryDelay = FMath::Max(0.f, AssistanceRetryDelay - DeltaSeconds);
	ACharacter* CompanionCharacter = Cast<ACharacter>(GetPawn());
	if (!CompanionCharacter) return;
	if (ARCompanion* Companion = Cast<ARCompanion>(CompanionCharacter); Companion && Companion->IsDead_Implementation()) return;
	if (CurrentOrder == ERCompanionOrder::Attack) { TickCombat(DeltaSeconds); return; }
	const bool bAccompanying = CurrentOrder == ERCompanionOrder::Follow ||
		(CurrentOrder == ERCompanionOrder::Stay && IsValid(FollowTarget) &&
		 FVector::DistSquared(CompanionCharacter->GetActorLocation(), FollowTarget->GetActorLocation()) <= FMath::Square(SneakMirrorDistance));
	const bool bShouldCrouch = bMirrorPlayerSneaking && bAccompanying && IsValid(FollowTarget) && FollowTarget->bIsCrouched;
	if (bShouldCrouch)
	{
		CompanionCharacter->Crouch();
		bAppliedLeaderCrouch = true;
	}
	else if (bAppliedLeaderCrouch)
	{
		CompanionCharacter->UnCrouch();
		bAppliedLeaderCrouch = false;
	}
	if (CurrentOrder != ERCompanionOrder::Follow) return;
	if (!IsValid(FollowTarget) || !FollowGoalActor.IsValid())
	{
		RestoreFollowSpeed();
		CancelCurrentMove();
		CurrentOrder = ERCompanionOrder::Stay;
		OnOrderFinished.Broadcast(ERCompanionOrder::Follow, false);
		return;
	}
	const FVector Goal = FollowTarget->GetNavAgentLocation() + FollowTarget->GetActorQuat().RotateVector(FormationOffset);
	// Actor goals let path following track the final destination continuously,
	// instead of braking toward a succession of fixed location snapshots.
	FollowGoalActor->SetActorLocation(Goal);
	const bool bMoving = GetMoveStatus() == EPathFollowingStatus::Moving;
	// Once settled, require extra separation before starting another path. This
	// prevents small leader turns from repeatedly toggling walk and idle.
	const float StopDistance = FMath::Max(1.f, ArrivalDistance);
	const float StartDistance = StopDistance + 75.f;
	const float GoalDistanceSquared = FVector::DistSquared2D(CompanionCharacter->GetActorLocation(), Goal);
	if (bFollowSpeedSaved)
	{
		UCharacterMovementComponent* Movement = CompanionCharacter->GetCharacterMovement();
		const float LeaderSpeed = FollowTarget->GetVelocity().Size2D();
		// Match the leader near formation rather than sprinting into the arrival
		// radius. Extra separation adds catch-up speed; getting too close slows us.
		const float DesiredSpeed = FMath::Max(0.f, LeaderSpeed + (FMath::Sqrt(GoalDistanceSquared) - StartDistance) * 1.5f);
		const float WalkTarget = FMath::Clamp(DesiredSpeed, FMath::Min(60.f, NormalWalkSpeed), NormalWalkSpeed);
		const float CrouchTarget = FMath::Clamp(DesiredSpeed, FMath::Min(60.f, NormalCrouchSpeed), NormalCrouchSpeed);
		Movement->MaxWalkSpeed = FMath::FInterpTo(Movement->MaxWalkSpeed, WalkTarget, DeltaSeconds, 4.f);
		Movement->MaxWalkSpeedCrouched = FMath::FInterpTo(Movement->MaxWalkSpeedCrouched, CrouchTarget, DeltaSeconds, 4.f);
	}
	if (GoalDistanceSquared <= FMath::Square(bMoving ? StopDistance : StartDistance))
	{
		if (bMoving) CancelCurrentMove();
		return;
	}
	if (bMoving) return;
	FAIMoveRequest Request(FollowGoalActor.Get());
	Request.SetAcceptanceRadius(StopDistance);
	Request.SetReachTestIncludesAgentRadius(false);
	Request.SetReachTestIncludesGoalRadius(false);
	Request.SetAllowPartialPath(false);
	// RequestMove broadcasts the old request's abort synchronously. It must not
	// turn this replacement Follow order into Stay.
	ActiveMove = FAIRequestID::InvalidRequest;
	const FPathFollowingRequestResult Result = MoveTo(Request);
	if (Result.Code == EPathFollowingRequestResult::RequestSuccessful) ActiveMove = Result.MoveId;
	else if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Companion %s cannot follow to %s. Check NavMesh coverage, agent dimensions, and movement settings."),
			*GetNameSafe(GetPawn()), *Goal.ToString());
		CurrentOrder = ERCompanionOrder::Stay;
		RestoreFollowSpeed();
		OnOrderFinished.Broadcast(ERCompanionOrder::Follow, false);
	}
}

void AAIC_Companion::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (RequestID != ActiveMove) return;
	ActiveMove = FAIRequestID::InvalidRequest;
	if (CurrentOrder == ERCompanionOrder::Attack)
	{
		if (!Result.IsSuccess()) FinishCombat(false);
		return;
	}
	if (bReturningToStay)
	{
		bReturningToStay = false;
		if (!Result.IsSuccess()) OnOrderFinished.Broadcast(ERCompanionOrder::Stay, false);
		return;
	}
	if (CurrentOrder == ERCompanionOrder::GoToLocation)
	{
		CurrentOrder = ERCompanionOrder::Stay;
		OnOrderFinished.Broadcast(ERCompanionOrder::GoToLocation, Result.IsSuccess());
	}
	else if (CurrentOrder == ERCompanionOrder::Follow && !Result.IsSuccess())
	{
		CurrentOrder = ERCompanionOrder::Stay;
		RestoreFollowSpeed();
		OnOrderFinished.Broadcast(ERCompanionOrder::Follow, false);
	}
}

void AAIC_Companion::CancelCombat()
{
	if (bAssistingAutomatically) AssistanceRetryDelay = 2.f;
	bAssistingAutomatically = false;
	bDefendingSelf = false;
	AssistanceQuietTime = 0.f;
	AssistanceLeader.Reset();
	RestoreFollowSpeed();
	bReturningToStay = false;
	AttackTarget = nullptr;
	if (ARCompanion* Companion = Cast<ARCompanion>(GetPawn())) Companion->CancelAttack();
	ClearFocus(EAIFocusPriority::Gameplay);
}

void AAIC_Companion::RestoreFollowSpeed()
{
	if (FollowGoalActor.IsValid()) FollowGoalActor->Destroy();
	FollowGoalActor.Reset();
	if (!bFollowSpeedSaved) return;
	if (ACharacter* Companion = Cast<ACharacter>(GetPawn()))
	{
		Companion->GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
		Companion->GetCharacterMovement()->MaxWalkSpeedCrouched = NormalCrouchSpeed;
		Companion->GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths = bNormalPathAcceleration;
	}
	bFollowSpeedSaved = false;
}

void AAIC_Companion::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreFollowSpeed();
	Super::EndPlay(EndPlayReason);
}

bool AAIC_Companion::Attack(AActor* Target)
{
	ARCompanion* Companion = Cast<ARCompanion>(GetPawn());
	if (!Companion || !Companion->IsCombatReady() || !Companion->CanAttackTarget(Target))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack rejected for %s: check required equipped weapon, attributes, AttackAbility damage settings, attack montages and a living enemy target."), *GetNameSafe(Companion));
		return false;
	}
	// Replacing the target retains the original assignment, not the Attack order.
	if (Companion->HasAssignedWeapon() && !Companion->EquipAssignedWeapons()) return false;
	if (CurrentOrder != ERCompanionOrder::Attack)
	{
		ResumeOrder = CurrentOrder;
		// A defensive interruption of the return trip must keep the original Stay position.
		if (!bReturningToStay) ResumeDestination = CurrentOrder == ERCompanionOrder::GoToLocation ? OrderDestination : Companion->GetActorLocation();
	}
	CancelCombat();
	CancelCurrentMove();
	CombatOrigin = Companion->GetActorLocation();
	AttackWait = AttackDelay = 0.f;
	AttackTarget = Target;
	IRCombatInterface::Execute_SetCombatTarget(Companion, Target);
	CurrentOrder = ERCompanionOrder::Attack;
	if (bAppliedLeaderCrouch) { Companion->UnCrouch(); bAppliedLeaderCrouch = false; }
	return true;
}

bool AAIC_Companion::TryAutomaticAssist(AActor* Target, ACharacter* Leader)
{
	ARCompanion* Companion = Cast<ARCompanion>(GetPawn());
	URCompanionPartyComponent* Party = Companion ? Companion->RegisteredParty.Get() : nullptr;
	const bool bSelfDefense = Party && Party->IsThreateningCompanion(Target, Companion);
	const ARCharacterBase* LeaderCharacter = Cast<ARCharacterBase>(Leader);
	if (!Companion || !Party || Party->GetOwner() != Leader || !IsValid(Leader) ||
		(LeaderCharacter && LeaderCharacter->IsDead_Implementation()) ||
		(Leader->Implements<URCombatInterface>() && IRCombatInterface::Execute_IsDead(Leader)) ||
		(!bSelfDefense && (!Party->CanAssistNow() || AssistanceRetryDelay > 0.f || bReturningToStay)) || !Party->IsActiveThreat(Target) ||
		!Companion->bAutomaticAssistance ||
		(CurrentOrder != ERCompanionOrder::Follow && CurrentOrder != ERCompanionOrder::Stay) ||
		!IsValid(Target) ||
		(bSelfDefense ? FVector::DistSquared(Companion->GetActorLocation(), Target->GetActorLocation()) > FMath::Square(Companion->AssistanceRadius) :
		 (FVector::DistSquared(Companion->GetActorLocation(), Leader->GetActorLocation()) > FMath::Square(Companion->AssistanceRadius) ||
		  FVector::DistSquared(Target->GetActorLocation(), Leader->GetActorLocation()) > FMath::Square(Companion->AssistanceRadius)))) return false;
	if (!Attack(Target)) return false;
	AssistanceLeader = Leader;
	bAssistingAutomatically = true;
	bDefendingSelf = bSelfDefense;
	return true;
}

void AAIC_Companion::TickCombat(float DeltaSeconds)
{
	ARCompanion* Companion = Cast<ARCompanion>(GetPawn());
	if (!Companion) { FinishCombat(false); return; }
	if (!IsValid(AttackTarget)) { FinishCombat(false); return; }
	const ARCharacterBase* TargetCharacter = Cast<ARCharacterBase>(AttackTarget);
	if ((TargetCharacter && TargetCharacter->IsDead_Implementation()) || IRCombatInterface::Execute_IsDead(AttackTarget))
	{
		FinishCombat(true);
		return;
	}
	if (bAssistingAutomatically)
	{
		ACharacter* Leader = AssistanceLeader.Get();
		const ARCharacterBase* LeaderCharacter = Cast<ARCharacterBase>(Leader);
		URCompanionPartyComponent* Party = Companion->RegisteredParty.Get();
		// Sneaking gates new assistance, not combat already underway. A lost
		// threat ends an existing engagement through the quiet timer below.
		if (!Leader || (LeaderCharacter && LeaderCharacter->IsDead_Implementation()) ||
			(Leader->Implements<URCombatInterface>() && IRCombatInterface::Execute_IsDead(Leader)) || !Party ||
			(!bDefendingSelf &&
			 (FVector::DistSquared(Companion->GetActorLocation(), Leader->GetActorLocation()) > FMath::Square(Companion->AssistanceRadius) ||
			  FVector::DistSquared(AttackTarget->GetActorLocation(), Leader->GetActorLocation()) > FMath::Square(Companion->AssistanceRadius))))
		{
			FinishCombat(true);
			return;
		}
		const bool bThreatActive = Party->IsActiveThreat(AttackTarget);
		AssistanceQuietTime = bThreatActive ? 0.f : AssistanceQuietTime + DeltaSeconds;
		if (!bThreatActive && AssistanceQuietTime >= FMath::Max(0.f, Companion->AssistanceDisengageDelay))
		{
			FinishCombat(true);
			return;
		}
	}
	if (!Companion->CanAttackTarget(AttackTarget) || !Companion->IsCombatReady()
		|| FVector::DistSquared(CombatOrigin, AttackTarget->GetActorLocation()) > FMath::Square(MaxChaseDistance))
	{
		FinishCombat(false);
		return;
	}
	if (Companion->IsAttackActive()) { AttackWait = 0.f; return; }
	AttackDelay = FMath::Max(0.f, AttackDelay - DeltaSeconds);
	const float Range = Companion->AttackAbility->GetDefaultObject<URCompanionAttackAbility>()->AttackRange;
	const bool bInRange = FVector::Dist(Companion->GetActorLocation(), AttackTarget->GetActorLocation()) <= Range;
	if (!bInRange || !LineOfSightTo(AttackTarget))
	{
		ClearFocus(EAIFocusPriority::Gameplay);
		if (GetMoveStatus() == EPathFollowingStatus::Moving) return;
		FAIMoveRequest Request(AttackTarget);
		Request.SetAcceptanceRadius(Range * 0.7f);
		Request.SetReachTestIncludesAgentRadius(false);
		Request.SetReachTestIncludesGoalRadius(false);
		Request.SetAllowPartialPath(false);
		const FPathFollowingRequestResult Result = MoveTo(Request);
		if (Result.Code == EPathFollowingRequestResult::RequestSuccessful) ActiveMove = Result.MoveId;
		else if (Result.Code == EPathFollowingRequestResult::Failed) FinishCombat(false);
		else
		{
			AttackWait += DeltaSeconds;
			if (AttackWait >= AttackActivationTimeout) FinishCombat(false);
		}
		return;
	}
	CancelCurrentMove();
	SetFocus(AttackTarget);
	const FVector Facing = AttackTarget->GetActorLocation() - Companion->GetActorLocation();
	Companion->SetActorRotation(FRotator(0.f, Facing.Rotation().Yaw, 0.f));
	if (AttackDelay > 0.f) return;
	if (Companion->TryAttack())
	{
		AttackWait = 0.f;
		AttackDelay = AttackInterval;
	}
	else
	{
		AttackWait += DeltaSeconds;
		if (AttackWait >= AttackActivationTimeout) FinishCombat(false);
	}
}

void AAIC_Companion::FinishCombat(bool bSucceeded)
{
	const ERCompanionOrder PreviousOrder = ResumeOrder;
	const FVector PreviousDestination = ResumeDestination;
	CancelCurrentMove();
	CancelCombat();
	CurrentOrder = ERCompanionOrder::Stay;
	if (PreviousOrder == ERCompanionOrder::Follow && IsValid(FollowTarget)) FollowLeader(FollowTarget);
	else if (PreviousOrder == ERCompanionOrder::GoToLocation) GoToLocation(PreviousDestination);
	else if (GetPawn() && !IRCombatInterface::Execute_IsDead(GetPawn()))
	{
		FAIMoveRequest Request(PreviousDestination);
		Request.SetAcceptanceRadius(ArrivalDistance);
		Request.SetAllowPartialPath(false);
		const FPathFollowingRequestResult Result = MoveTo(Request);
		if (Result.Code == EPathFollowingRequestResult::RequestSuccessful) { ActiveMove = Result.MoveId; bReturningToStay = true; }
		else if (Result.Code == EPathFollowingRequestResult::Failed) OnOrderFinished.Broadcast(ERCompanionOrder::Stay, false);
	}
	OnOrderFinished.Broadcast(ERCompanionOrder::Attack, bSucceeded);
}

void AAIC_Companion::OnUnPossess()
{
	CancelCurrentMove();
	CancelCombat();
	CurrentOrder = ERCompanionOrder::Stay;
	FollowTarget = nullptr;
	Super::OnUnPossess();
}
