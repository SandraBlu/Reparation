// Fill out your copyright notice in the Description page of Project Settings.


#include "Locomotion/RLocomotionComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "KismetAnimationLibrary.h"
#include "RGameplayTags.h"
#include "GameFramework/Character.h"
#include "Locomotion/RCharacterMovementComponent.h"
#include "Locomotion/RLocomotionConfig.h"

URLocomotionComponent::URLocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void URLocomotionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ACharacter>(GetOwner());
	if (!OwningCharacter)
	{
		SetComponentTickEnabled(false);
		return;
	}

	MovementComponent = Cast<URCharacterMovementComponent>(OwningCharacter->GetCharacterMovement());
	if (!MovementComponent)
	{
		// Without the custom movement component there is nothing to drive. The
		// character must select URCharacterMovementComponent in its constructor.
		SetComponentTickEnabled(false);
		return;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();
	MovementComponent->JumpZVelocity = Cfg.JumpZVelocity;
	MovementComponent->AirControl = Cfg.AirControl;
	MovementComponent->ApplyGaitSettings(Cfg.GetGaitSettings(CurrentGait, CurrentStance));
	MovementComponent->ApplySwimSettings(Cfg.SwimSurfaceSpeed, Cfg.SwimAcceleration);

	OwningCharacter->LandedDelegate.AddDynamic(this, &URLocomotionComponent::HandleLanded);
	OwningCharacter->MovementModeChangedDelegate.AddDynamic(this, &URLocomotionComponent::HandleMovementModeChanged);

	ApplyStateTags(CurrentState, CurrentState);
}

void URLocomotionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwningCharacter)
	{
		OwningCharacter->LandedDelegate.RemoveDynamic(this, &URLocomotionComponent::HandleLanded);
		OwningCharacter->MovementModeChangedDelegate.RemoveDynamic(this, &URLocomotionComponent::HandleMovementModeChanged);
	}

	if (UAbilitySystemComponent* ASC = GetOwnerASC())
	{
		for (const FGameplayTag& Tag : AppliedTags)
		{
			ASC->RemoveLooseGameplayTag(Tag);
		}
	}
	AppliedTags.Reset();

	Super::EndPlay(EndPlayReason);
}

void URLocomotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwningCharacter || !MovementComponent)
	{
		return;
	}

	StateLockRemaining = FMath::Max(0.f, StateLockRemaining - DeltaTime);
	TimeFalling = MovementComponent->IsFalling() ? TimeFalling + DeltaTime : 0.f;

	UpdateGait();
	UpdateVolumetricModes(DeltaTime);
	EnterState(EvaluateDesiredState());
	ApplyMovementSettings();

	// Vertical swim input has no equivalent on land, so it is applied here rather
	// than in the pawn's input handler.
	if (MovementComponent->IsSwimming() && !FMath::IsNearlyZero(SwimVerticalInput))
	{
		OwningCharacter->AddMovementInput(FVector::UpVector, SwimVerticalInput);
	}

	UpdateAnimData(DeltaTime);
}

// --- Input ------------------------------------------------------------------

void URLocomotionComponent::SetMovementInput(const FVector2D& InInput)
{
	MovementInput = InInput;
}

void URLocomotionComponent::SetSprintHeld(bool bHeld)
{
	bSprintHeld = bHeld;
}

void URLocomotionComponent::SetWalkHeld(bool bHeld)
{
	bWalkHeld = bHeld;
}

void URLocomotionComponent::ToggleCrouch()
{
	if (!OwningCharacter || !MovementComponent)
	{
		return;
	}

	// Crouching in air or underwater has no meaning and would fight the mode.
	if (!MovementComponent->IsMovingOnGround())
	{
		return;
	}

	if (CurrentStance == ERStance::Crouching)
	{
		OwningCharacter->UnCrouch();
		CurrentStance = ERStance::Standing;
	}
	else
	{
		OwningCharacter->Crouch();
		CurrentStance = ERStance::Crouching;
	}
}

