// Fill out your copyright notice in the Description page of Project Settings.


#include "Locomotion/RLocomotionComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Animation/AnimMontage.h"
#include "KismetAnimationLibrary.h"
#include "RGameplayTags.h"
#include "GameFramework/Character.h"
#include "Locomotion/RCharacterMovementComponent.h"
#include "Locomotion/RLocomotionConfig.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CVarDebugLocomotion(
	TEXT("r.Reparation.DebugLocomotion"),
	0,
	TEXT("Draw the locomotion state on screen. 1 to enable."),
	ECVF_Cheat);
#endif

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
		UE_LOG(LogTemp, Error, TEXT("%hs: owner [%s] is not a Character, so locomotion is disabled."), __FUNCTION__, *GetNameSafe(GetOwner()));
		SetComponentTickEnabled(false);
		return;
	}

	MovementComponent = Cast<URCharacterMovementComponent>(OwningCharacter->GetCharacterMovement());
	if (!MovementComponent)
	{
		// Without the custom movement component there is nothing to drive. The
		// character must select URCharacterMovementComponent in its constructor.
		UE_LOG(LogTemp, Error, TEXT("%hs: [%s] does not use URCharacterMovementComponent, so locomotion is disabled. Add SetDefaultSubobjectClass<URCharacterMovementComponent>(ACharacter::CharacterMovementComponentName) to its constructor."), __FUNCTION__, *GetNameSafe(OwningCharacter));
		SetComponentTickEnabled(false);
		return;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();
	MovementComponent->JumpZVelocity = Cfg.JumpZVelocity;
	MovementComponent->AirControl = Cfg.AirControl;
	MovementComponent->MaxAscendableSlopeAngle = Cfg.SlideMinSlopeAngle;
	MovementComponent->ApplyGaitSettings(Cfg.GetGaitSettings(CurrentGait, CurrentStance));
	MovementComponent->ApplySwimSettings(Cfg.SwimSurfaceSpeed, Cfg.SwimAcceleration);

	// Tick after the movement component, not merely in the same group. Without
	// this the state machine reads the previous frame's movement mode, so a jump
	// spends a frame still reported as running before it reads as airborne.
	AddTickPrerequisiteComponent(MovementComponent);

	MovementComponent->OnAirborneImpact.AddUObject(this, &URLocomotionComponent::HandleAirborneImpact);

	OwningCharacter->LandedDelegate.AddDynamic(this, &URLocomotionComponent::HandleLanded);
	OwningCharacter->MovementModeChangedDelegate.AddDynamic(this, &URLocomotionComponent::HandleMovementModeChanged);

	PreviousYaw = OwningCharacter->GetActorRotation().Yaw;

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
	// Time airborne, not merely time in MOVE_Falling. Gliding and skydiving are
	// both off the ground, and resetting this on entering them would fail the
	// fall time gate that lets a wing open out of a skydive.
	TimeFalling = (MovementComponent->IsFalling() || MovementComponent->IsGliding() || MovementComponent->IsSkydiving())
		? TimeFalling + DeltaTime
		: 0.f;

	UpdateStance();
	UpdateGait();
	UpdateVolumetricModes(DeltaTime);
	EnterState(EvaluateDesiredState());
	UpdateStrafe();
	ApplyMovementSettings();
	UpdateSlide(DeltaTime);

	// Climbing input is applied here because it has to be built against the wall,
	// not the camera. Forward goes up the face, right goes across it.
	if (MovementComponent->IsClimbing() && !MovementInput.IsNearlyZero())
	{
		const FVector Normal = MovementComponent->GetClimbSurfaceNormal();
		const FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, Normal).GetSafeNormal();

		// The character is already rotated to face the wall, so its right vector
		// lies in the plane. This matches how ClimbRightSpeed is measured.
		OwningCharacter->AddMovementInput(WallUp, MovementInput.Y);
		OwningCharacter->AddMovementInput(OwningCharacter->GetActorRightVector(), MovementInput.X);
	}

	// Vertical swim input has no equivalent on land, so it is applied here rather
	// than in the pawn's input handler.
	if (MovementComponent->IsSwimming() && !FMath::IsNearlyZero(SwimVerticalInput))
	{
		OwningCharacter->AddMovementInput(FVector::UpVector, SwimVerticalInput);
	}

	UpdateAnimData(DeltaTime);

#if !UE_BUILD_SHIPPING
	DrawDebugState();
