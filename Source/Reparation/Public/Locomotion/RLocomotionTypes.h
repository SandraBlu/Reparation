// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RLocomotionTypes.generated.h"

class UAnimMontage;

/**
 * High level locomotion state. This is what drives animation and what abilities
 * gate against; it is deliberately coarser than the movement component's mode.
 */
UENUM(BlueprintType)
enum class ERLocomotionState : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	Walk			UMETA(DisplayName = "Walk"),
	Run				UMETA(DisplayName = "Run"),
	Sprint			UMETA(DisplayName = "Sprint"),
	Crouch			UMETA(DisplayName = "Crouch"),
	Jump			UMETA(DisplayName = "Jump"),
	Fall			UMETA(DisplayName = "Fall"),
	Land			UMETA(DisplayName = "Land"),
	Roll			UMETA(DisplayName = "Roll"),
	SwimSurface		UMETA(DisplayName = "Swim Surface"),
	SwimUnderwater	UMETA(DisplayName = "Swim Underwater"),

	Fly				UMETA(DisplayName = "Fly"),
	Glide			UMETA(DisplayName = "Glide"),
	Climb			UMETA(DisplayName = "Climb"),
	Mantle			UMETA(DisplayName = "Mantle"),
	Vault			UMETA(DisplayName = "Vault"),

	// Appended rather than slotted next to the ground states so the saved values
	// in DA_LocomotionConfig keep their meaning.
	Slide			UMETA(DisplayName = "Slide")
};

/** Ground speed tier. Independent of state so Crouch/Walk/Run share one axis. */
UENUM(BlueprintType)
enum class ERGait : uint8
{
	Walk	UMETA(DisplayName = "Walk"),
	Run		UMETA(DisplayName = "Run"),
	Sprint	UMETA(DisplayName = "Sprint")
};

UENUM(BlueprintType)
enum class ERStance : uint8
{
	Standing	UMETA(DisplayName = "Standing"),
	Crouching	UMETA(DisplayName = "Crouching")
};

/**
 * Values passed to UCharacterMovementComponent::SetMovementMode as the custom
 * mode byte. Only used when MovementMode == MOVE_Custom.
 */
UENUM(BlueprintType)
enum class ERCustomMovementMode : uint8
{
	None	UMETA(DisplayName = "None"),
	Glide	UMETA(DisplayName = "Glide"),
	Climb	UMETA(DisplayName = "Climb"),
	Traversal	UMETA(DisplayName = "Traversal")
};

/** Speed and handling for one gait tier. */
USTRUCT(BlueprintType)
struct FRGaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	float MaxSpeed = 375.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	float MaxAcceleration = 2048.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	float BrakingDeceleration = 1024.f;

	/** Degrees per second the mesh turns toward the movement direction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	float RotationRate = 540.f;
};

/**
 * One legal edge in the state machine.
 *
 * Conditions live in C++ (URLocomotionComponent::EvaluateDesiredState); this
 * table governs whether an edge is permitted at all and what it costs
 * cosmetically. That split keeps designers in control of feel without pushing
 * physics decisions into data.
 */
USTRUCT(BlueprintType)
struct FRLocomotionTransition
{
	GENERATED_BODY()

	/** States this edge may be taken from. Empty means any state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	TArray<ERLocomotionState> FromStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	ERLocomotionState ToState = ERLocomotionState::Idle;

	/** Set to forbid this edge outright, overriding the permissive default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	bool bBlocked = false;

	/** Optional montage played on entry. Does not block the state change. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	TObjectPtr<UAnimMontage> TransitionMontage = nullptr;

	/** Seconds the new state is held before another change is considered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition", meta = (ClampMin = "0.0"))
	float LockDuration = 0.f;

	/** Checked highest first, so specific edges can shadow general ones. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	int32 Priority = 0;
};

/**
 * Snapshot the anim instance reads. Written on the game thread by
 * URLocomotionComponent, read from NativeThreadSafeUpdateAnimation, so it holds
 * only plain values and no pointers to be dereferenced off-thread.
 */
USTRUCT(BlueprintType)
struct FRLocomotionAnimData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	ERLocomotionState State = ERLocomotionState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	ERGait Gait = ERGait::Run;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	ERStance Stance = ERStance::Standing;

	/** Horizontal speed, cm/s. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed = 0.f;

	/** Signed vertical velocity, cm/s. Negative while falling. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float VerticalSpeed = 0.f;

	/** -180..180, movement direction relative to actor rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsGrounded = true;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsInWater = false;

	/** 0 at the surface, 1 when the capsule is fully submerged. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float ImmersionDepth = 0.f;

	/** Seconds spent in the current state. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float TimeInState = 0.f;

	/**
	 * 0 to 1 progress through the current vault or mantle, 0 in every other
	 * state. The traversal clips hold no root motion, so drive a sequence
	 * evaluator explicit time from this and the feet stay locked to the capsule
	 * whatever the clip length is.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float TraversalAlpha = 0.f;

	/**
	 * Velocity in the plane of the climbed wall, cm/s. Right is positive toward
	 * the character's right hand, Up is positive climbing upward. Both read zero
	 * when not climbing.
	 *
	 * GroundSpeed and Direction are useless on a wall: the character is rotated to
	 * face it, and climbing straight up has no horizontal component at all.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float ClimbRightSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float ClimbUpSpeed = 0.f;

	/**
	 * True while the character faces its target rather than its direction of
	 * travel. Direction only sweeps the full -180..180 in this mode; orienting to
	 * movement keeps it pinned near zero.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsStrafing = false;

	/**
	 * True in Idle, Walk, Run and Sprint, the states a single ground blendspace
	 * covers. Not the same as bIsGrounded, which is also true while crouched,
	 * landing, rolling and sliding.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsGroundLocomotion = false;

	/**
	 * Which way a turn in place is going. Latched when the turn starts so a yaw
	 * rate wobbling through zero near the end cannot flip the animation.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsTurningRight = false;

	/** True when the last landing was hard enough for the heavier Land clip. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bHeavyLanding = false;

	/**
	 * Signed yaw rate of the capsule, degrees per second, positive turning right.
	 * Pair it with a low GroundSpeed to detect turning on the spot, which is the
	 * case that foot skates when nothing animates it.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float YawSpeed = 0.f;

	/**
	 * True while the capsule turns on the spot: yaw changing quickly with little
	 * translation. Latched with hysteresis so it cannot flicker at the threshold.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsTurningInPlace = false;

	/** Seconds spent continuously falling, 0 the moment the character lands. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float TimeFalling = 0.f;

	/** Angle of the floor under the capsule in degrees. 0 when flat or airborne. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float FloorAngle = 0.f;

	/** Downward speed at the moment of the last landing, cm/s. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float LastLandingImpactSpeed = 0.f;
};
