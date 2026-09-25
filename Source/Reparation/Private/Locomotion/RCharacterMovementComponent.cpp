// Fill out your copyright notice in the Description page of Project Settings.


#include "Locomotion/RCharacterMovementComponent.h"

#include "Animation/AnimSequenceBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

URCharacterMovementComponent::URCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanSwim = true;
	NavAgentProps.bCanFly = true;

	bOrientRotationToMovement = true;
	bUseControllerDesiredRotation = false;
	RotationRate = FRotator(0.f, 540.f, 0.f);
	DefaultGroundFriction = GroundFriction;

	// Deliberately permissive. This is not "how steep can you walk up" — that is
	// MaxAscendableSlopeAngle, which strips uphill input at 40. This is only the
	// angle past which the engine stops calling a surface ground at all, and
	// anything past it is MOVE_Falling, where the Slide state cannot run and the
	// engine scrapes you down with a falling animation instead.
	//
	// Set high so the slide band is 40 to 80 rather than 40 to 45. Steep faces are
	// still unwalkable in practice: uphill input is stripped above 40 and the
	// slide takes over.
	SetWalkableFloorAngle(80.f);

	// Water and flight. The locomotion component overwrites these from its config.
	MaxSwimSpeed = 300.f;
	BrakingDecelerationSwimming = 1024.f;
	Buoyancy = 1.f;
	MaxFlySpeed = 1200.f;
	BrakingDecelerationFlying = 1024.f;
}

float URCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Custom)
	{
		switch (GetCustomMovementModeEnum())
		{
		case ERCustomMovementMode::Skydive:
		case ERCustomMovementMode::Glide:		return GlideMaxSpeed;
		case ERCustomMovementMode::Climb:		return ClimbMaxSpeed;
		case ERCustomMovementMode::Traversal:	return 0.f;
		default:								break;
		}
	}

	return Super::GetMaxSpeed();
}

float URCharacterMovementComponent::GetMaxAcceleration() const
{
	if (MovementMode == MOVE_Custom)
	{
		switch (GetCustomMovementModeEnum())
		{
		case ERCustomMovementMode::Skydive:
		case ERCustomMovementMode::Glide:	return GlideAcceleration;
		case ERCustomMovementMode::Climb:	return ClimbAcceleration;
		default:							break;
		}
	}

	return Super::GetMaxAcceleration();
}

float URCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode == MOVE_Custom)
	{
		switch (GetCustomMovementModeEnum())
		{
		case ERCustomMovementMode::Skydive:
		case ERCustomMovementMode::Glide:	return GlideBrakingDeceleration;
		case ERCustomMovementMode::Climb:	return ClimbBrakingDeceleration;
		default:							break;
		}
	}

	return Super::GetMaxBrakingDeceleration();
}

bool URCharacterMovementComponent::CanAttemptJump() const
{
	// Jumping off a wall is how a climb is abandoned upward, so allow it there.
	return Super::CanAttemptJump() || IsClimbing();
}

void URCharacterMovementComponent::ApplyGaitSettings(const FRGaitSettings& Settings)
{
	MaxWalkSpeed = Settings.MaxSpeed;
	MaxWalkSpeedCrouched = Settings.MaxSpeed;
	MaxAcceleration = Settings.MaxAcceleration;
	BrakingDecelerationWalking = Settings.BrakingDeceleration;
	RotationRate = FRotator(0.f, Settings.RotationRate, 0.f);

	// Restored here because a slide lowers it and nothing else would put it back.
	GroundFriction = DefaultGroundFriction;
}

FVector URCharacterMovementComponent::ScaleInputAcceleration(const FVector& InputAcceleration) const
{
	const FVector Scaled = Super::ScaleInputAcceleration(InputAcceleration);

	if (!IsMovingOnGround() || GetFloorAngle() < MaxAscendableSlopeAngle)
	{
		return Scaled;
	}

	// Too steep for feet, so no foot movement at all: not across it, not up it.
	// Gravity is the only thing that moves the character on a face this steep,
	// which is what makes the slide read as losing your footing rather than as
	// sidestepping down a hill. Getting up a face like this is climbing, and that
	// needs geometry blocking the Climbable channel.
	return FVector::ZeroVector;
}

