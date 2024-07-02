// Copyright 2023 Klian. All Rights Reserved.


#include "ReplicatedTossComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Net/UnrealNetwork.h"

#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Async/ParallelFor.h"

// Sets default values for this component's properties
UReplicatedTossComponent::UReplicatedTossComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bAlwaysEnablePhysics = true;
	bIsUsingArch = false;
	
}

#pragma region Defaults

void UReplicatedTossComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Tick functions exposed to blueprint for easier management
}

void UReplicatedTossComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replication logics
	DOREPLIFETIME(UReplicatedTossComponent, TossOrigin);
	DOREPLIFETIME_CONDITION_NOTIFY(UReplicatedTossComponent, ReplicatedTossEnabled, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION(UReplicatedTossComponent, ArchOutPoints, COND_SkipOwner);
	DOREPLIFETIME_CONDITION_NOTIFY(UReplicatedTossComponent, bPrevisionIsValidCurrently, COND_None, REPNOTIFY_OnChanged);
}

void UReplicatedTossComponent::Activate(bool bReset)
{
	Super::Activate(bReset);	

	// Replicate activate function
	if (GetOwner()->GetLocalRole() == ENetRole::ROLE_Authority) {
		ReplicatedTossEnabled = true;	
		OnReplicatedTossActive.Broadcast(true);
	}
	else {
		ActivateOnServer(bReset);
	}
}

void UReplicatedTossComponent::Deactivate()
{
	Super::Deactivate();		

	// Replicate deactivate function
	if (GetOwner()->GetLocalRole() == ENetRole::ROLE_Authority) {

		if (CurrentTossAimTrailComponent) {
			CurrentTossAimTrailComponent->Deactivate();
			CurrentTossAimTrailComponent = nullptr;
			ArchOutPoints = TArray<FVector>();
		}

		ReplicatedTossEnabled = false;		
		OnReplicatedTossActive.Broadcast(false);

		bPrevisionIsValidCurrently = EReplicatedTossPrevisionValidity::PV_None;
		OnPrevisionValidityChange.Broadcast(bPrevisionIsValidCurrently);			
	}
	else {		
		DeactivateOnServer();
		OnPrevisionValidityChange.Broadcast(EReplicatedTossPrevisionValidity::PV_None);
	}
}

void UReplicatedTossComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UReplicatedTossComponent::OnRep_ReplicatedTossEnabled()
{
	// Replication for clients when this component is activated or deactivated on server
	if (CurrentTossAimTrailComponent && !ReplicatedTossEnabled) {
		CurrentTossAimTrailComponent->Deactivate();
		CurrentTossAimTrailComponent = nullptr;	
		ArchOutPoints.Empty();
	}

	OnReplicatedTossActive.Broadcast(ReplicatedTossEnabled);
}

void UReplicatedTossComponent::ActivateOnServer_Implementation(bool bReset)
{
	// Used by clients to activate this component on server
	ReplicatedTossEnabled = true;	
	ArchOutPoints.Empty();
	OnReplicatedTossActive.Broadcast(true);	
}

void UReplicatedTossComponent::DeactivateOnServer_Implementation()
{
	// Used by clients to deactivate this component on server

	if (CurrentTossAimTrailComponent) {
		CurrentTossAimTrailComponent->Deactivate();
		CurrentTossAimTrailComponent = nullptr;
		ArchOutPoints.Empty();
	}

	ReplicatedTossEnabled = false;
	OnReplicatedTossActive.Broadcast(false);

	bPrevisionIsValidCurrently = EReplicatedTossPrevisionValidity::PV_None;	
	OnPrevisionValidityChange.Broadcast(bPrevisionIsValidCurrently);	
}

#pragma endregion

#pragma region Runtime

void UReplicatedTossComponent::OnRep_PrevisionIsValidCurrentlyChanged(const EReplicatedTossPrevisionValidity OldValue)
{	
	// Replicated event, used to signal the net actor prevision validity has changed. Used to change visual effects depending on its result (ie. green if valid, red if not)
	// Called on next tick to give 1 frame to the tick function so it can generate the visual effect before updating it (otherwise the Listen Server will send the update before the visual effect is valid on clients, and any action affecting it won't take place)
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UReplicatedTossComponent::WaitArch_PrevisionIsValidCurrentlyChanged);
}