#endif
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

	// Only a request. Crouch and UnCrouch set an intent the movement component
	// applies, and standing up is refused when there is no headroom. UpdateStance
	// reads back whatever it settles on.
	if (OwningCharacter->bIsCrouched)
	{
		OwningCharacter->UnCrouch();
	}
	else
	{
		OwningCharacter->Crouch();
	}
}

bool URLocomotionComponent::TryJump()
{
	if (!OwningCharacter || !MovementComponent)
	{
		return false;
	}

	// Timed here, not in the airborne branch, so the press that launches the jump
	// starts the window. Otherwise deploying would take three taps.
	const float Now = GetWorld()->GetTimeSeconds();
	const bool bDoubleTap = (Now - LastJumpPressTime) <= GetConfigRef().GlideToggleDoubleTapWindow;
	LastJumpPressTime = Now;

	// A scripted traversal owns the character until it completes.
	if (MovementComponent->IsTraversing())
	{
		return false;
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
		// Airborne, a double tap toggles the wing.
		if (!bDoubleTap)
		{
			return false;
		}

		// The wing only. Skydiving is not deployed, it is what falling becomes once
		// the skill is known, so stowing here drops back into it rather than into a
		// plain fall.
		if (bGlideHeld)
		{
			bGlideHeld = false;
			if (MovementComponent->IsGliding())
			{
				MovementComponent->SetMovementMode(MOVE_Falling);
			}
			return true;
		}

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

bool URLocomotionComponent::CanSkydive() const
{
	if (!MovementComponent || !MovementComponent->IsFalling())
	{
		return false;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();
	if (Cfg.SkydiveUnlockTag.IsValid())
	{
		const UAbilitySystemComponent* ASC = GetOwnerASC();
		if (!ASC || !ASC->HasMatchingGameplayTag(Cfg.SkydiveUnlockTag))
		{
			return false;
		}
	}

	// Its own gate, longer than the glider's: a hop should not throw the
	// character into a dive.
	return TimeFalling >= Cfg.MinFallTimeBeforeSkydive;
}

bool URLocomotionComponent::CanDeployGlider() const
{
	// Skydiving counts: opening the wing out of a dive is the point of carrying
	// one, and a skydive is not MOVE_Falling.
	if (!MovementComponent || !(MovementComponent->IsFalling() || MovementComponent->IsSkydiving()))
	{
		return false;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();
	if (Cfg.GlideUnlockTag.IsValid())
	{
		const UAbilitySystemComponent* ASC = GetOwnerASC();
		if (!ASC || !ASC->HasMatchingGameplayTag(Cfg.GlideUnlockTag))
		{
			return false;
		}
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

bool URLocomotionComponent::TryTraversal(ERTraversalEntry Entry)
{
	if (!MovementComponent || MovementComponent->IsTraversing() || MovementComponent->IsSwimming())
	{
		return false;
	}

	FRTraversalQuery Query;
	if (!MovementComponent->FindTraversal(Entry, Query))
	{
		return false;
	}

	// The config picks the variant from the measurements. With no table, or no
	// row that fits, the movement component's own verdict and durations stand,
	// which is what keeps traversal working before any clips exist.
	const FRTraversalAction* Action = GetConfigRef().FindTraversalAction(Query);

	const ERLocomotionState Type = Action ? Action->State : Query.FallbackState;

	// Clamped the same way BeginTraversal clamps it, so the play rate below is
	// derived from the duration the move actually runs for.
	const float Duration = FMath::Max(0.1f, Action
		? Action->Duration
		: (Type == ERLocomotionState::Vault
			? MovementComponent->VaultDuration
			: MovementComponent->MantleDuration));

	ActiveTraversalType = Type;

	// Published before EnterState so the animation graph sees the clip on the
	// same frame it is told to enter the traversal state, not one frame later.
	AnimData.TraversalAnim = Action ? Action->Animation : nullptr;

	// Held until the move completes, when HandleMovementModeChanged plays it.
	PendingTraversalLanding = Action ? Action->LandingAnimation : nullptr;
	PendingTraversalLandingRecovery = Action ? Action->LandingRecoveryTime : 0.f;

	// FindTraversalAction only returns a row whose finish has room, so the
	// chosen end is always a valid one.
	FVector End = Query.End;
	if (Action && Action->Finish == ERTraversalFinish::OnTop)
	{
		End = Query.OnTopEnd;
	}
	else if (Action && Action->Finish == ERTraversalFinish::FarSide)
	{
		End = Query.FarSideEnd;
	}

	// The traversal clips carry no root motion, so the capsule owns the movement
	// and the clip has to be stretched onto it. A rate under one slows a clip
	// shorter than the move; over one speeds up a longer one.
	float MontagePlayRate = 1.f;
	if (const FRLocomotionTransition* Transition = GetConfigRef().FindTransition(CurrentState, Type))
	{
		if (Transition->TransitionMontage)
		{
			const float ClipLength = Transition->TransitionMontage->GetPlayLength();
			if (ClipLength > KINDA_SMALL_NUMBER)
			{
				MontagePlayRate = ClipLength / Duration;
			}
		}
	}

	// Clear first so the traversal state is never refused by a stale lock, then
	// hold it for the whole move.
	StateLockRemaining = 0.f;
	MovementComponent->BeginTraversal(Query.Start, Query.Mid, End, Duration,
		Action && Action->bTurnAround,
		Action ? Action->Animation.Get() : nullptr,
		Action ? Action->ApexTime : 0.f);
	EnterState(Type, MontagePlayRate);
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
			TryTraversal(ERTraversalEntry::Climbing);
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
		return;
	}

	// Automatic. Once the skill is known, a long fall is a skydive; there is
	// nothing to press. The wing is the deliberate act on top of it.
	if (CanSkydive())
	{
		// Applied on entry because a glider pushes its own numbers onto the same
		// properties, and whichever went last would otherwise win.
		MovementComponent->ApplyGlideSettings(Cfg.SkydiveSettings);
		MovementComponent->SetCustomMovementMode(ERCustomMovementMode::Skydive);
	}
}

// --- Queries ----------------------------------------------------------------

bool URLocomotionComponent::IsClimbing() const
{
	return MovementComponent && MovementComponent->IsClimbing();
}

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

	if (MovementComponent->IsSkydiving())
	{
		return ERLocomotionState::Skydive;
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
		// Steep ground takes over everything else on foot. Hysteresis on the angle
		// so a slope hovering at the threshold cannot flicker the state.
		const float RequiredSlopeAngle = CurrentState == ERLocomotionState::Slide
			? Cfg.SlideMinSlopeAngle - Cfg.SlideSlopeAngleHysteresis
			: Cfg.SlideMinSlopeAngle;

		// Angle alone is not enough: a ramp is equally steep running up it, which
		// put the character into a slide while ascending. Gravity has to be the
		// thing moving them.
		const FVector Downhill = FVector::VectorPlaneProject(FVector::DownVector, MovementComponent->GetFloorNormal()).GetSafeNormal();
		const float DownhillSpeed = FVector::DotProduct(MovementComponent->Velocity, Downhill);

		// Starting one wants real downhill movement, so standing still never does.
		// Sustaining one only needs the sign, so a slide crossing a flatter patch is
		// not cut short, while moving back up the face always ends it.
		const float RequiredDownhillSpeed = CurrentState == ERLocomotionState::Slide
			? 0.f
			: Cfg.SlideMinDownhillSpeed;

		// Greater or equal, so standing still on a steep face slides. Landing on one
		// left the character stuck otherwise: no downhill speed to qualify, no input
		// allowed above MaxAscendableSlopeAngle, and walking mode never pulls a
		// capsule down a slope by itself.
		if (DownhillSpeed >= RequiredDownhillSpeed && MovementComponent->GetFloorAngle() >= RequiredSlopeAngle)
		{
			return ERLocomotionState::Slide;
		}

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

bool URLocomotionComponent::EnterState(ERLocomotionState NewState, float MontagePlayRate)
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
		OwningCharacter->PlayAnimMontage(Transition->TransitionMontage, MontagePlayRate);
	}

	ApplyStateTags(PreviousState, NewState);
	OnLocomotionStateChanged.Broadcast(PreviousState, NewState);

	return true;
}

#if !UE_BUILD_SHIPPING
void URLocomotionComponent::DrawDebugState() const
{
	if (CVarDebugLocomotion.GetValueOnGameThread() <= 0 || !GEngine)
	{
		return;
	}

	const FString StateName = StaticEnum<ERLocomotionState>()->GetNameStringByValue(static_cast<int64>(CurrentState));
	const FString GaitName = StaticEnum<ERGait>()->GetNameStringByValue(static_cast<int64>(CurrentGait));
	const FString StanceName = StaticEnum<ERStance>()->GetNameStringByValue(static_cast<int64>(CurrentStance));

	// The two numbers the slide actually turns on, so it is never a guess which
	// of the angle or the speed is failing.
	const FVector Downhill = FVector::VectorPlaneProject(FVector::DownVector, MovementComponent->GetFloorNormal()).GetSafeNormal();
	const float DownhillSpeed = FVector::DotProduct(MovementComponent->Velocity, Downhill);

	// Skydive gating, split so it is clear which clause refuses.
	const URLocomotionConfig& DebugCfg = GetConfigRef();
	const UAbilitySystemComponent* DebugASC = GetOwnerASC();
	const bool bHasSkydiveTag = DebugCfg.SkydiveUnlockTag.IsValid()
		&& DebugASC && DebugASC->HasMatchingGameplayTag(DebugCfg.SkydiveUnlockTag);

	// Splits "the key is not reaching us" from "the sweep finds nothing", which
	// are the two ways a climb silently fails to start.
	FHitResult ClimbSurface;
	const bool bClimbSurfaceFound = MovementComponent->FindClimbableSurface(ClimbSurface);

	// The same sweep without the climbable filter. If this blocks while the
	// filtered version does not, the geometry is on the channel and something in
	// IsSurfaceClimbable is rejecting it, which is a different problem from the
	// trace missing entirely.
	const FVector RawStart = OwningCharacter->GetActorLocation();
	const FVector RawEnd = RawStart + OwningCharacter->GetActorForwardVector() * MovementComponent->ClimbDetectionDistance;
	FCollisionQueryParams RawParams(SCENE_QUERY_STAT(RClimbDebugTrace), false, OwningCharacter);
	FHitResult RawHit;
	const bool bRawBlocked = GetWorld()->SweepSingleByChannel(
		RawHit, RawStart, RawEnd, FQuat::Identity, MovementComponent->ClimbTraceChannel,
		FCollisionShape::MakeSphere(MovementComponent->ClimbTraceRadius), RawParams);

	// Draw the climb sweep itself, so it is visible whether it even reaches the
	// wall and what it is hitting, rather than inferring from a bool.
	const FVector ClimbStart = OwningCharacter->GetActorLocation();
	const FVector ClimbEnd = ClimbStart + OwningCharacter->GetActorForwardVector() * MovementComponent->ClimbDetectionDistance;
	const FColor SweepColour = bClimbSurfaceFound ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), ClimbStart, ClimbEnd, SweepColour, false, -1.f, 0, 1.f);
	DrawDebugSphere(GetWorld(), ClimbEnd, MovementComponent->ClimbTraceRadius, 12, SweepColour, false, -1.f, 0, 1.f);
	if (bClimbSurfaceFound)
	{
		DrawDebugDirectionalArrow(GetWorld(), ClimbSurface.ImpactPoint, ClimbSurface.ImpactPoint + ClimbSurface.ImpactNormal * 60.f, 12.f, FColor::Cyan, false, -1.f, 0, 1.f);
	}

	// Keyed on the component so each character overwrites its own line instead
	// of the list growing every frame.
	GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()), 0.f, FColor::Green,
		FString::Printf(
			TEXT("%s | %s | %s | Speed %.0f | Yaw %.0f | Lock %.2f | Falling %.2f | VertSpeed %.0f | Floor %.0f | Downhill %.0f | Strafe %d | Turn %d | ClimbHeld %d | ChannelHit %d | ClimbFound %d | ASC %d | SkyTagSet %d | HasSkyTag %d | CanSky %d"),
			*StateName, *GaitName, *StanceName,
			AnimData.GroundSpeed, AnimData.YawSpeed, StateLockRemaining,
			AnimData.TimeFalling, AnimData.VerticalSpeed, AnimData.FloorAngle, DownhillSpeed,
			AnimData.bIsStrafing ? 1 : 0, AnimData.bIsTurningInPlace ? 1 : 0,
			bClimbHeld ? 1 : 0, bRawBlocked ? 1 : 0, bClimbSurfaceFound ? 1 : 0,
			DebugASC ? 1 : 0, DebugCfg.SkydiveUnlockTag.IsValid() ? 1 : 0,
			bHasSkydiveTag ? 1 : 0, CanSkydive() ? 1 : 0));
}
#endif