float URCharacterMovementComponent::GetFloorAngle() const
{
	if (!IsMovingOnGround())
	{
		return 0.f;
	}

	const FVector Normal = CurrentFloor.HitResult.ImpactNormal;
	if (Normal.IsNearlyZero())
	{
		return 0.f;
	}

	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Normal.Z, -1.f, 1.f)));
}

FVector URCharacterMovementComponent::GetFloorNormal() const
{
	return IsMovingOnGround() ? CurrentFloor.HitResult.ImpactNormal : FVector::ZeroVector;
}

void URCharacterMovementComponent::ApplySlideSettings(float InMaxSpeed, float InFriction, float InBrakingDeceleration)
{
	MaxWalkSpeed = InMaxSpeed;

	// A slide is not steerable. Zeroing acceleration makes AddMovementInput inert
	// without intercepting input anywhere, and ApplyGaitSettings restores it.
	MaxAcceleration = 0.f;
	GroundFriction = InFriction;
	BrakingDecelerationWalking = InBrakingDeceleration;
}

void URCharacterMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);

	// Only airborne collisions. On the ground this fires constantly against every
	// kerb and doorframe.
	if (!IsFalling() && !IsGliding() && !IsSkydiving())
	{
		return;
	}

	// A walkable surface is a landing, which HandleLanded already reports. This is
	// for the faces you cannot land on: cliffs, walls, the side of a mountain.
	if (IsWalkable(Hit))
	{
		return;
	}

	// Speed into the surface, not raw speed. Skimming a wall at a glancing angle
	// should not read the same as flying straight at it.
	const float SpeedIntoSurface = FVector::DotProduct(Velocity, -Hit.ImpactNormal);
	if (SpeedIntoSurface >= MinAirborneImpactSpeed)
	{
		OnAirborneImpact.Broadcast(SpeedIntoSurface, Hit);
	}
}

void URCharacterMovementComponent::ApplyGlideSettings(const FRGlideSettings& Settings)
{
	GlideMaxSpeed = Settings.MaxSpeed;
	GlideDescentRate = Settings.DescentRate;
	GlideDescentInterpSpeed = Settings.DescentInterpSpeed;
	GlideAcceleration = Settings.Acceleration;
	GlideFriction = Settings.Friction;
	GlideBrakingDeceleration = Settings.BrakingDeceleration;
	GlideRotationRate = Settings.RotationRate;
	GlideDiveSpeedBonus = Settings.DiveSpeedBonus;

	// Applied live if already airborne, so swapping wings mid flight takes hold.
	if (IsGliding())
	{
		RotationRate = FRotator(0.f, GlideRotationRate, 0.f);
	}
}

void URCharacterMovementComponent::ApplySwimSettings(float InMaxSpeed, float InAcceleration)
{
	MaxSwimSpeed = InMaxSpeed;

	if (IsSwimming())
	{
		MaxAcceleration = InAcceleration;
	}
}

void URCharacterMovementComponent::ApplyFlySettings(float InMaxSpeed, float InAcceleration, float InBrakingDeceleration)
{
	MaxFlySpeed = InMaxSpeed;
	BrakingDecelerationFlying = InBrakingDeceleration;

	if (MovementMode == MOVE_Flying)
	{
		MaxAcceleration = InAcceleration;
	}
}

void URCharacterMovementComponent::SetCustomMovementMode(ERCustomMovementMode Mode)
{
	if (Mode == ERCustomMovementMode::None)
	{
		SetMovementMode(MOVE_Falling);
		return;
	}

	SetMovementMode(MOVE_Custom, static_cast<uint8>(Mode));
}

ERCustomMovementMode URCharacterMovementComponent::GetCustomMovementModeEnum() const
{
	return static_cast<ERCustomMovementMode>(CustomMovementMode);
}

bool URCharacterMovementComponent::IsCustomMode(ERCustomMovementMode Mode) const
{
	return MovementMode == MOVE_Custom && GetCustomMovementModeEnum() == Mode;
}