void UReplicatedTossComponent::WaitArch_PrevisionIsValidCurrentlyChanged()
{
	// As stated before, this is used to prevent clients from update the Niagara visual component before it's valid in their game net instance
	if (GetOwner()->GetLocalRole() == ENetRole::ROLE_SimulatedProxy) {
		OnPrevisionValidityChange.Broadcast(bPrevisionIsValidCurrently);		
	}
}

void UReplicatedTossComponent::SetPrevisionIsValidCurrentlyOnServer_Implementation(const bool IsValid)
{
	// Function used by clients to notify the client prevision validity has changed. This will then replicate the information to other clients
	bPrevisionIsValidCurrently = (IsValid)? EReplicatedTossPrevisionValidity::PV_True : EReplicatedTossPrevisionValidity::PV_False;
	OnPrevisionValidityChange.Broadcast(bPrevisionIsValidCurrently);
}

#pragma endregion

#pragma region Editables

void UReplicatedTossComponent::UseHighArch(const bool Enabled)
{
	// Local function to enable / disable high arch prevision
	bIsUsingArch = Enabled;
}

#pragma endregion

#pragma region TossOrigin

void UReplicatedTossComponent::SetTossOrigin(FTossOriginInformation NewTossOrigin)
{
	// Replication logics
	if (!NewTossOrigin.IsValid()) {
		return;
	}

	if (GetOwnerRole() == ENetRole::ROLE_Authority) {
		SetTossOrigin_Func(NewTossOrigin);
	}
	else {
		SetTossOrigin_Server(NewTossOrigin);
	}
}

FTransform UReplicatedTossComponent::GetTossOriginTransform() const
{
	if (!TossOrigin.Origin) {
		return FTransform();
	}

	return TossOrigin.Origin->GetSocketTransform(TossOrigin.SocketName);
}

FVector UReplicatedTossComponent::GetTossOriginLocation() const
{
	if (!TossOrigin.Origin) {
		return FVector();
	}

	return TossOrigin.Origin->GetSocketLocation(TossOrigin.SocketName);
}

FRotator UReplicatedTossComponent::GetTossOriginRotation() const
{
	if (!TossOrigin.Origin) {
		return FRotator();
	}

	return TossOrigin.Origin->GetSocketRotation(TossOrigin.SocketName);
}

FQuat UReplicatedTossComponent::GetTossOriginQuat() const
{
	if (!TossOrigin.Origin) {
		return FQuat();
	}

	return TossOrigin.Origin->GetSocketQuaternion(TossOrigin.SocketName);
}

FTransform UReplicatedTossComponent::GetTossOriginRelativeTransform() const
{
	if (!TossOrigin.Origin) {
		return FTransform();
	}

	return TossOrigin.Origin->GetSocketTransform(TossOrigin.SocketName, ERelativeTransformSpace::RTS_Actor);
}

FVector UReplicatedTossComponent::GetTossOriginRelativeLocation() const
{
	if (!TossOrigin.Origin) {
		return FVector();
	}

	return TossOrigin.Origin->GetSocketTransform(TossOrigin.SocketName, ERelativeTransformSpace::RTS_Actor).GetLocation();
}

FRotator UReplicatedTossComponent::GetTossOriginRelativeRotation() const
{
	if (!TossOrigin.Origin) {
		return FRotator();
	}

	return TossOrigin.Origin->GetSocketTransform(TossOrigin.SocketName, ERelativeTransformSpace::RTS_Actor).GetRotation().Rotator();
}

FQuat UReplicatedTossComponent::GetTossOriginRelativeQuat() const
{
	if (!TossOrigin.Origin) {
		return FQuat();
	}

	return TossOrigin.Origin->GetSocketTransform(TossOrigin.SocketName, ERelativeTransformSpace::RTS_Actor).GetRotation();
}

void UReplicatedTossComponent::SetTossOrigin_Server_Implementation(FTossOriginInformation NewTossOrigin)
{
	// From client to server function
	SetTossOrigin_Func(NewTossOrigin);
}