void URLocomotionComponent::UpdateStance()
{
	// Crouch() and UnCrouch() only set an intent. The movement component decides,
	// and it refuses to stand up under a low ceiling. Read the result back rather
	// than assuming the request took, or the gait settings, the reported stance
	// and the actual capsule all disagree.
	CurrentStance = OwningCharacter->bIsCrouched ? ERStance::Crouching : ERStance::Standing;
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

	if (CurrentState == ERLocomotionState::Slide)
	{
		MovementComponent->ApplySlideSettings(Cfg.SlideMaxSpeed, Cfg.SlideGroundFriction, Cfg.SlideBrakingDeceleration);
		return;
	}

	// Targeting replaces the gait tier outright, so speed, acceleration and turn
	// rate while locked on all come from one place in the config.
	MovementComponent->ApplyGaitSettings(bIsStrafing
		? Cfg.TargetingSettings
		: Cfg.GetGaitSettings(CurrentGait, CurrentStance));
}

void URLocomotionComponent::UpdateSlide(float DeltaTime)
{
	if (CurrentState != ERLocomotionState::Slide)
	{
		return;
	}

	const FVector Normal = MovementComponent->GetFloorNormal();
	if (Normal.IsNearlyZero())
	{
		return;
	}

	// Gravity projected onto the slope. Walking mode keeps the capsule on walkable
	// floors and never pulls it down them, so the slide has to be driven here.
	// Steering is already dead: ApplySlideSettings zeroed MaxAcceleration.
	const FVector Downhill = FVector::VectorPlaneProject(FVector::DownVector, Normal).GetSafeNormal();
	const URLocomotionConfig& Cfg = GetConfigRef();
	MovementComponent->Velocity += Downhill * Cfg.SlideAcceleration * DeltaTime;

	// Turn to face the way gravity is taking us. Without this the character keeps
	// whatever facing it arrived with and slides down backwards.
	const FVector FacingTarget = FVector(Downhill.X, Downhill.Y, 0.f).GetSafeNormal();
	if (!FacingTarget.IsNearlyZero())
	{
		const FRotator Target(0.f, FacingTarget.Rotation().Yaw, 0.f);
		OwningCharacter->SetActorRotation(
			FMath::RInterpConstantTo(OwningCharacter->GetActorRotation(), Target, DeltaTime, Cfg.SlideRotationRate));
	}
}