float URCharacterMovementComponent::GetImmersionFraction() const
{
	if (!CharacterOwner)
	{
		return 0.f;
	}

	return IsInWater() ? ImmersionDepth() : 0.f;
}

void URCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	const bool bWasClimbing = PreviousMovementMode == MOVE_Custom
		&& static_cast<ERCustomMovementMode>(PreviousCustomMode) == ERCustomMovementMode::Climb;

	if (bWasClimbing && !IsClimbing())
	{
		ClimbSurfaceNormal = FVector::ZeroVector;
		bAtClimbLedge = false;

		// Climbing suppresses rotation to face the wall; restore normal handling.
		bOrientRotationToMovement = true;
	}

	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		Velocity = FVector::ZeroVector;
	}

	if (IsGliding() || IsSkydiving())
	{
		// Set explicitly rather than inherited. Coming off a targeted approach
		// leaves bUseControllerDesiredRotation set, and the glider then refuses
		// to turn toward where the player is steering.
		bOrientRotationToMovement = true;
		bUseControllerDesiredRotation = false;
		RotationRate = FRotator(0.f, GlideRotationRate, 0.f);
	}

	if (IsTraversing())
	{
		Velocity = FVector::ZeroVector;
		TraversalElapsed = 0.f;
	}
}

void URCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	// Input still produces acceleration mid move, and orient to movement would
	// turn the capsule toward it, undoing a clip that turns the character round.
	if (IsTraversing())
	{
		return;
	}

	Super::PhysicsRotation(DeltaTime);
}

void URCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	switch (GetCustomMovementModeEnum())
	{
	// Skydiving is a glide with a far steeper sink, so it shares the physics and
	// differs only in the numbers applied when the mode is entered.
	case ERCustomMovementMode::Skydive:
	case ERCustomMovementMode::Glide:
		PhysGlide(DeltaTime, Iterations);
		break;
	case ERCustomMovementMode::Climb:
		PhysClimb(DeltaTime, Iterations);
		break;
	case ERCustomMovementMode::Traversal:
		PhysTraversal(DeltaTime, Iterations);
		break;
	default:
		SetMovementMode(MOVE_Falling);
		break;
	}
}

// --- Glide ------------------------------------------------------------------

void URCharacterMovementComponent::PhysGlide(float DeltaTime, int32 Iterations)
{
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		// Horizontal handling behaves like flight; vertical is forced toward a
		// steady sink rate so the glider loses height predictably.
		CalcVelocity(DeltaTime, GlideFriction, true, GetMaxBrakingDeceleration());

		const float TargetDescent = -GlideDescentRate;
		Velocity.Z = FMath::FInterpTo(Velocity.Z, TargetDescent, DeltaTime, GlideDescentInterpSpeed);
	}

	ApplyRootMotionToVelocity(DeltaTime);

	Iterations++;
	bJustTeleported = false;

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * DeltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		// Captured before sliding. SlideAlongSurface takes Hit by reference and
		// overwrites it, so testing afterwards asks about whatever the slide ran
		// into rather than the ground just touched, and the glide never ended.
		const bool bTouchedGround = IsWalkable(Hit);

		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);

		if (bTouchedGround)
		{
			SetMovementMode(MOVE_Walking);

			// Landing out of a glide or a skydive never passes through PhysFalling,
			// so ProcessLanded does not run and the character's landing path is
			// never entered: no LandedDelegate, no landed event, no roll, and the
			// wing stays flagged as held into the next fall. Report it by hand.
			// Done after the mode change, so anything reading movement state during
			// the notification sees the character on the ground.
			if (CharacterOwner && CharacterOwner->ShouldNotifyLanded(Hit))
			{
				CharacterOwner->Landed(Hit);
			}

			return;
		}
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		// Horizontal is taken from the move so collisions are honoured, but the
		// vertical stays as authored. Sliding along rising ground produces upward
		// displacement, and folding that back in turns terrain contact into lift:
		// the glider would climb a ramp and cross a landscape indefinitely.
		const FVector MovedVelocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
		Velocity.X = MovedVelocity.X;
		Velocity.Y = MovedVelocity.Y;
	}

	// Entering water cancels the glide so swimming can take over.
	if (IsInWater())
	{
		SetMovementMode(MOVE_Swimming);
	}
}