void UReplicatedTossComponent::SetTossOrigin_Func(FTossOriginInformation NewTossOrigin)
{
	// Invalid origin is not allowed
	if (!NewTossOrigin.IsValid()) {
		return;
	}

	// Set new Toss Origin info
	TossOrigin = NewTossOrigin;
}

#pragma endregion

#pragma region TossPrevision

bool UReplicatedTossComponent::TracePrevisionTrajectory(const FReplicatedTossPrevisionParameters PrevisionParams, const FVector SimpleDirection, const bool bUseGoalLocation, const FVector GoalLocation, FVector& TossForceResult, const float ForceMulti)
{
	// Do not trace if component is disabled or there is no instigator or there is no TossOrigin (required)
	if (!ReplicatedTossEnabled || !GetOwner()->GetInstigator() || !GetTossOrigin()) {
		return false;
	}

	// This function happen locally to avoid network overload, its result will be then replicated over the network. Since the server doesn't own any playable character it doesn't need to run this function.
	if (UKismetSystemLibrary::IsDedicatedServer(this) && GetNetMode() == ENetMode::NM_DedicatedServer) {
		return false;
	}

	// Create a new visualization Niagara component if not valid already. Will happen only locally if "bReplicatesPrevision" is disabled.
	if (GetOwner()->GetInstigator()->IsLocallyControlled() || bReplicatesPrevision) {
		if (!CurrentTossAimTrailComponent) {
			if (PrevisionParams.TossAimTrail) {
				CurrentTossAimTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(PrevisionParams.TossAimTrail, GetTossOrigin(), FName(), GetTossOriginLocation(), GetTossOriginRotation(), FVector(1.0f), EAttachLocation::KeepWorldPosition, true, ENCPoolMethod::None, true);
			}
		}
	}

	bool bCanDoPrevision = false;
	const FVector StartLoc = GetTossOriginLocation();

	// If this actor instance is not locally controlled, therefore it's a SimulatedProxy of any sort, just update visualization points. Otherwise run the portion of this function responsable for local prevision.
	if (!GetOwner()->GetInstigator()->IsLocallyControlled()) {
		if (ArchOutPoints.Num() > 0) {
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(CurrentTossAimTrailComponent, FName("User.PointArray"), ArchOutPoints);
		}
	}
	else {
		// Ignore self and attached actors
		TArray<AActor*> ActorsToIgnore = TArray<AActor*>{ GetOwner() };
		GetOwner()->GetAttachedActors(ActorsToIgnore, false, true);		

		// Prepare variable for results
		FPredictProjectilePathResult PathResult = FPredictProjectilePathResult();

		// Set traced channels
		FCollisionResponseParams SuggestionParam = FCollisionResponseParams();
		SuggestionParam.CollisionResponse.SetAllChannels(ECollisionResponse::ECR_Ignore);
		SuggestionParam.CollisionResponse.SetResponse(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

		FVector SuggestedTossForce = FVector();
		bool UsePointsAnyway = false;

		// Set prevision parameters
		FPredictProjectilePathParams ProjParam = FPredictProjectilePathParams();
		ProjParam.bTraceComplex = PrevisionParams.bTraceComplex;
		ProjParam.OverrideGravityZ = PrevisionParams.OverrideGravityZ;
		ProjParam.ProjectileRadius = PrevisionParams.ProjectileRadius;
		ProjParam.SimFrequency = PrevisionParams.SimFrequency;
		ProjParam.StartLocation = StartLoc;
		ProjParam.bTraceWithCollision = PrevisionParams.bTraceWithCollision;
		ProjParam.ObjectTypes = PrevisionParams.ObjectTypes;
		ProjParam.bTraceWithChannel = PrevisionParams.bTraceWithChannel;
		ProjParam.TraceChannel = PrevisionParams.TraceChannel;
		ProjParam.DrawDebugType = (PrevisionParams.DrawDebug) ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		ProjParam.MaxSimTime = PrevisionParams.MaxSimTime;
		ProjParam.ActorsToIgnore = ActorsToIgnore;

		// Select prevision type
		if (bUseGoalLocation) {
			bCanDoPrevision = UGameplayStatics::SuggestProjectileVelocity(this, SuggestedTossForce, GetTossOriginLocation(), GoalLocation, ((bIsUsingArch) ? PrevisionParams.MaxArchForce : PrevisionParams.MaxLinearForce) * ForceMulti, bIsUsingArch, 2.0f, 0.0f, ESuggestProjVelocityTraceOption::DoNotTrace, SuggestionParam, ActorsToIgnore);
			if (bCanDoPrevision) {
				TossForceResult = SuggestedTossForce;
				LastValidPrevisionForce = SuggestedTossForce;
			}
			else {
				if (UGameplayStatics::SuggestProjectileVelocity(this, SuggestedTossForce, GetTossOriginLocation(), GoalLocation, ((bIsUsingArch) ? PrevisionParams.MaxArchForce : PrevisionParams.MaxLinearForce) * ForceMulti, bIsUsingArch, 2.0f, 0.0f, ESuggestProjVelocityTraceOption::DoNotTrace, SuggestionParam, ActorsToIgnore)) {
					UsePointsAnyway = true;
				}
				else {
					UGameplayStatics::SuggestProjectileVelocity(this, SuggestedTossForce, GetTossOriginLocation(), GoalLocation, 1000000.0f, false, 0.0f, 0.0f, ESuggestProjVelocityTraceOption::DoNotTrace, SuggestionParam, ActorsToIgnore);
				}
			}

			// Set prevision velocity
			ProjParam.LaunchVelocity = SuggestedTossForce;
			UGameplayStatics::PredictProjectilePath(this, ProjParam, PathResult);
		}
		else {
			// Set prevision velocity
			FPredictProjectilePathResult SimplePathResult = FPredictProjectilePathResult();
			FPredictProjectilePathParams SimpleProjParam = ProjParam;
			SimpleProjParam.LaunchVelocity = SimpleDirection * ForceMulti * ((bIsUsingArch) ? PrevisionParams.MaxArchForce : PrevisionParams.MaxLinearForce);
			;

			if (UGameplayStatics::PredictProjectilePath(this, SimpleProjParam, SimplePathResult)) {
				bCanDoPrevision = UGameplayStatics::SuggestProjectileVelocity(this, SuggestedTossForce, GetTossOriginLocation(), SimplePathResult.HitResult.ImpactPoint, ((bIsUsingArch) ? PrevisionParams.MaxArchForce : PrevisionParams.MaxLinearForce) * ForceMulti, bIsUsingArch, 2.0f, 0.0f, ESuggestProjVelocityTraceOption::DoNotTrace, SuggestionParam, ActorsToIgnore);
			}
			else {
				SuggestedTossForce = SimpleDirection * ForceMulti * ((bIsUsingArch) ? PrevisionParams.MaxArchForce : PrevisionParams.MaxLinearForce);
				UsePointsAnyway = true;
			}
			
			ProjParam.LaunchVelocity = SuggestedTossForce;	
			UGameplayStatics::PredictProjectilePath(this, ProjParam, PathResult);		

			TossForceResult = SuggestedTossForce;
			LastValidPrevisionForce = SuggestedTossForce;
		}		

		// Update Niagara Component with new points
		if (CurrentTossAimTrailComponent) {
			TArray<FVector> OutPoints = TArray<FVector>();

			if (bCanDoPrevision || UsePointsAnyway) {
				OutPoints.Init(FVector(), PathResult.PathData.Num());

				ParallelFor(PathResult.PathData.Num(), [&](int32 Idx) {
					OutPoints[Idx] = PathResult.PathData[Idx].Location;
					});

				OutPoints.Insert(StartLoc, 0);
			}
			else {
				OutPoints.Add(StartLoc);
				OutPoints.Add(GoalLocation);
			}

			ArchOutPoints = OutPoints;

			// Replicate new points on server if needed (is client)
			if (!GetOwner()->GetInstigator()->HasAuthority() && bReplicatesPrevision) {
				UpdateArchOnServer(ArchOutPoints);
			}

			// Update points
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(CurrentTossAimTrailComponent, FName("User.PointArray"), OutPoints);
		}

		const EReplicatedTossPrevisionValidity PrevVal = bPrevisionIsValidCurrently;
		bPrevisionIsValidCurrently = (bCanDoPrevision) ? EReplicatedTossPrevisionValidity::PV_True : EReplicatedTossPrevisionValidity::PV_False;

		// Update variables and trigger delegates / onreps
		if (bPrevisionIsValidCurrently != PrevVal) {
			if (GetOwner()->GetLocalRole() != ENetRole::ROLE_Authority) {
				if (GetOwner()->GetLocalRole() == ENetRole::ROLE_AutonomousProxy) {
					OnPrevisionValidityChange.Broadcast(bPrevisionIsValidCurrently);
					SetPrevisionIsValidCurrentlyOnServer(bCanDoPrevision);
				}
			}
			else {
				OnPrevisionValidityChange.Broadcast(bPrevisionIsValidCurrently);
			}
		}
	}

	return bCanDoPrevision;
}

#pragma endregion

#pragma region TossAction

void UReplicatedTossComponent::TossActor(AActor* TargetActor, const FVector TossForce, const float ForceMulti)
{
	// Replication logics

	if (!TargetActor || TossForce.IsZero() || !GetTossOrigin()) {
		return;
	}

	if (GetOwnerRole() == ENetRole::ROLE_Authority) {
		TossActor_Func(TargetActor, TossForce, ForceMulti);
	}
	else {
		TossActor_Server(TargetActor, TossForce, ForceMulti);
	}
}

void UReplicatedTossComponent::SpawnAndTossActor(TSubclassOf<AActor> ActorClass, const FVector TossForce, const float ForceMulti)
{
	// Replication logics

	if (!ActorClass || TossForce.IsZero() || !GetTossOrigin()) {
		return;
	}

	if (GetOwnerRole() == ENetRole::ROLE_Authority) {
		SpawnAndTossActor_Func(ActorClass, TossForce, ForceMulti);
	}
	else {
		SpawnAndTossActor_Server(ActorClass, TossForce, ForceMulti);
	}
}

void UReplicatedTossComponent::TossActor_Server_Implementation(AActor* TargetActor, const FVector TossForce, const float ForceMulti)
{
	// From client to server function

	TossActor_Func(TargetActor, TossForce, ForceMulti);
}

void UReplicatedTossComponent::TossActor_Func(AActor* TargetActor, const FVector TossForce, const float ForceMulti)
{
	if (!TargetActor || TossForce.IsZero() || !GetTossOrigin()) {
		return;
	}

	UPrimitiveComponent* TargetComponent = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());

	if (!TargetComponent) {
		return;
	}

	if (!TargetComponent->IsSimulatingPhysics()) {
		if (bAlwaysEnablePhysics) {
			TargetComponent->SetSimulatePhysics(true);
		}
		else {
			return;
		}
	}
	
	// Toss actor
	TargetComponent->SetWorldLocationAndRotation(GetTossOriginLocation(), GetTossOriginRotation(), false, (FHitResult*)nullptr, ETeleportType::ResetPhysics);
	TargetComponent->SetAllPhysicsLinearVelocity(TossForce, false); // Thanks to JoshCulpArt for this feedback
	OnActorTossed.Broadcast(TargetActor, TargetComponent, TossForce);
}

void UReplicatedTossComponent::SpawnAndTossActor_Server_Implementation(TSubclassOf<AActor> ActorClass, const FVector TossForce, const float ForceMulti)
{
	// From client to server function

	SpawnAndTossActor_Func(ActorClass, TossForce, ForceMulti);
}

void UReplicatedTossComponent::SpawnAndTossActor_Func(TSubclassOf<AActor> ActorClass, const FVector TossForce, const float ForceMulti)
{
	FActorSpawnParameters SpawnParam = FActorSpawnParameters();
	SpawnParam.bNoFail = true;
	SpawnParam.Owner = GetOwner();
	SpawnParam.Instigator = GetOwner()->GetInstigator();

	// Spawn & Toss
	AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, GetTossOriginTransform(), SpawnParam);
	TossActor_Func(NewActor, TossForce, ForceMulti);
}

#pragma endregion

#pragma region Visualization

void UReplicatedTossComponent::UpdateArchOnServer_Implementation(const TArray<FVector>& Points)
{
	// From client to server function

	ArchOutPoints = Points;
}

UNiagaraComponent* UReplicatedTossComponent::GetAimTrail() const
{
	return CurrentTossAimTrailComponent;
}

#pragma endregion

