// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RAIController.h"
#include "AI/RProtectionComponent.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "RDebugHelper.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BrainComponent.h"
#include "Characters/RCharacterBase.h"
#include "Stealth/RStealthComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "AbilitySystemComponent.h"
#include "RGameplayTags.h"
#include "AI/RCombatCoordinator.h"
#include "AI/REnemy.h"
#include "Characters/RPlayer.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"


ARAIController::ARAIController(const FObjectInitializer& ObjectInitializer)

	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>("PathFollowingComponent"))
{	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	AISightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("AISightConfig");
	AISightConfig->DetectionByAffiliation.bDetectEnemies = true;
	AISightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AISightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	AISightConfig->SightRadius = 5000.0f;
	AISightConfig->LoseSightRadius = 5500.f;
	AISightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;
	AISightConfig->PeripheralVisionAngleDegrees = SightHalfAngle;

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>("PerceptionComp");
	PerceptionComp->ConfigureSense(*AISightConfig);
	PerceptionComp->SetDominantSense(UAISenseConfig_Sight::StaticClass());
	PerceptionComp->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ThisClass::OnEnemyPerceptionUpdated);

	SetGenericTeamId(FGenericTeamId(1));
	
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComp");
	check(Blackboard);
	BTree = CreateDefaultSubobject<UBehaviorTree>("BehaviorTree");
	check(BTree);
 
}

ETeamAttitude::Type ARAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* PawnToCheck = Cast<const APawn>(&Other);
	const IGenericTeamAgentInterface* OtherTeamAgent = PawnToCheck ? Cast<const IGenericTeamAgentInterface>(PawnToCheck->GetController()) : nullptr;
	if (Other.ActorHasTag(TEXT("player"))) return ETeamAttitude::Hostile;
	if (OtherTeamAgent && OtherTeamAgent->GetGenericTeamId() < GetGenericTeamId())
	{
		return ETeamAttitude::Hostile;
	}
	return ETeamAttitude::Friendly;
}

void ARAIController::BeginPlay()
{
	Super::BeginPlay();
	// Saved controller Blueprints may retain old sight configuration overrides.
	AISightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;
	AISightConfig->PeripheralVisionAngleDegrees = FMath::Clamp(SightHalfAngle, 0.f, 180.f);
	AISightConfig->LoseSightRadius = FMath::Max(AISightConfig->LoseSightRadius, AISightConfig->SightRadius);
	PerceptionComp->ConfigureSense(*AISightConfig);
	
	if (UCrowdFollowingComponent* CrowdComp = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
	{
		CrowdComp->SetCrowdSimulationState(bEnableCrowdAvoidance? ECrowdSimulationState::Enabled : ECrowdSimulationState::Disabled);
		switch (CrowdAvoidanceQuality)
		{
		case 1: CrowdComp->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Low);    break;
		case 2: CrowdComp->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Medium); break;
		case 3: CrowdComp->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Good);   break;
		case 4: CrowdComp->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::High);   break;
		default:
			break;
		}
		CrowdComp->SetAvoidanceGroup(1);
		CrowdComp->SetGroupsToAvoid(1);
		CrowdComp->SetCrowdCollisionQueryRange(CollisionQueryRange);
	}
}

void ARAIController::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>()) return;
	HandleSightUpdate(Actor, Stimulus.WasSuccessfullySensed(), Stimulus.StimulusLocation);
}

void ARAIController::ClearCombatTarget()
{
	if (UBlackboardComponent* BB = GetBlackboardComponent()) BB->ClearValue(TEXT("TargetActor"));
	if (GetPawn() && GetPawn()->Implements<URCombatInterface>())
		IRCombatInterface::Execute_SetCombatTarget(GetPawn(), nullptr);
}

void ARAIController::BeginSearch()
{
	if (bSearching) { ClearCombatTarget(); return; }
	bSearching = true;
	// Stop tree tasks/services so nearest-player services cannot keep tracking a
	// concealed actor. Search navigation only uses the frozen sight snapshot.
	if (!UsesBehaviorTreeSearch() && BrainComponent) BrainComponent->StopLogic(TEXT("Lost visual target; search last known position"));
	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	ClearCombatTarget();
	if (const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = CharacterBase->GetAbilitySystemComponent())
		{
			FGameplayTagContainer Attacks;
			Attacks.AddTag(FRGameplayTags::Get().ability_Attack_Melee);
			Attacks.AddTag(FRGameplayTags::Get().ability_Attack_Ranged);
			ASC->CancelAbilities(&Attacks);
		}
	}
	if (UsesBehaviorTreeSearch())
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), LastKnownLocation);
		GetBlackboardComponent()->SetValueAsBool(TEXT("SearchActive"), true);
	}
	else StartSearchMovement();
}