// --- Climb ------------------------------------------------------------------

bool URCharacterMovementComponent::IsSurfaceClimbable(const FHitResult& Hit) const
{
	// The trace channel is the authority here. Only geometry set to block
	// Climbable is hit at all, so there is no angle test to fight: a designer can
	// mark a shallow face or an overhang climbable and it simply works.
	if (!bRequireClimbableTag)
	{
		return true;
	}

	if (const AActor* HitActor = Hit.GetActor())
	{
		if (HitActor->ActorHasTag(ClimbableTag))
		{
			return true;
		}
	}

	if (const UPrimitiveComponent* HitComponent = Hit.GetComponent())
	{
		if (HitComponent->ComponentHasTag(ClimbableTag))
		{
			return true;
		}
	}

	return false;
}

bool URCharacterMovementComponent::FindClimbableSurface(FHitResult& OutHit) const
{
	if (!CharacterOwner || !UpdatedComponent)
	{
		return false;
	}

	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FVector End = Start + UpdatedComponent->GetForwardVector() * ClimbDetectionDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RClimbTrace), false, CharacterOwner);

	FHitResult Hit;
	const bool bBlocked = GetWorld()->SweepSingleByChannel(
		Hit, Start, End, FQuat::Identity, ClimbTraceChannel,
		FCollisionShape::MakeSphere(ClimbTraceRadius), Params);

	if (!bBlocked || !IsSurfaceClimbable(Hit))
	{
		return false;
	}

	OutHit = Hit;
	return true;
}

bool URCharacterMovementComponent::UpdateClimbSurface()
{
	FHitResult Hit;
	if (!FindClimbableSurface(Hit))
	{
		return false;
	}

	ClimbSurfaceNormal = Hit.ImpactNormal;
	bAtClimbLedge = CheckForClimbLedge();
	return true;
}

bool URCharacterMovementComponent::CheckForClimbLedge() const
{
	if (!CharacterOwner || !UpdatedComponent)
	{
		return false;
	}

	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	if (!Capsule)
	{
		return false;
	}

	// Probe forward from just above the head. Finding nothing means the wall has
	// ended and the character is at a ledge.
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FVector EyeLevel = UpdatedComponent->GetComponentLocation() + FVector::UpVector * (HalfHeight + ClimbTraceRadius);
	const FVector Ahead = EyeLevel + UpdatedComponent->GetForwardVector() * ClimbDetectionDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RClimbLedgeTrace), false, CharacterOwner);

	FHitResult Hit;
	const bool bBlocked = GetWorld()->SweepSingleByChannel(
		Hit, EyeLevel, Ahead, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(ClimbTraceRadius * 0.5f), Params);

	return !bBlocked;
}

void URCharacterMovementComponent::RotateTowardClimbSurface(float DeltaTime)
{
	if (ClimbSurfaceNormal.IsNearlyZero() || !UpdatedComponent)
	{
		return;
	}

	// Face into the wall: forward is the inverse of its outward normal.
	const FRotator Target = (-ClimbSurfaceNormal).GetSafeNormal2D().Rotation();
	const FRotator Current = UpdatedComponent->GetComponentRotation();
	const FRotator Next = FMath::RInterpConstantTo(Current, FRotator(0.f, Target.Yaw, 0.f), DeltaTime, ClimbRotationRate);

	MoveUpdatedComponent(FVector::ZeroVector, Next, false);
}