void URLocomotionComponent::UpdateStrafe()
{
	const URLocomotionConfig& Cfg = GetConfigRef();

	// Strafing follows the same rule as targeting: ground locomotion only.
	const bool bModeAllowsStrafe = CanTarget();

	bIsStrafing = false;
	if (Cfg.bStrafeWhileTargeting && bModeAllowsStrafe)
	{
		if (const UAbilitySystemComponent* ASC = GetOwnerASC())
		{
			bIsStrafing = ASC->HasMatchingGameplayTag(FRGameplayTags::Get().status_targeting);
		}
	}

	if (!bModeAllowsStrafe)
	{
		return;
	}

	// Re-asserted every tick rather than only on change: leaving a climb resets
	// bOrientRotationToMovement in OnMovementModeChanged, which would otherwise
	// silently drop the character out of strafe.
	MovementComponent->bOrientRotationToMovement = !bIsStrafing;
	MovementComponent->bUseControllerDesiredRotation = bIsStrafing;
}

void URLocomotionComponent::UpdateAnimData(float DeltaTime)
{
	const URLocomotionConfig& Cfg = GetConfigRef();
	const FVector Velocity = MovementComponent->Velocity;

	AnimData.State = CurrentState;
	AnimData.Gait = CurrentGait;
	AnimData.Stance = CurrentStance;
	AnimData.GroundSpeed = Velocity.Size2D();
	AnimData.VerticalSpeed = Velocity.Z;
	AnimData.Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwningCharacter->GetActorRotation());

	// Turning on the spot has no velocity to read, so publish the capsule turn
	// rate instead. Orient to movement spins the character toward new input while
	// it is barely translating, and strafing turns it to face the target; both
	// show up here.
	const float CurrentYaw = OwningCharacter->GetActorRotation().Yaw;
	AnimData.YawSpeed = DeltaTime > KINDA_SMALL_NUMBER
		? FMath::FindDeltaAngleDegrees(PreviousYaw, CurrentYaw) / DeltaTime
		: 0.f;
	PreviousYaw = CurrentYaw;
	PreviousVelocity = Velocity;

	// Latching on last frame's value is the hysteresis: once turning, the yaw rate
	// has to drop well under the threshold before the turn is called finished.
	const float RequiredYawSpeed = AnimData.bIsTurningInPlace
		? FMath::Max(0.f, Cfg.TurnInPlaceYawSpeed - Cfg.TurnInPlaceYawSpeedHysteresis)
		: Cfg.TurnInPlaceYawSpeed;

	const bool bWasTurning = AnimData.bIsTurningInPlace;

	AnimData.bIsTurningInPlace = MovementComponent->IsMovingOnGround()
		&& AnimData.GroundSpeed <= Cfg.TurnInPlaceMaxGroundSpeed
		&& FMath::Abs(AnimData.YawSpeed) >= RequiredYawSpeed;

	// Captured once, on the frame the turn begins. Sampling the sign every frame
	// would flip the animation if the yaw rate crossed zero as the turn settled.
	if (AnimData.bIsTurningInPlace && !bWasTurning)
	{
		AnimData.bIsTurningRight = AnimData.YawSpeed > 0.f;
	}
	AnimData.bHasMovementInput = !MovementInput.IsNearlyZero();
	AnimData.bIsGrounded = MovementComponent->IsMovingOnGround();
	AnimData.bIsInWater = MovementComponent->IsSwimming();
	AnimData.ImmersionDepth = MovementComponent->GetImmersionFraction();
	AnimData.TimeInState += DeltaTime;
	AnimData.TimeFalling = TimeFalling;
	AnimData.bIsLongFall = TimeFalling >= Cfg.LongFallTime;
	AnimData.FloorAngle = MovementComponent->GetFloorAngle();
	AnimData.bIsStrafing = bIsStrafing;

	// Same predicate CanTarget uses, published so the anim graph does not have to
	// rebuild it out of four enum comparisons and drift from this one.
	AnimData.bIsGroundLocomotion = CanTarget();
	AnimData.TraversalAlpha = MovementComponent->GetTraversalAlpha();

	// Wall plane axes, only meaningful while climbing. The character faces the
	// wall, so its right vector lies along the wall and world Z is up the wall.
	if (MovementComponent->IsClimbing())
	{
		AnimData.ClimbRightSpeed = FVector::DotProduct(Velocity, OwningCharacter->GetActorRightVector());
		AnimData.ClimbUpSpeed = Velocity.Z;
	}
	else
	{
		AnimData.ClimbRightSpeed = 0.f;
		AnimData.ClimbUpSpeed = 0.f;
	}
}