bool ARAIController::UsesBehaviorTreeSearch() const
{
	const UBlackboardComponent* BB = GetBlackboardComponent();
	return bBehaviorTreeSearchEnabled && BB && BB->GetKeyID(TEXT("SearchActive")) != FBlackboard::InvalidKey;
}

void ARAIController::StartSearchMovement()
{
	MoveToLocation(LastKnownLocation, 100.f, false, true, true, false, nullptr, false);
	bSearchWasMoving = GetMoveStatus() == EPathFollowingStatus::Moving;
	SearchMoveElapsed = SearchBlockedTime = 0.f;
	SearchStepTime = FMath::Max(0.5f, SearchStepInterval);
}

void ARAIController::HandleSightUpdate(AActor* Actor, bool bVisible, const FVector& SeenLocation)
{
	if (!IsValid(Actor)) return;
	if (bVisible && GetPawn()) bVisible = IsWithinSightCone(Actor->GetActorLocation());
	if (const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(Actor))
		bVisible &= !CharacterBase->IsDead_Implementation() && !CharacterBase->Stealth->IsConcealed();
	if (bVisible)
	{
		if (SightTarget.IsValid() && SightTarget.Get() != Actor && !bSearching) return;
		SelectCombatTarget(Actor, SeenLocation);
	}
	else if (SightTarget.Get() == Actor)
	{
		SightTarget.Reset();
		BeginSearch();
	}
}

void ARAIController::SelectCombatTarget(AActor* Actor, const FVector& SeenLocation)
{
	if (!IsValid(Actor)) return;
	if (const ARCharacterBase* Guard = Cast<ARCharacterBase>(GetPawn()); Guard && Guard->Protection && !Guard->Protection->CanEngage(Actor)) return;
	const bool bChanged = SightTarget.Get() != Actor;
		SightTarget = Actor;
	if (bChanged) TargetCommitmentRemaining = GetWorld()->GetSubsystem<URCombatCoordinator>()->GetProfile().TargetCommitmentSeconds;
		bHasSeenTarget = true;
		LastKnownLocation = FAISystem::IsValidLocation(SeenLocation) ? SeenLocation : Actor->GetActorLocation();
		const bool bWasSearching = bSearching;
		bSearching = false;
		if (bWasSearching) StopMovement();
		if (UsesBehaviorTreeSearch())
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), LastKnownLocation);
			GetBlackboardComponent()->SetValueAsBool(TEXT("SearchActive"), false);
		}
		if (UBlackboardComponent* BB = GetBlackboardComponent()) BB->SetValueAsObject(TEXT("TargetActor"), Actor);
		if (GetPawn() && GetPawn()->Implements<URCombatInterface>()) IRCombatInterface::Execute_SetCombatTarget(GetPawn(), Actor);
		if (bWasSearching && !UsesBehaviorTreeSearch() && BrainComponent) BrainComponent->RestartLogic();
}

void ARAIController::NotifyDamagedBy(AActor* Attacker, float Damage)
{
	if (!IsValid(Attacker) || Damage <= 0.f || !GetPawn() || GetTeamAttitudeTowards(*Attacker) != ETeamAttitude::Hostile) return;
	if (ARCharacterBase* Guard = Cast<ARCharacterBase>(GetPawn()); Guard && Guard->Protection) Guard->Protection->NotifyProvoked(Attacker);
	FThreatEntry* Entry = Threats.FindByPredicate([Attacker](const FThreatEntry& T) { return T.Actor.Get() == Attacker; });
	if (!Entry) { FThreatEntry New; New.Actor = Attacker; Threats.Add(New); Entry = &Threats.Last(); }
	Entry->Score = FMath::Min(1000.f, Entry->Score + 100.f + Damage * 2.f);
	FaceDamageSource(Attacker);
}