void URCharacterMovementComponent::PhysClimb(float DeltaTime, int32 Iterations)
{
	if (!UpdateClimbSurface())
	{
		SetMovementMode(MOVE_Falling);
		return;
	}

	// Climbing down the bottom of a wall leaves the surface trace succeeding, so
	// nothing else here ends the climb: the character hangs at the base with
	// their feet in the floor until the key is released. Descending onto
	// walkable ground is a dismount.
	if (Velocity.Z <= -ClimbDismountDescentSpeed)
	{
		FFindFloorResult Floor;
		FindFloor(UpdatedComponent->GetComponentLocation(), Floor, false);

		if (Floor.IsWalkableFloor())
		{
			SetMovementMode(MOVE_Walking);
			return;
		}
	}

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(DeltaTime, ClimbFriction, false, GetMaxBrakingDeceleration());

		// Movement is confined to the plane of the wall.
		Velocity = FVector::VectorPlaneProject(Velocity, ClimbSurfaceNormal);

		// Gentle pull inward keeps the capsule attached over bumps and corners.
		Velocity -= ClimbSurfaceNormal * ClimbSnapSpeed;
	}

	ApplyRootMotionToVelocity(DeltaTime);

	Iterations++;
	bJustTeleported = false;

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * DeltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;

		// The snap component is a positional correction, not real motion; leaving
		// it in the velocity would feed back into the next frame's acceleration.
		Velocity = FVector::VectorPlaneProject(Velocity, ClimbSurfaceNormal);
	}

	RotateTowardClimbSurface(DeltaTime);
}

// --- Traversal (vault and mantle) -------------------------------------------

bool URCharacterMovementComponent::HasRoomAt(const FVector& Location) const
{
	const UCapsuleComponent* Capsule = CharacterOwner ? CharacterOwner->GetCapsuleComponent() : nullptr;
	if (!Capsule)
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RTraversalRoom), false, CharacterOwner);

	const FCollisionShape Shape = FCollisionShape::MakeCapsule(
		Capsule->GetScaledCapsuleRadius(),
		Capsule->GetScaledCapsuleHalfHeight());

	return !GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity, TraceChannel, Shape, Params);
}