// --- Tags -------------------------------------------------------------------

bool URLocomotionComponent::CanTarget() const
{
	return CurrentState == ERLocomotionState::Idle
		|| CurrentState == ERLocomotionState::Walk
		|| CurrentState == ERLocomotionState::Run
		|| CurrentState == ERLocomotionState::Sprint;
}

bool URLocomotionComponent::IsGroundState(ERLocomotionState State)
{
	return State == ERLocomotionState::Idle
		|| State == ERLocomotionState::Walk
		|| State == ERLocomotionState::Run
		|| State == ERLocomotionState::Sprint
		|| State == ERLocomotionState::Crouch
		|| State == ERLocomotionState::Land
		|| State == ERLocomotionState::Roll
		|| State == ERLocomotionState::Slide;
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
	case ERLocomotionState::Skydive:
		// Separate from the ability.skydive unlock, which is permanent once learned.
		// Fall damage keys off this one, so it is suppressed during a dive and not
		// for the rest of the game.
		Result.AddTag(Tags.state_locomotion_skydiving);
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

	// Captured before the reset, since a fall damage rule may care how long the
	// drop lasted rather than only how fast it ended.
	const float FallDuration = TimeFalling;
	TimeFalling = 0.f;

	// Fold whatever is out on touchdown, so the next fall does not auto deploy it.
	bGlideHeld = false;

	// Speed into the ground, not merely downward speed, so this is the same
	// measure an airborne collision with a wall uses and one curve can serve
	// both. On flat ground the two are identical; on a slope, arriving fast
	// along it counts for something, which it should.
	//
	// Velocity is already zeroed by the time this fires, hence the cached value
	// from the last tick while still airborne.
	const float ImpactSpeed = FMath::Max(0.f, FVector::DotProduct(PreviousVelocity, -Hit.ImpactNormal));
	AnimData.LastLandingImpactSpeed = ImpactSpeed;
	AnimData.bHeavyLanding = ImpactSpeed >= GetConfigRef().HeavyLandingSpeed;

	// Broadcast before the soft landing early out below, so every touchdown is
	// reported and not only the ones that enter a landing state.
	OnLandedImpact.Broadcast(ImpactSpeed, FallDuration);

	// Also sent as a gameplay event, so an ability can be triggered by it rather
	// than something having to bind a delegate at BeginPlay and unbind later.
	// Magnitude carries the impact speed; fall damage scales off it.
	FGameplayEventData LandedEvent;
	LandedEvent.EventTag = FRGameplayTags::Get().Event_Landed;
	LandedEvent.Instigator = OwningCharacter;
	LandedEvent.Target = OwningCharacter;
	LandedEvent.EventMagnitude = ImpactSpeed;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwningCharacter, LandedEvent.EventTag, LandedEvent);

	// A traversal's own landing is already playing. The capsule settling the last
	// couple of centimetres onto the ground after it must not replace it.
	if (StateLockRemaining > 0.f
		&& (CurrentState == ERLocomotionState::Land || CurrentState == ERLocomotionState::Roll))
	{
		return;
	}

	const URLocomotionConfig& Cfg = GetConfigRef();

	// Once the table has rows it decides every landing, including whether there
	// is one at all.
	if (Cfg.LandingActions.Num() > 0)
	{
		if (const FRLandingAction* Action = Cfg.FindLandingAction(CurrentState, ImpactSpeed, PreviousVelocity.Size2D()))
		{
			BeginLanding(Action->State, Action->Animation, Action->RecoveryTime);
		}
		return;
	}

	// A light landing needs no dedicated state; the ground states read fine.
	if (ImpactSpeed < Cfg.SoftLandingSpeed)
	{
		return;
	}

	const ERLocomotionState LandingState = ImpactSpeed >= Cfg.HardLandingSpeed
		? ERLocomotionState::Roll
		: ERLocomotionState::Land;

	BeginLanding(LandingState, nullptr, Cfg.LandRecoveryTime);
}

