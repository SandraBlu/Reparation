// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RLocomotionTypes.generated.h"

class UAnimMontage;
class UAnimSequenceBase;

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
	Slide			UMETA(DisplayName = "Slide"),
	Skydive			UMETA(DisplayName = "Skydive")
};

/**
 * Where a traversal was started from. The same ledge needs a different clip
 * depending on whether the character walked up to it or was hanging off it,
 * and the geometry alone cannot tell the two apart.
 */
UENUM(BlueprintType)
enum class ERTraversalEntry : uint8
{
	Grounded	UMETA(DisplayName = "Grounded"),
	Climbing	UMETA(DisplayName = "Climbing")
};

/**
 * Where a traversal leaves the capsule. The measurements cannot decide this on
 * their own: a tall thin fence can be climbed over or stood on, and which one
 * happens is a property of the clip, not the fence.
 */
UENUM(BlueprintType)
enum class ERTraversalFinish : uint8
{
	/** The movement component's height and depth limits decide. */
	Auto		UMETA(DisplayName = "Auto"),

	/** Standing on top of the obstacle. */
	OnTop		UMETA(DisplayName = "On Top"),

	/** Down on the ground beyond it. Needs the far side to drop away. */
	FarSide		UMETA(DisplayName = "Far Side")
};

/**
 * What the traversal traces measured. Filled by FindTraversal and matched
 * against the FRTraversalAction rows to choose a clip, so that adding a
 * variant is a row of data rather than a state in the animation graph.
 */
USTRUCT(BlueprintType)
struct FRTraversalQuery
{
	GENERATED_BODY()

	/** Capsule position as the move begins. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	FVector Start = FVector::ZeroVector;

	/** Capsule position clearing the top of the obstacle. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	FVector Mid = FVector::ZeroVector;

	/** Capsule position where the move finishes, by the movement component's own verdict. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	FVector End = FVector::ZeroVector;

	/** Where an On Top finish stands the capsule. Valid when bCanFinishOnTop. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	FVector OnTopEnd = FVector::ZeroVector;

	/** Where a Far Side finish lands the capsule. Valid when bCanFinishFarSide. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	FVector FarSideEnd = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	bool bCanFinishOnTop = false;

	/** The far side drops away and the capsule fits on the ground there. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	bool bCanFinishFarSide = false;

	/** Top of the obstacle above the character's feet. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float ObstacleHeight = 0.f;

	/**
	 * Near face to far edge along the approach. Clamped to MaxVaultDepth, which
	 * is also what a surface with no far edge in range reports, so a wall and a
	 * very deep ledge look the same here. Both want a mantle.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float ObstacleDepth = 0.f;

	/** Drop from the top of the obstacle to the far ground. Negative if higher. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float FarSideDrop = 0.f;

	/** True when there is standable ground beyond the obstacle to land on. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	bool bHasFarSideGround = false;

	/** Horizontal speed carried into the obstacle, cm/s. */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float ApproachSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	ERTraversalEntry Entry = ERTraversalEntry::Grounded;

	/**
	 * The movement component's own vault-or-mantle verdict, from its height and
	 * depth limits. Used when the config lists no matching action, so traversal
	 * keeps working with an empty table and the animation rows stay optional.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	ERLocomotionState FallbackState = ERLocomotionState::Vault;
};

/**
 * One traversal variant. Rows are tested in order and the first whose bounds
 * all contain the measured obstacle wins, so order them most specific first.
 */
USTRUCT(BlueprintType)
struct FRTraversalAction
{
	GENERATED_BODY()