bool URCharacterMovementComponent::FindTraversal(ERTraversalEntry Entry, FRTraversalQuery& OutQuery) const
{
	const UCapsuleComponent* Capsule = CharacterOwner ? CharacterOwner->GetCapsuleComponent() : nullptr;
	if (!Capsule || !UpdatedComponent)
	{
		return false;
	}

	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float Radius = Capsule->GetScaledCapsuleRadius();
	const FVector Location = UpdatedComponent->GetComponentLocation();
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector FootLevel = Location - FVector::UpVector * HalfHeight;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RTraversalTrace), false, CharacterOwner);

	// 1. Is there an obstacle directly ahead, around knee to waist height?
	const FVector ProbeStart = FootLevel + FVector::UpVector * (HalfHeight * 0.5f);
	const FVector ProbeEnd = ProbeStart + Forward * TraversalDetectionDistance;

	FHitResult FrontHit;
	if (!GetWorld()->SweepSingleByChannel(FrontHit, ProbeStart, ProbeEnd, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(Radius * 0.5f), Params))
	{
		return false;
	}

	// Sloped faces should be walked up, not vaulted.
	if (FMath::Abs(FrontHit.ImpactNormal.Z) > 0.4f)
	{
		return false;
	}

	// 2. Find the top of it by tracing down from above the contact point.
	const FVector TopProbeStart = FrontHit.ImpactPoint + Forward * 5.f + FVector::UpVector * (MaxMantleHeight + TraversalClearance);
	const FVector TopProbeEnd = FrontHit.ImpactPoint + Forward * 5.f;

	FHitResult TopHit;
	if (!GetWorld()->LineTraceSingleByChannel(TopHit, TopProbeStart, TopProbeEnd, TraceChannel, Params))
	{
		// Too tall to get over; this is a climb, not a traversal.
		return false;
	}

	const float ObstacleHeight = TopHit.ImpactPoint.Z - FootLevel.Z;
	if (ObstacleHeight <= 0.f || ObstacleHeight > MaxMantleHeight)
	{
		return false;
	}

	// 3. How deep is it? Walk forward along the top surface until it runs out.
	// Sampled rather than traced back from beyond the obstacle, because a far
	// face that is not vertical defeats that, and so does anything deeper than
	// the probe. Never finding an edge is the honest answer for a wall.
	float ObstacleDepth = MaxVaultDepth;
	const int32 DepthSamples = 8;

	for (int32 Sample = 1; Sample <= DepthSamples; ++Sample)
	{
		const float Offset = (MaxVaultDepth * Sample) / DepthSamples;
		const FVector DepthStart = TopHit.ImpactPoint + Forward * Offset + FVector::UpVector * TraversalClearance;
		const FVector DepthEnd = DepthStart - FVector::UpVector * (TraversalClearance * 2.f);

		FHitResult DepthHit;
		if (!GetWorld()->LineTraceSingleByChannel(DepthHit, DepthStart, DepthEnd, TraceChannel, Params))
		{
			ObstacleDepth = Offset;
			break;
		}
	}

	// 4. Trace down just past the far edge to find what there is to land on.
	const FVector FarProbeStart = TopHit.ImpactPoint + Forward * (ObstacleDepth + Radius) + FVector::UpVector * TraversalClearance;
	const FVector FarProbeEnd = FarProbeStart - FVector::UpVector * (MaxMantleHeight + HalfHeight * 2.f);

	FHitResult FarHit;
	const bool bFarGround = GetWorld()->LineTraceSingleByChannel(FarHit, FarProbeStart, FarProbeEnd, TraceChannel, Params);

	const bool bDropsAway = bFarGround && FarHit.ImpactPoint.Z < TopHit.ImpactPoint.Z - 10.f;

	const FVector TopSurface = TopHit.ImpactPoint + FVector::UpVector * (HalfHeight + 2.f);

	// Both finishes are measured, because which one happens is the clip's choice.
	// On top stands a little past the edge so the capsule is fully supported.
	const FVector OnTopEnd = TopHit.ImpactPoint + Forward * Radius + FVector::UpVector * (HalfHeight + 2.f);
	const FVector FarSideEnd = FarHit.ImpactPoint + FVector::UpVector * (HalfHeight + 2.f);

	const bool bCanFinishOnTop = HasRoomAt(OnTopEnd);
	const bool bCanFinishFarSide = bDropsAway && HasRoomAt(FarSideEnd);

	if (!bCanFinishOnTop && !bCanFinishFarSide)
	{
		return false;
	}

	// The limits' own verdict, for Auto rows and an empty table: thin and low
	// enough to hop over goes over, anything else stands on top. When the
	// preferred finish has no room, the other one is all there is.
	const bool bFitsVault = ObstacleHeight <= MaxVaultHeight && ObstacleDepth < MaxVaultDepth;
	const bool bVault = bCanFinishFarSide && (bFitsVault || !bCanFinishOnTop);

	OutQuery.Start = Location;
	OutQuery.Mid = TopSurface;
	OutQuery.End = bVault ? FarSideEnd : OnTopEnd;
	OutQuery.OnTopEnd = OnTopEnd;
	OutQuery.FarSideEnd = FarSideEnd;
	OutQuery.bCanFinishOnTop = bCanFinishOnTop;
	OutQuery.bCanFinishFarSide = bCanFinishFarSide;
	OutQuery.ObstacleHeight = ObstacleHeight;
	OutQuery.ObstacleDepth = ObstacleDepth;
	OutQuery.FarSideDrop = bFarGround ? TopHit.ImpactPoint.Z - FarHit.ImpactPoint.Z : 0.f;
	OutQuery.bHasFarSideGround = bFarGround;
	OutQuery.ApproachSpeed = Velocity.Size2D();
	OutQuery.Entry = Entry;
	OutQuery.FallbackState = bVault ? ERLocomotionState::Vault : ERLocomotionState::Mantle;
	return true;
}