void URLocomotionComponent::BeginLanding(ERLocomotionState LandingState, UAnimSequenceBase* Animation, float RecoveryTime)
{
	// Published before EnterState, as the traversal clip is, so the graph never
	// enters the Land state holding last landing's clip.
	AnimData.LandingAnim = Animation;

	// Clear any residual lock so the landing itself is never swallowed.
	StateLockRemaining = 0.f;

	if (!EnterState(LandingState))
	{
		return;
	}

	float Hold = RecoveryTime;
	if (Hold <= 0.f)
	{
		Hold = Animation ? Animation->GetPlayLength() : GetConfigRef().LandRecoveryTime;
	}

	StateLockRemaining = FMath::Max(StateLockRemaining, Hold);
}

void URLocomotionComponent::HandleAirborneImpact(float SpeedIntoSurface, const FHitResult& Hit)
{
	// Sent as a gameplay event for the same reason landing is: an ability can be
	// triggered by it rather than something having to bind and unbind a delegate.
	// Separate from event.landed on purpose, since the skydive skill spares you a
	// fall but should not spare you flying into a cliff.
	FGameplayEventData ImpactEvent;
	ImpactEvent.EventTag = FRGameplayTags::Get().Event_Impact;
	ImpactEvent.Instigator = OwningCharacter;
	ImpactEvent.Target = OwningCharacter;
	ImpactEvent.EventMagnitude = SpeedIntoSurface;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwningCharacter, ImpactEvent.EventTag, ImpactEvent);
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
	}
	// Leaving water: restore gait handling and drop any dive input still held.
	else if (PreviousMode == MOVE_Swimming)
	{
		SwimVerticalInput = 0.f;
		MovementComponent->ApplyGaitSettings(Cfg.GetGaitSettings(CurrentGait, CurrentStance));
	}

	// A traversal finishing hands over to its landing clip, if its row has one.
	// Not into water, where there is nothing to land on.
	if (PreviousMode == MOVE_Custom && PreviousCustomMode == static_cast<uint8>(ERCustomMovementMode::Traversal))
	{
		if (PendingTraversalLanding && !MovementComponent->IsSwimming())
		{
			BeginLanding(ERLocomotionState::Land, PendingTraversalLanding, PendingTraversalLandingRecovery);
		}
		PendingTraversalLanding = nullptr;
	}

	// The mode has already changed by the time this fires, so reflect it now.
	EnterState(EvaluateDesiredState());
}