	/**
	 * Played by the traversal state's sequence evaluator, scrubbed to the
	 * capsule's progress rather than its own clock.
	 *
	 * A clip with root motion lends the capsule its path: the capsule follows
	 * the root, warped to fit the obstacle, and turns where the root turns.
	 * Tick Enable Root Motion on the clip, or the mesh travels the path a second
	 * time on top of the capsule and snaps back at the end.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	/** Reported as the locomotion state while the move runs. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	ERLocomotionState State = ERLocomotionState::Vault;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	ERTraversalEntry Entry = ERTraversalEntry::Grounded;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.0"))
	float MinObstacleHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.0"))
	float MaxObstacleHeight = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.0"))
	float MinObstacleDepth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.0"))
	float MaxObstacleDepth = 1000.f;

	/**
	 * Speed the character must carry into it. This is what separates a running
	 * hurdle from a standing step over the same fence.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.0"))
	float MinApproachSpeed = 0.f;

	/**
	 * Require somewhere to land past the obstacle. A vault through needs it; a
	 * mantle onto the top does not.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bRequiresFarSideGround = false;

	/** How long the capsule takes. The clip is stretched onto this. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.1"))
	float Duration = 0.6f;

	/**
	 * Where the capsule ends up. Leave on Auto unless the clip disagrees with the
	 * limits: a fence climb taller than MaxVaultHeight still wants Far Side.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	ERTraversalFinish Finish = ERTraversalFinish::Auto;

	/**
	 * Fraction of the clip at which the body passes over the obstacle edge.
	 * Leave at 0 and it is found from the clip: the top of the jump for a
	 * vault, the moment the body is nearly up for a mantle. Set it only if a
	 * clip reaches the edge noticeably early or late.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float ApexTime = 0.f;

	/**
	 * For in place clips that finish facing back the way they came. The capsule
	 * turns 180 degrees as the move completes. Clips with root motion ignore
	 * this: the capsule follows their turn as it happens.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bTurnAround = false;

	/**
	 * Played in the Land state once the move completes. The capsule arrives
	 * resting on the ground, so without this a drop off the far side has no
	 * landing at all.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UAnimSequenceBase> LandingAnimation = nullptr;

	/** Seconds LandingAnimation holds the character. 0 uses the clip length. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.0"))
	float LandingRecoveryTime = 0.f;
};

/**
 * One landing variant. Rows are tested in order and the first that fits wins,
 * the same way traversal actions are, so order them most specific first.
 */
USTRUCT(BlueprintType)
struct FRLandingAction
{
	GENERATED_BODY()

	/** Played by the Land state's sequence player. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	/** Land or Roll. Both play in the same graph state; this is for gameplay and tags. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	ERLocomotionState State = ERLocomotionState::Land;

	/** States the landing may come out of, such as Skydive or Glide. Empty means any. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	TArray<ERLocomotionState> FromStates;

	/**
	 * Speed into the ground, cm/s. Rows replace SoftLandingSpeed entirely, so a
	 * row starting at 0 plays on every step off a kerb.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta = (ClampMin = "0.0"))
	float MinImpactSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta = (ClampMin = "0.0"))
	float MaxImpactSpeed = 100000.f;

	/** Horizontal speed carried into the ground. Separates a running landing from a dead drop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta = (ClampMin = "0.0"))
	float MinGroundSpeed = 0.f;

	/** Seconds the landing holds the character. 0 uses the clip length. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta = (ClampMin = "0.0"))
	float RecoveryTime = 0.f;
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
	Traversal	UMETA(DisplayName = "Traversal"),
	Skydive		UMETA(DisplayName = "Skydive")
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
 * Flight characteristics of one glider. Held on the ARGlider actor and pushed
 * onto the movement component when it is acquired, so a better wing is a better
 * wing rather than a different code path.
 */
USTRUCT(BlueprintType)
struct FRGlideSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float MaxSpeed = 900.f;

	/** Steady descent the glide settles to, cm/s downward. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float DescentRate = 200.f;

	/** How quickly vertical speed converges on DescentRate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float DescentInterpSpeed = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float Acceleration = 1024.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float Friction = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float BrakingDeceleration = 200.f;

	/** Degrees per second the wing turns. The main handling difference. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float RotationRate = 180.f;

	/** Diving steepens the glide and trades height for speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	float DiveSpeedBonus = 400.f;
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

	/**
	 * True once a fall has lasted long enough to read as a real drop rather than
	 * a step off a kerb. A short fall is a tuck; a long one is a body trying to
	 * balance itself, which is a different pose.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsLongFall = false;

	/** Seconds spent continuously falling, 0 the moment the character lands. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float TimeFalling = 0.f;

	/** Angle of the floor under the capsule in degrees. 0 when flat or airborne. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float FloorAngle = 0.f;

	/** Downward speed at the moment of the last landing, cm/s. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float LastLandingImpactSpeed = 0.f;

	/**
	 * Clip the current traversal picked. Drive the traversal state's sequence
	 * evaluator from this and one state covers every vault and mantle variant;
	 * pair it with GetTraversalExplicitTime so the clip tracks the capsule.
	 * Null when the config lists no matching action.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	TObjectPtr<UAnimSequenceBase> TraversalAnim = nullptr;

	/**
	 * Clip the current landing picked, from the landing table or the traversal
	 * that just finished. Bind the Land state's sequence player to this and one
	 * state covers every landing and roll. Null when the landing table is empty.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	TObjectPtr<UAnimSequenceBase> LandingAnim = nullptr;
};