bool URLocomotionComponent::TryJump()
{
	if (!OwningCharacter || !MovementComponent)
	{
		return false;
	}

	// A scripted traversal owns the character until it completes.
	if (MovementComponent->IsTraversing())
	{
		return false;
	}

	// While gliding, the jump key folds the glider away.
	if (MovementComponent->IsGliding())
	{
		bGlideHeld = false;
		MovementComponent->SetMovementMode(MOVE_Falling);
		return true;
	}

	// While climbing, it pushes off the wall.
	if (MovementComponent->IsClimbing())
	{
		const FVector Away = MovementComponent->GetClimbSurfaceNormal() * GetConfigRef().ClimbJumpAwaySpeed;
		bClimbHeld = false;
		MovementComponent->SetMovementMode(MOVE_Falling);
		OwningCharacter->LaunchCharacter(Away + FVector::UpVector * GetConfigRef().JumpZVelocity, true, true);
		return true;
	}

	// In water, jump means push toward the surface.
	if (MovementComponent->IsSwimming())
	{
		MovementComponent->Velocity.Z = FMath::Max(MovementComponent->Velocity.Z, GetConfigRef().WaterExitBoost);
		return true;
	}

	// Stand up first rather than refusing silently.
	if (CurrentStance == ERStance::Crouching)
	{
		ToggleCrouch();
		return false;
	}

	// An obstacle in front is crossed rather than jumped into. This is what makes
	// running at a log or a low wall read as a vault.
	if (TryTraversal())
	{
		return true;
	}

	if (!MovementComponent->IsMovingOnGround())
	{
		// Airborne, the jump key deploys the glider if one is carried.
		if (CanDeployGlider())
		{
			bGlideHeld = true;
			return true;
		}
		return false;
	}

	bJumpInitiated = true;
	OwningCharacter->Jump();
	return true;
}

void URLocomotionComponent::StopJumping()
{
	if (OwningCharacter)
	{
		OwningCharacter->StopJumping();
	}
}

void URLocomotionComponent::SetSwimVerticalInput(float InVertical)
{
	SwimVerticalInput = FMath::Clamp(InVertical, -1.f, 1.f);
}

void URLocomotionComponent::SetGlideHeld(bool bHeld)
{
	bGlideHeld = bHeld;
}

void URLocomotionComponent::SetClimbHeld(bool bHeld)
{
	bClimbHeld = bHeld;
}

void URLocomotionComponent::SetGliderEquipped(bool bEquipped)
{
	bGliderEquipped = bEquipped;

	// Losing the glider mid flight drops the character rather than leaving them
	// gliding on equipment they no longer have.
	if (!bGliderEquipped && MovementComponent && MovementComponent->IsGliding())
	{
		bGlideHeld = false;
		MovementComponent->SetMovementMode(MOVE_Falling);
	}
}