void ARAIController::FaceDamageSource(AActor* Attacker)
{
	if (!IsValid(Attacker) || !GetPawn()) return;
	// Damage turns attention toward the impact source, but never reveals a concealed actor.
	const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(Attacker);
	if (CharacterBase && (CharacterBase->IsDead_Implementation() || CharacterBase->Stealth->IsConcealed())) return;
	if (AREnemy* Enemy = Cast<AREnemy>(GetPawn()); Enemy && Enemy->IsDead_Implementation()) return;
	if (UAnimInstance* Anim = Cast<ACharacter>(GetPawn()) ? Cast<ACharacter>(GetPawn())->GetMesh()->GetAnimInstance() : nullptr; Anim && Anim->Montage_IsPlaying(nullptr))
	{
		PendingDamageSource = Attacker;
		return;
	}
	const FVector Direction = Attacker->GetActorLocation() - GetPawn()->GetActorLocation();
	const FRotator Facing(0.f, Direction.Rotation().Yaw, 0.f);
	SetControlRotation(Facing);
	GetPawn()->SetActorRotation(Facing);
	PerceptionComp->RequestStimuliListenerUpdate();
	if (LineOfSightTo(Attacker) && (bSearching || !SightTarget.IsValid() || TargetCommitmentRemaining <= 0.f)) SelectCombatTarget(Attacker, Attacker->GetActorLocation());
}

void ARAIController::UpdateThreats(const TArray<AActor*>& VisibleActors, float DeltaSeconds)
{
	URCombatCoordinator* Coordinator = GetWorld()->GetSubsystem<URCombatCoordinator>();
	const FRCombatDifficultyProfile Profile = Coordinator->GetProfile();
	TargetCommitmentRemaining = FMath::Max(0.f, TargetCommitmentRemaining - DeltaSeconds);
	if (PendingDamageSource.IsValid())
	{
		AActor* Attacker = PendingDamageSource.Get();
		PendingDamageSource.Reset();
		FaceDamageSource(Attacker);
	}
	for (FThreatEntry& Entry : Threats) Entry.Score = FMath::Max(0.f, Entry.Score - FMath::Max(0.f, Profile.ThreatDecayPerSecond) * DeltaSeconds);
	Threats.RemoveAll([](const FThreatEntry& Entry) { return !Entry.Actor.IsValid(); });
	TArray<AActor*> Candidates = VisibleActors;
	// A recent hit may precede the next sight update. Confirm the ray and cone,
	// then allow retaliation without reading a hidden actor's current position.
	for (const FThreatEntry& Entry : Threats)
	{
		const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(Entry.Actor.Get());
		if (!Entry.Actor.IsValid() || (CharacterBase && (CharacterBase->IsDead_Implementation() || CharacterBase->Stealth->IsConcealed()))) continue;
		if (Entry.Score > 0.f && IsWithinSightCone(Entry.Actor->GetActorLocation()) && LineOfSightTo(Entry.Actor.Get())) Candidates.AddUnique(Entry.Actor.Get());
	}
	AActor* Best = nullptr;
	float BestScore = -1.f, CurrentScore = -1.f;
	for (AActor* Actor : Candidates)
	{
		if (!IsValid(Actor) || GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile || !IsWithinSightCone(Actor->GetActorLocation())) continue;
		if (const ARCharacterBase* Guard = Cast<ARCharacterBase>(GetPawn()); Guard && Guard->Protection && !Guard->Protection->CanEngage(Actor)) continue;
		const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(Actor);
		if (CharacterBase && (CharacterBase->IsDead_Implementation() || CharacterBase->Stealth->IsConcealed())) continue;
		const FThreatEntry* Entry = Threats.FindByPredicate([Actor](const FThreatEntry& T) { return T.Actor.Get() == Actor; });
		float Score = (Actor->IsA<ARPlayer>() ? 25.f : 10.f) + (Entry ? Entry->Score : 0.f) + 10.f / (1.f + GetPawn()->GetDistanceTo(Actor) / 200.f);
		if (!Coordinator->CanStartAttack(Cast<AREnemy>(GetPawn()), Actor)) Score *= 0.45f;
		if (Actor == SightTarget.Get()) CurrentScore = Score;
		if (Score > BestScore) { BestScore = Score; Best = Actor; }
	}
	if (!Best) return;
	if (Best != SightTarget.Get() && CurrentScore >= 0.f &&
		(TargetCommitmentRemaining > 0.f || BestScore < CurrentScore * FMath::Max(1.f, Profile.TargetSwitchRatio))) return;
	if (AREnemy* Enemy = Cast<AREnemy>(GetPawn()); Enemy && Enemy->GetMesh()->GetAnimInstance() && Enemy->GetMesh()->GetAnimInstance()->Montage_IsPlaying(nullptr) && Best != SightTarget.Get()) return;
	SelectCombatTarget(Best, Best->GetActorLocation());
}

