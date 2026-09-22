// Fill out your copyright notice in the Description page of Project Settings.


#include "Locomotion/RCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
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

void URCharacterMovementComponent::ApplySlideSettings(float InMaxSpeed, float InFriction, float InBrakingDeceleration)
{
	MaxWalkSpeed = InMaxSpeed;
	GroundFriction = InFriction;
	BrakingDecelerationWalking = InBrakingDeceleration;
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

	if (IsTraversing())
	{
		Velocity = FVector::ZeroVector;
		TraversalElapsed = 0.f;
	}
}

void URCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	switch (GetCustomMovementModeEnum())
	{
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
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);

		// Touching walkable ground ends the glide.
		if (IsWalkable(Hit))
		{
			SetMovementMode(MOVE_Walking);
			return;
		}
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
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
	// Near vertical only; walkable slopes should be walked, not climbed.
	if (FMath::Abs(Hit.ImpactNormal.Z) > ClimbMaxSurfaceNormalZ)
	{
		return false;
	}

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
		Hit, Start, End, FQuat::Identity, TraceChannel,
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

bool URCharacterMovementComponent::FindTraversal(FVector& OutStart, FVector& OutMid, FVector& OutEnd, ERLocomotionState& OutType) const
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

	// 3. How deep is it? Trace down beyond the far edge to find the landing.
	const FVector FarProbeStart = TopHit.ImpactPoint + Forward * MaxVaultDepth + FVector::UpVector * TraversalClearance;
	const FVector FarProbeEnd = FarProbeStart - FVector::UpVector * (MaxMantleHeight + HalfHeight * 2.f);

	FHitResult FarHit;
	const bool bFarGround = GetWorld()->LineTraceSingleByChannel(FarHit, FarProbeStart, FarProbeEnd, TraceChannel, Params);

	// Thin and low enough to hop over, with ground waiting on the far side.
	const bool bThin = bFarGround && FarHit.ImpactPoint.Z < TopHit.ImpactPoint.Z - 10.f;
	const bool bVault = bThin && ObstacleHeight <= MaxVaultHeight;

	const FVector TopSurface = TopHit.ImpactPoint + FVector::UpVector * (HalfHeight + 2.f);

	FVector Landing;
	if (bVault)
	{
		Landing = FarHit.ImpactPoint + FVector::UpVector * (HalfHeight + 2.f);
		OutType = ERLocomotionState::Vault;
	}
	else
	{
		// Stand on top, a little past the edge so the capsule is fully supported.
		Landing = TopHit.ImpactPoint + Forward * Radius + FVector::UpVector * (HalfHeight + 2.f);
		OutType = ERLocomotionState::Mantle;
	}

	if (!HasRoomAt(Landing))
	{
		return false;
	}

	OutStart = Location;
	OutMid = TopSurface;
	OutEnd = Landing;
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

void URCharacterMovementComponent::BeginTraversal(const FVector& Start, const FVector& Mid, const FVector& End, float Duration)
{
	TraversalStart = Start;
	TraversalMid = Mid;
	TraversalEnd = End;
	TraversalDuration = FMath::Max(0.1f, Duration);
	TraversalElapsed = 0.f;

	SetCustomMovementMode(ERCustomMovementMode::Traversal);
}

void URCharacterMovementComponent::PhysTraversal(float DeltaTime, int32 Iterations)
{
	TraversalElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(TraversalElapsed / TraversalDuration, 0.f, 1.f);

	// Quadratic bezier through the obstacle top, which keeps the capsule clear of
	// the edge instead of clipping the corner.
	const float OneMinus = 1.f - Alpha;
	const FVector Target =
		OneMinus * OneMinus * TraversalStart +
		2.f * OneMinus * Alpha * TraversalMid +
		Alpha * Alpha * TraversalEnd;

	// Swept but non-blocking: the path was validated before it started, and a
	// blocking move here would stall the character halfway over the obstacle.
	FHitResult Hit(1.f);
	MoveUpdatedComponent(Target - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), false, &Hit);

	if (Alpha >= 1.f)
	{
		Velocity = FVector::ZeroVector;
		SetMovementMode(MOVE_Falling);
	}
}
