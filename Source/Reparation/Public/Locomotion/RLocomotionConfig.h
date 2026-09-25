// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Locomotion/RLocomotionTypes.h"
#include "RLocomotionConfig.generated.h"

/**
 * Tuning for one character's locomotion. Assign to URLocomotionComponent so
 * player, companion and enemy variants can share a state machine but not feel
 * the same.
 */
UCLASS(BlueprintType)
class REPARATION_API URLocomotionConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	URLocomotionConfig();

	/** Ground handling per gait tier while standing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground")
	FRGaitSettings WalkSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground")
	FRGaitSettings RunSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground")
	FRGaitSettings SprintSettings;

	/** Replaces the gait tier entirely while crouched. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground")
	FRGaitSettings CrouchSettings;

	/** Below this horizontal speed the character reads as Idle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float IdleSpeedThreshold = 10.f;

	/** Sprint is refused below this forward input magnitude. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SprintInputThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air")
	float JumpZVelocity = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air")
	float AirControl = 0.25f;

	/** Downward speed above which landing plays the Land state rather than resuming directly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float SoftLandingSpeed = 500.f;

	/** Downward speed above which landing rolls instead. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float HardLandingSpeed = 1000.f;

	/**
	 * Within the Land state, the speed above which the heavier of the two landing
	 * clips is used. Sits between SoftLandingSpeed and HardLandingSpeed, which
	 * decide whether there is a landing state at all and whether it rolls.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float HeavyLandingSpeed = 750.f;

	/** How long the Land state is held before returning to ground locomotion. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float LandRecoveryTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water")
	float SwimSurfaceSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water")
	float SwimUnderwaterSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water")
	float SwimAcceleration = 1024.f;

	/**
	 * Immersion fraction at which surface swimming becomes underwater swimming.
	 * UCharacterMovementComponent reports 1.0 fully submerged.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UnderwaterImmersionThreshold = 0.85f;

	/** Hysteresis so bobbing at the surface does not flip the state every frame. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float ImmersionHysteresis = 0.08f;

	/** Upward velocity applied when surfacing out of water onto land. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water")
	float WaterExitBoost = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float FlyMaxSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float FlyAcceleration = 2048.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float FlyBrakingDeceleration = 1024.f;

	/** Falling must last this long before the glider can deploy, so a hop does not trigger it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide", meta = (ClampMin = "0.0"))
	float MinFallTimeBeforeGlide = 0.25f;

	/**
	 * Seconds of falling after which a character who knows the skill drops into a
	 * skydive. Longer than the glider gate, so stepping off a ledge is still just
	 * a step.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skydive", meta = (ClampMin = "0.0"))
	float MinFallTimeBeforeSkydive = 0.8f;

	/** Seconds of falling after which bIsLongFall is set, for the airborne pose. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float LongFallTime = 0.8f;

	/**
	 * Seconds within which a second jump press counts as a double tap, which
	 * deploys or stows the wing. A toggle rather than a hold, so a long glide
	 * does not mean a held key.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide", meta = (ClampMin = "0.0"))
	float GlideToggleDoubleTapWindow = 0.3f;

	/**
	 * Tag the owner must carry before gliding is offered, granted when the player
	 * finds a glider. Leave unset to allow gliding unconditionally.
	 *
	 * A tag rather than a flag on the component, so the unlock works the same way
	 * as any other learned traversal skill, and so a debuff or a no fly zone can
	 * suppress it without anyone writing glider specific code.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	FGameplayTag GlideUnlockTag;

	/**
	 * Learned skill that turns a lethal drop into a controlled one. Without it a
	 * long fall is simply a fall. Leave unset to allow skydiving unconditionally.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skydive")
	FGameplayTag SkydiveUnlockTag;

	/**
	 * Handling while skydiving. The same shape as a wing because it is the same
	 * physics: a far steeper descent and less horizontal reach, but steerable.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skydive")
	FRGlideSettings SkydiveSettings;

	/** Topping out a climb hands off to a mantle automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bAutoMantleAtClimbLedge = true;

	/**
	 * Traversal variants, tested in order, first match wins. Order them most
	 * specific first: a running hurdle before a standing step over the same
	 * fence, since the hurdle is the one with the speed requirement.
	 *
	 * Leave it empty and traversal still works, falling back on the movement
	 * component's vault and mantle limits with no clip chosen.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TArray<FRTraversalAction> TraversalActions;

	/** Sideways push given when jumping off a climbed wall. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climb")
	float ClimbJumpAwaySpeed = 250.f;

	/** Strafe while the owner carries status.targeting, facing the aim rather than the path. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strafe")
	bool bStrafeWhileTargeting = true;

	/**
	 * Replaces the gait tier outright while strafing, the same way CrouchSettings
	 * does while crouched. Locking on is meant to be slower and turn faster.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strafe")
	FRGaitSettings TargetingSettings;

	/** Yaw rate above which the character reads as turning on the spot, deg/s. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (ClampMin = "0.0"))
	float TurnInPlaceYawSpeed = 45.f;

	/** How far the yaw rate must fall below the threshold before the turn ends. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (ClampMin = "0.0"))
	float TurnInPlaceYawSpeedHysteresis = 35.f;

	/** Above this ground speed a turn is a moving turn, not a turn on the spot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (ClampMin = "0.0"))
	float TurnInPlaceMaxGroundSpeed = 60.f;

	/** Floor angle at which the character starts sliding, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float SlideMinSlopeAngle = 40.f;

	/** How far the angle must drop below the threshold before the slide releases. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "0.0"))
	float SlideSlopeAngleHysteresis = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide")
	float SlideMaxSpeed = 900.f;

	/** Low friction is what lets gravity carry the slide rather than the legs. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "0.0"))
	float SlideGroundFriction = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "0.0"))
	float SlideBrakingDeceleration = 200.f;

	/**
	 * Speed down the face needed to start a slide. Angle alone is not enough,
	 * since a ramp is equally steep walking up it, and standing still must not
	 * start one. Once sliding, any downhill movement at all sustains it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "0.0"))
	float SlideMinDownhillSpeed = 0.f;

	/**
	 * Downhill acceleration applied while sliding. Walking mode holds the capsule
	 * on walkable floors and never pulls it down them, so without this a slide is
	 * only slippery walking.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "0.0"))
	float SlideAcceleration = 1200.f;

	/**
	 * Degrees per second the character turns to face downhill while sliding.
	 * Orient to movement cannot do this: it derives facing from acceleration, and
	 * a slope this steep has none, so the character would slide down backwards
	 * still facing up the hill.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "0.0"))
	float SlideRotationRate = 360.f;

	/**
	 * Legal edges. Any edge not listed is permitted with no montage and no lock;
	 * list an edge only to attach cosmetics, a lock, or to block it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions")
	TArray<FRLocomotionTransition> Transitions;

	/** Resolved settings for a gait/stance pair. */
	const FRGaitSettings& GetGaitSettings(ERGait Gait, ERStance Stance) const;

	/** Highest priority matching entry, or nullptr when the edge is unlisted. */
	const FRLocomotionTransition* FindTransition(ERLocomotionState From, ERLocomotionState To) const;

	/** False only when an explicit blocking entry matches. */
	bool IsTransitionAllowed(ERLocomotionState From, ERLocomotionState To) const;

	/** First action whose bounds all contain the measurement, or nullptr. */
	const FRTraversalAction* FindTraversalAction(const FRTraversalQuery& Query) const;
};