bool ARAIController::IsWithinSightCone(const FVector& Location) const
{
	if (!GetPawn()) return false;
	const FVector Direction = (Location - GetPawn()->GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero()) return true;
	const float Limit = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(SightHalfAngle, 0.f, 180.f)));
	return FVector::DotProduct(GetPawn()->GetActorForwardVector().GetSafeNormal2D(), Direction) >= Limit;
}

void ARAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!GetPawn()) return;
	if (const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(GetPawn()); CharacterBase && CharacterBase->IsDead_Implementation()) return;
	TArray<AActor*> VisibleActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), VisibleActors);
	UpdateThreats(VisibleActors, DeltaSeconds);
	for (AActor* Actor : VisibleActors)
	{
		if (!IsValid(Actor) || !IsWithinSightCone(Actor->GetActorLocation())) continue;
		if (const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(Actor); CharacterBase && (CharacterBase->IsDead_Implementation() || CharacterBase->Stealth->IsConcealed())) continue;
		HandleSightUpdate(Actor, true, Actor->GetActorLocation());
	}
	if (SightTarget.IsValid())
	{
		const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(SightTarget.Get());
		const bool bConfirmedDamageTarget = Threats.ContainsByPredicate([this](const FThreatEntry& T) { return T.Actor == SightTarget && T.Score > 0.f; }) && LineOfSightTo(SightTarget.Get());
		if ((!VisibleActors.Contains(SightTarget.Get()) && !bConfirmedDamageTarget) || !IsWithinSightCone(SightTarget->GetActorLocation()) || (CharacterBase && CharacterBase->Stealth->IsConcealed()))
			HandleSightUpdate(SightTarget.Get(), false, LastKnownLocation);
	}
	else if (bHasSeenTarget && !bSearching)
	{
		BeginSearch();
	}
	// Legacy tree services can assign TargetActor without consulting perception.
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		if (ARCharacterBase* Target = Cast<ARCharacterBase>(BB->GetValueAsObject(TEXT("TargetActor"))); Target &&
			(Target->Stealth->IsConcealed() || !IsWithinSightCone(Target->GetActorLocation())))
		{
			SightTarget.Reset();
			if (!bHasSeenTarget) LastKnownLocation = GetPawn()->GetActorLocation();
			BeginSearch();
		}
	}
	if (bSearching && !UsesBehaviorTreeSearch()) TickSearchMovement(DeltaSeconds);
}

void ARAIController::TickSearchMovement(float DeltaSeconds)
{
	if (!bSearching || !GetPawn()) return;
	ClearCombatTarget();
	const bool bSearchMoving = GetMoveStatus() == EPathFollowingStatus::Moving;
	if (bSearchMoving)
	{
		bSearchWasMoving = true;
		SearchMoveElapsed += DeltaSeconds;
		SearchBlockedTime = GetPawn()->GetVelocity().SizeSquared2D() < 25.f ? SearchBlockedTime + DeltaSeconds : 0.f;
		// A nearby pawn can physically block the remembered destination. Do not
		// remain in Moving forever with our sight cone pointing away from them.
		if (SearchMoveElapsed < 6.f && SearchBlockedTime < 1.5f) return;
		StopMovement();
	}
	// Wait and sweep the forward cone after arriving, including when the last
	// known location is already within acceptance distance or navigation fails.
	if (bSearchWasMoving)
	{
		bSearchWasMoving = false;
		SearchStepTime = FMath::Max(0.5f, SearchStepInterval);
	}
	ClearFocus(EAIFocusPriority::Gameplay);
	ClearFocus(EAIFocusPriority::Move);
	const float ScanYaw = FRotator::NormalizeAxis(GetPawn()->GetActorRotation().Yaw + FMath::Max(1.f, SearchScanTurnRate) * DeltaSeconds);
	const FRotator ScanRotation(0.f, ScanYaw, 0.f);
	SetControlRotation(ScanRotation);
	GetPawn()->SetActorRotation(ScanRotation);
	PerceptionComp->RequestStimuliListenerUpdate();
	SearchStepTime -= DeltaSeconds;
	if (SearchStepTime > 0.f) return;
	SearchStepTime = FMath::Max(0.5f, SearchStepInterval);
	if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation Point;
		if (Nav->GetRandomReachablePointInRadius(LastKnownLocation, FMath::Max(0.f, SearchRadius), Point))
		{
			MoveToLocation(Point.Location, 100.f, false, true, true, false, nullptr, false);
			bSearchWasMoving = GetMoveStatus() == EPathFollowingStatus::Moving;
			SearchMoveElapsed = SearchBlockedTime = 0.f;
		}
	}
}