bool URLocomotionComponent::CanDeployGlider() const
{
	if (!MovementComponent || !MovementComponent->IsFalling())
	{
		return false;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();
	if (Cfg.bRequireGliderEquipped && !bGliderEquipped)
	{
		return false;
	}

	// A short hop should not pop the glider open.
	return TimeFalling >= Cfg.MinFallTimeBeforeGlide;
}

void URLocomotionComponent::ToggleFlight()
{
	SetFlying(!(MovementComponent && MovementComponent->MovementMode == MOVE_Flying));
}

void URLocomotionComponent::SetFlying(bool bEnabled)
{
	if (!MovementComponent)
	{
		return;
	}

	if (bEnabled)
	{
		const URLocomotionConfig& Cfg = GetConfigRef();
		MovementComponent->ApplyFlySettings(Cfg.FlyMaxSpeed, Cfg.FlyAcceleration, Cfg.FlyBrakingDeceleration);
		MovementComponent->SetMovementMode(MOVE_Flying);
	}
	else if (MovementComponent->MovementMode == MOVE_Flying)
	{
		MovementComponent->SetMovementMode(MOVE_Falling);
	}
}

bool URLocomotionComponent::TryTraversal()
{
	if (!MovementComponent || MovementComponent->IsTraversing() || MovementComponent->IsSwimming())
	{
		return false;
	}

	FVector Start = FVector::ZeroVector;
	FVector Mid = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	ERLocomotionState Type = ERLocomotionState::Vault;

	if (!MovementComponent->FindTraversal(Start, Mid, End, Type))
	{
		return false;
	}

	ActiveTraversalType = Type;

	const float Duration = Type == ERLocomotionState::Vault
		? MovementComponent->VaultDuration
		: MovementComponent->MantleDuration;

	// Clear first so the traversal state is never refused by a stale lock, then
	// hold it for the whole move.
	StateLockRemaining = 0.f;
	MovementComponent->BeginTraversal(Start, Mid, End, Duration);
	EnterState(Type);
	StateLockRemaining = FMath::Max(StateLockRemaining, Duration);

	return true;
}

void URLocomotionComponent::UpdateVolumetricModes(float DeltaTime)
{
	const URLocomotionConfig& Cfg = GetConfigRef();

	// A scripted traversal runs to completion without interference.
	if (MovementComponent->IsTraversing())
	{
		return;
	}

	if (MovementComponent->IsClimbing())
	{
		if (!bClimbHeld)
		{
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
		else if (Cfg.bAutoMantleAtClimbLedge && MovementComponent->IsAtClimbLedge())
		{
			// Topping out hands over to a mantle instead of hovering at the lip.
			TryTraversal();
		}
		return;
	}

	// Grab on when the climb key is held and there is something to grab.
	if (bClimbHeld && !MovementComponent->IsSwimming() && MovementComponent->MovementMode != MOVE_Flying)
	{
		FHitResult Surface;
		if (MovementComponent->FindClimbableSurface(Surface))
		{
			MovementComponent->SetCustomMovementMode(ERCustomMovementMode::Climb);
			return;
		}
	}

	if (MovementComponent->IsGliding())
	{
		if (!bGlideHeld || MovementComponent->IsSwimming())
		{
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
		return;
	}

	if (bGlideHeld && CanDeployGlider())
	{
		MovementComponent->SetCustomMovementMode(ERCustomMovementMode::Glide);
	}
}

// --- Queries ----------------------------------------------------------------

bool URLocomotionComponent::IsInWater() const
{
	return MovementComponent && MovementComponent->IsSwimming();
}

bool URLocomotionComponent::RequestState(ERLocomotionState NewState)
{
	return EnterState(NewState);
}

// --- State machine ----------------------------------------------------------

const URLocomotionConfig& URLocomotionComponent::GetConfigRef() const
{
	return Config ? *Config : *GetDefault<URLocomotionConfig>();
}

ERLocomotionState URLocomotionComponent::EvaluateDesiredState() const
{
	if (!MovementComponent)
	{
		return CurrentState;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();

	// Custom and scripted modes report themselves; nothing else may override them.
	if (MovementComponent->IsTraversing())
	{
		return ActiveTraversalType;
	}

	if (MovementComponent->IsClimbing())
	{
		return ERLocomotionState::Climb;
	}

	if (MovementComponent->IsGliding())
	{
		return ERLocomotionState::Glide;
	}

	if (MovementComponent->MovementMode == MOVE_Flying)
	{
		return ERLocomotionState::Fly;
	}

	// Water takes precedence; entering it interrupts anything happening on land.
	if (MovementComponent->IsSwimming())
	{
		const float Immersion = MovementComponent->GetImmersionFraction();

		// Hysteresis keeps bobbing at the surface from flipping state every frame.
		if (CurrentState == ERLocomotionState::SwimUnderwater)
		{
			return Immersion < Cfg.UnderwaterImmersionThreshold - Cfg.ImmersionHysteresis
				? ERLocomotionState::SwimSurface
				: ERLocomotionState::SwimUnderwater;
		}

		return Immersion >= Cfg.UnderwaterImmersionThreshold
			? ERLocomotionState::SwimUnderwater
			: ERLocomotionState::SwimSurface;
	}

	if (MovementComponent->IsFalling())
	{
		// Only the ascent of a deliberate jump reads as Jump; a walked-off ledge falls.
		const bool bRising = MovementComponent->Velocity.Z > 0.f;
		return bJumpInitiated && bRising ? ERLocomotionState::Jump : ERLocomotionState::Fall;
	}

	if (MovementComponent->IsMovingOnGround())
	{
		if (CurrentStance == ERStance::Crouching)
		{
			return ERLocomotionState::Crouch;
		}

		if (MovementComponent->Velocity.Size2D() <= Cfg.IdleSpeedThreshold)
		{
			return ERLocomotionState::Idle;
		}

		switch (CurrentGait)
		{
		case ERGait::Walk:		return ERLocomotionState::Walk;
		case ERGait::Sprint:	return ERLocomotionState::Sprint;
		case ERGait::Run:
		default:				return ERLocomotionState::Run;
		}
	}

	return CurrentState;
}

bool URLocomotionComponent::EnterState(ERLocomotionState NewState)
{
	if (NewState == CurrentState)
	{
		return false;
	}

	// Land and Roll hold the character briefly so the recovery animation reads.
	if (StateLockRemaining > 0.f)
	{
		return false;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();
	const FRLocomotionTransition* Transition = Cfg.FindTransition(CurrentState, NewState);
	if (Transition && Transition->bBlocked)
	{
		return false;
	}

	const ERLocomotionState PreviousState = CurrentState;
	CurrentState = NewState;

	AnimData.State = NewState;
	AnimData.TimeInState = 0.f;
	StateLockRemaining = Transition ? Transition->LockDuration : 0.f;

	if (Transition && Transition->TransitionMontage && OwningCharacter)
	{
		OwningCharacter->PlayAnimMontage(Transition->TransitionMontage);
	}

	ApplyStateTags(PreviousState, NewState);
	OnLocomotionStateChanged.Broadcast(PreviousState, NewState);

	return true;
}

void URLocomotionComponent::UpdateGait()
{
	const URLocomotionConfig& Cfg = GetConfigRef();

	if (bWalkHeld)
	{
		CurrentGait = ERGait::Walk;
		return;
	}

	// Sprinting sideways or backwards reads badly, so require committed input.
	const bool bCanSprint = bSprintHeld
		&& CurrentStance == ERStance::Standing
		&& MovementInput.Size() >= Cfg.SprintInputThreshold;

	CurrentGait = bCanSprint ? ERGait::Sprint : ERGait::Run;
}

void URLocomotionComponent::ApplyMovementSettings()
{
	if (!MovementComponent)
	{
		return;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();

	// Glide, climb and traversal own their own speed and acceleration; writing
	// gait settings over them would fight GetMaxSpeed every frame.
	if (MovementComponent->MovementMode == MOVE_Custom)
	{
		return;
	}

	if (MovementComponent->IsSwimming())
	{
		const float SwimSpeed = CurrentState == ERLocomotionState::SwimUnderwater
			? Cfg.SwimUnderwaterSpeed
			: Cfg.SwimSurfaceSpeed;

		MovementComponent->ApplySwimSettings(SwimSpeed, Cfg.SwimAcceleration);
		return;
	}

	if (MovementComponent->MovementMode == MOVE_Flying)
	{
		MovementComponent->ApplyFlySettings(Cfg.FlyMaxSpeed, Cfg.FlyAcceleration, Cfg.FlyBrakingDeceleration);
		return;
	}

	MovementComponent->ApplyGaitSettings(Cfg.GetGaitSettings(CurrentGait, CurrentStance));
}

void URLocomotionComponent::UpdateAnimData(float DeltaTime)
{
	const FVector Velocity = MovementComponent->Velocity;

	AnimData.State = CurrentState;
	AnimData.Gait = CurrentGait;
	AnimData.Stance = CurrentStance;
	AnimData.GroundSpeed = Velocity.Size2D();
	AnimData.VerticalSpeed = Velocity.Z;
	AnimData.Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwningCharacter->GetActorRotation());
	AnimData.bHasMovementInput = !MovementInput.IsNearlyZero();
	AnimData.bIsGrounded = MovementComponent->IsMovingOnGround();
	AnimData.bIsInWater = MovementComponent->IsSwimming();
	AnimData.ImmersionDepth = MovementComponent->GetImmersionFraction();
	AnimData.TimeInState += DeltaTime;
}

// --- Tags -------------------------------------------------------------------

bool URLocomotionComponent::IsGroundState(ERLocomotionState State)
{
	return State == ERLocomotionState::Idle
		|| State == ERLocomotionState::Walk
		|| State == ERLocomotionState::Run
		|| State == ERLocomotionState::Sprint
		|| State == ERLocomotionState::Crouch
		|| State == ERLocomotionState::Land
		|| State == ERLocomotionState::Roll;
}

bool URLocomotionComponent::IsWaterState(ERLocomotionState State)
{
	return State == ERLocomotionState::SwimSurface || State == ERLocomotionState::SwimUnderwater;
}

FGameplayTagContainer URLocomotionComponent::GetTagsForState(ERLocomotionState State) const
{
	const FRGameplayTags& Tags = FRGameplayTags::Get();
	FGameplayTagContainer Result;

	// Climb and traversal are neither grounded nor airborne in any useful sense,
	// so they get their own category rather than being forced into one.
	switch (State)
	{
	case ERLocomotionState::Climb:
		Result.AddTag(Tags.state_locomotion_climbing);
		return Result;

	case ERLocomotionState::Vault:
	case ERLocomotionState::Mantle:
		Result.AddTag(Tags.state_locomotion_traversing);
		return Result;

	default:
		break;
	}

	if (IsGroundState(State))
	{
		Result.AddTag(Tags.state_locomotion_grounded);
	}
	else if (IsWaterState(State))
	{
		Result.AddTag(Tags.state_locomotion_swimming);
		if (State == ERLocomotionState::SwimUnderwater)
		{
			Result.AddTag(Tags.state_locomotion_underwater);
		}
	}
	else
	{
		Result.AddTag(Tags.state_locomotion_inAir);
	}

	switch (State)
	{
	case ERLocomotionState::Sprint:
		Result.AddTag(Tags.state_locomotion_sprinting);
		break;
	case ERLocomotionState::Crouch:
		Result.AddTag(Tags.state_locomotion_crouching);
		break;
	case ERLocomotionState::Land:
	case ERLocomotionState::Roll:
		Result.AddTag(Tags.state_locomotion_landing);
		break;
	case ERLocomotionState::Fly:
		Result.AddTag(Tags.state_locomotion_flying);
		break;
	case ERLocomotionState::Glide:
		Result.AddTag(Tags.state_locomotion_gliding);
		break;
	default:
		break;
	}

	return Result;
}

void URLocomotionComponent::ApplyStateTags(ERLocomotionState PreviousState, ERLocomotionState NewState)
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	const FGameplayTagContainer DesiredTags = GetTagsForState(NewState);

	for (const FGameplayTag& Tag : AppliedTags)
	{
		if (!DesiredTags.HasTagExact(Tag))
		{
			ASC->RemoveLooseGameplayTag(Tag);
		}
	}

	for (const FGameplayTag& Tag : DesiredTags)
	{
		if (!AppliedTags.HasTagExact(Tag))
		{
			ASC->AddLooseGameplayTag(Tag);
		}
	}

	AppliedTags = DesiredTags;
}

UAbilitySystemComponent* URLocomotionComponent::GetOwnerASC() const
{
	if (const IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		return AbilityInterface->GetAbilitySystemComponent();
	}

	return nullptr;
}

// --- Movement component callbacks -------------------------------------------

void URLocomotionComponent::HandleLanded(const FHitResult& Hit)
{
	bJumpInitiated = false;
	TimeFalling = 0.f;

	// Fold the glider on touchdown so the next fall does not auto deploy it.
	bGlideHeld = false;

	// Velocity.Z is already zeroed by the time this fires, so use the value
	// cached on the last tick while still airborne.
	const float ImpactSpeed = FMath::Abs(AnimData.VerticalSpeed);
	AnimData.LastLandingImpactSpeed = ImpactSpeed;

	const URLocomotionConfig& Cfg = GetConfigRef();

	// A light landing needs no dedicated state; the ground states read fine.
	if (ImpactSpeed < Cfg.SoftLandingSpeed)
	{
		return;
	}

	const ERLocomotionState LandingState = ImpactSpeed >= Cfg.HardLandingSpeed
		? ERLocomotionState::Roll
		: ERLocomotionState::Land;

	// Clear any residual lock so the landing itself is never swallowed.
	StateLockRemaining = 0.f;

	if (EnterState(LandingState))
	{
		StateLockRemaining = FMath::Max(StateLockRemaining, Cfg.LandRecoveryTime);
	}
}

void URLocomotionComponent::HandleMovementModeChanged(ACharacter* Character, EMovementMode PreviousMode, uint8 PreviousCustomMode)
{
	if (!MovementComponent)
	{
		return;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();

	// Entering water: swap to swim handling immediately so the first stroke is
	// not evaluated with land acceleration.
	if (MovementComponent->IsSwimming())
	{
		MovementComponent->ApplySwimSettings(Cfg.SwimSurfaceSpeed, Cfg.SwimAcceleration);
		bJumpInitiated = false;
		StateLockRemaining = 0.f;
		return;
	}

	// Leaving water: restore gait handling and drop any dive input still held.
	if (PreviousMode == MOVE_Swimming)
	{
		SwimVerticalInput = 0.f;
		MovementComponent->ApplyGaitSettings(Cfg.GetGaitSettings(CurrentGait, CurrentStance));
	}
}
