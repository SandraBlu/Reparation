// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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

	/** When true the character must be carrying a glider before gliding is offered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glide")
	bool bRequireGliderEquipped = true;

	/** Topping out a climb hands off to a mantle automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bAutoMantleAtClimbLedge = true;

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
	float SlideMinSlopeAngle = 35.f;

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
	 * Upward speed above which the character is climbing the slope rather than
	 * losing to it, so no slide. A ramp is equally steep walking up it, which is
	 * why angle alone is not enough. Near zero still slides, so standing on steep
	 * ground starts one.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "0.0"))
	float SlideMaxUphillSpeed = 10.f;

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
};