float URCharacterMovementComponent::GetTraversalAlpha() const
{
	if (!IsTraversing() || TraversalDuration <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	return FMath::Clamp(TraversalElapsed / TraversalDuration, 0.f, 1.f);
}

void URCharacterMovementComponent::BeginTraversal(const FVector& Start, const FVector& Mid, const FVector& End, float Duration,
	bool bTurnAround, const UAnimSequenceBase* Clip, float ApexTime)
{
	TraversalStart = Start;
	TraversalMid = Mid;
	TraversalEnd = End;
	TraversalDuration = FMath::Max(0.1f, Duration);
	TraversalElapsed = 0.f;
	TraversalStartYaw = UpdatedComponent ? UpdatedComponent->GetComponentRotation().Yaw : 0.f;

	// A clip that turns round is followed through the turn, so the flag is only
	// needed by clips with no root motion to follow.
	bTraversalTurnAround = !BuildTraversalPath(Clip, ApexTime) && bTurnAround;

	SetCustomMovementMode(ERCustomMovementMode::Traversal);
}

bool URCharacterMovementComponent::BuildTraversalPath(const UAnimSequenceBase* Clip, float ApexTime)
{
	TraversalPath.Reset();
	TraversalPathYaw.Reset();

	const USkeletalMeshComponent* Mesh = CharacterOwner ? CharacterOwner->GetMesh() : nullptr;
	if (!Clip || !Mesh || !UpdatedComponent)
	{
		return false;
	}

	const float Length = Clip->GetPlayLength();
	if (Length <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// Thirty a second is finer than the capsule moves in a frame at any sane
	// frame rate, and the whole path is only built once per move.
	const int32 Segments = FMath::Clamp(FMath::CeilToInt(Length * 30.f), 8, 300);

	// The root track is in mesh space, and the mesh is usually turned to face
	// the capsule's forward. Its relative transform takes the root's movement
	// into the capsule's frame.
	const FTransform MeshRelative = Mesh->GetRelativeTransform();
	const FTransform Root0 = Clip->ExtractRootTrackTransform(FAnimExtractContext(0.0), nullptr);

	TArray<FVector> Offsets;
	Offsets.Reserve(Segments + 1);
	TraversalPathYaw.Reserve(Segments + 1);

	float Yaw = 0.f;
	float PreviousRawYaw = 0.f;
	float Reach = 0.f;

	for (int32 Index = 0; Index <= Segments; ++Index)
	{
		const double Time = Length * Index / Segments;
		const FTransform Root = Clip->ExtractRootTrackTransform(FAnimExtractContext(Time), nullptr);

		const FVector Offset = MeshRelative.TransformVector(Root.GetLocation() - Root0.GetLocation());
		Offsets.Add(Offset);
		Reach = FMath::Max(Reach, Offset.Size());

		// Accumulated step by step so a half turn does not wrap from +180 to -180
		// and spin the capsule the long way round.
		const float RawYaw = (Root.GetRotation() * Root0.GetRotation().Inverse()).Rotator().Yaw;
		if (Index > 0)
		{
			Yaw += FMath::FindDeltaAngleDegrees(PreviousRawYaw, RawYaw);
		}
		PreviousRawYaw = RawYaw;
		TraversalPathYaw.Add(Yaw);
	}

	// An in place clip has nothing to follow; the arc will do.
	if (Reach < 10.f)
	{
		TraversalPathYaw.Reset();
		return false;
	}

	// The sample that should pass over the obstacle edge. For a vault that is
	// the top of the jump; for a mantle, the moment the body is nearly up.
	int32 Apex = Segments / 2;
	if (ApexTime > 0.f)
	{
		Apex = FMath::RoundToInt(ApexTime * Segments);
	}
	else
	{
		float MaxRise = 0.f;
		for (const FVector& Offset : Offsets)
		{
			MaxRise = FMath::Max(MaxRise, Offset.Z);
		}

		if (MaxRise > 10.f)
		{
			for (int32 Index = 0; Index <= Segments; ++Index)
			{
				if (Offsets[Index].Z >= MaxRise * 0.9f)
				{
					Apex = Index;
					break;
				}
			}
		}
	}
	Apex = FMath::Clamp(Apex, 1, Segments - 1);

	// Targets in the capsule's starting frame: over the edge, then the landing.
	const FRotator Frame(0.f, TraversalStartYaw, 0.f);
	const FVector ToMid = Frame.UnrotateVector(TraversalMid - TraversalStart);
	const FVector ToEnd = Frame.UnrotateVector(TraversalEnd - TraversalStart);

	// Stretches one axis of one half of the clip so it starts and ends where the
	// geometry says. Where the clip barely moves on that axis, scaling would
	// blow its jitter up into lurches, so the move is spread evenly over the
	// half instead and the clip's own small motion kept on top.
	auto WarpAxis = [](float Value, float From0, float From1, float To0, float To1, float TimeFraction)
	{
		const float Span = From1 - From0;
		if (FMath::Abs(Span) < 10.f)
		{
			return To0 + (To1 - To0) * TimeFraction + (Value - From0);
		}
		return To0 + (Value - From0) * (To1 - To0) / Span;
	};

	TraversalPath.Reserve(Segments + 1);
	for (int32 Index = 0; Index <= Segments; ++Index)
	{
		const bool bFirstHalf = Index <= Apex;
		const int32 From = bFirstHalf ? 0 : Apex;
		const int32 To = bFirstHalf ? Apex : Segments;
		const FVector& A = Offsets[From];
		const FVector& B = Offsets[To];
		const FVector TargetA = bFirstHalf ? FVector::ZeroVector : ToMid;
		const FVector TargetB = bFirstHalf ? ToMid : ToEnd;
		const float TimeFraction = static_cast<float>(Index - From) / (To - From);

		const FVector& Offset = Offsets[Index];
		const FVector Warped(
			WarpAxis(Offset.X, A.X, B.X, TargetA.X, TargetB.X, TimeFraction),
			WarpAxis(Offset.Y, A.Y, B.Y, TargetA.Y, TargetB.Y, TimeFraction),
			WarpAxis(Offset.Z, A.Z, B.Z, TargetA.Z, TargetB.Z, TimeFraction));

		TraversalPath.Add(TraversalStart + Frame.RotateVector(Warped));
	}

	return true;
}

void URCharacterMovementComponent::PhysTraversal(float DeltaTime, int32 Iterations)
{
	TraversalElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(TraversalElapsed / TraversalDuration, 0.f, 1.f);

	FVector Target;
	FQuat Facing = UpdatedComponent->GetComponentQuat();

	if (TraversalPath.Num() >= 2)
	{
		// Wherever the clip's root is at this point in the clip, so the feet stay
		// where the animation put them.
		const float Position = Alpha * (TraversalPath.Num() - 1);
		const int32 Index = FMath::Min(FMath::FloorToInt(Position), TraversalPath.Num() - 2);
		const float Blend = Position - Index;

		Target = FMath::Lerp(TraversalPath[Index], TraversalPath[Index + 1], Blend);
		const float Yaw = TraversalStartYaw + FMath::Lerp(TraversalPathYaw[Index], TraversalPathYaw[Index + 1], Blend);
		Facing = FRotator(0.f, Yaw, 0.f).Quaternion();
	}
	else
	{
		// Quadratic bezier through the obstacle top, which keeps the capsule clear
		// of the edge instead of clipping the corner.
		const float OneMinus = 1.f - Alpha;
		Target =
			OneMinus * OneMinus * TraversalStart +
			2.f * OneMinus * Alpha * TraversalMid +
			Alpha * Alpha * TraversalEnd;
	}

	// Swept but non-blocking: the path was validated before it started, and a
	// blocking move here would stall the character halfway over the obstacle.
	FHitResult Hit(1.f);
	MoveUpdatedComponent(Target - UpdatedComponent->GetComponentLocation(), Facing, false, &Hit);

	if (Alpha >= 1.f)
	{
		TraversalPath.Reset();
		TraversalPathYaw.Reset();

		// The clip turned the character round inside itself and the capsule never
		// followed. Turning it now, on the clip's last frame, lets the next state
		// start facing where the clip left off instead of snapping back.
		if (bTraversalTurnAround)
		{
			const FRotator TurnedAround = UpdatedComponent->GetComponentRotation() + FRotator(0.f, 180.f, 0.f);
			MoveUpdatedComponent(FVector::ZeroVector, TurnedAround.Quaternion(), false);
			bTraversalTurnAround = false;
		}

		Velocity = FVector::ZeroVector;
		SetMovementMode(MOVE_Falling);
	}
}
