// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Locomotion/RLocomotionTypes.h"
#include "Reparation/Reparation.h"
#include "RCharacterMovementComponent.generated.h"

/**
 * Movement component for Reparation characters.
 *
 * Ground, air, water and flight use the engine's built in modes. Glide, climb and
 * traversal (vault and mantle) are MOVE_Custom modes integrated here. No network
 * prediction is implemented, so custom modes are locally authoritative only.
 */
UCLASS()
class REPARATION_API URCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	URCharacterMovementComponent();

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual bool CanAttemptJump() const override;
	virtual FVector ScaleInputAcceleration(const FVector& InputAcceleration) const override;

	/**
	 * Ground steeper than this cannot be ascended: the uphill part of movement
	 * input is stripped. Pushed from URLocomotionConfig so it stays in step with
	 * the slide threshold rather than drifting as a second number.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxAscendableSlopeAngle = 40.f;

	/** Pushes a gait tier onto the walk/crouch speed and handling properties. */
	void ApplyGaitSettings(const FRGaitSettings& Settings);

	void ApplySwimSettings(float InMaxSpeed, float InAcceleration);

	void ApplyFlySettings(float InMaxSpeed, float InAcceleration, float InBrakingDeceleration);

	void SetCustomMovementMode(ERCustomMovementMode Mode);

	ERCustomMovementMode GetCustomMovementModeEnum() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool IsCustomMode(ERCustomMovementMode Mode) const;

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool IsGliding() const { return IsCustomMode(ERCustomMovementMode::Glide); }

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool IsClimbing() const { return IsCustomMode(ERCustomMovementMode::Climb); }

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool IsTraversing() const { return IsCustomMode(ERCustomMovementMode::Traversal); }

	/** 0 to 1 progress through the current traversal, 0 when not traversing. */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	float GetTraversalAlpha() const;

	/** Fraction of the capsule below the water surface, 0 when dry, 1 fully submerged. */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	float GetImmersionFraction() const;

	/** Angle of the floor under the capsule, degrees. 0 when flat or airborne. */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	float GetFloorAngle() const;

	/** Normal of the floor under the capsule. Zero when airborne. */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	FVector GetFloorNormal() const;

	/** Low friction and braking are what let gravity carry a slide down a slope. */
	void ApplySlideSettings(float InMaxSpeed, float InFriction, float InBrakingDeceleration);

	// --- Climb ---

	/**
	 * Sweeps forward for a climbable surface.
	 *
	 * A surface qualifies only when it is steep enough AND its actor or component
	 * carries ClimbableTag. Requiring the tag keeps the character from sticking to
	 * every wall in the level.
	 */
	bool FindClimbableSurface(FHitResult& OutHit) const;

	/** Outward normal of the surface being climbed. Zero when not climbing. */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	FVector GetClimbSurfaceNormal() const { return ClimbSurfaceNormal; }

	/** True once the climb reaches the top of the wall, so a mantle can take over. */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool IsAtClimbLedge() const { return bAtClimbLedge; }

	// --- Traversal (vault and mantle) ---

	/**
	 * Looks for an obstacle ahead that can be crossed.
	 *
	 * Thin obstacles up to MaxVaultHeight resolve as a vault that lands beyond
	 * them; thicker or taller ones up to MaxMantleHeight resolve as a mantle that
	 * lands on top. OutType reports which.
	 */
	bool FindTraversal(FVector& OutStart, FVector& OutMid, FVector& OutEnd, ERLocomotionState& OutType) const;

	/** Begins a scripted move along Start, Mid, End. Duration is clamped to a sane minimum. */
	void BeginTraversal(const FVector& Start, const FVector& Mid, const FVector& End, float Duration);

	// --- Glide tuning ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Glide")
	float GlideMaxSpeed = 900.f;

	/** Steady descent the glide settles to, cm/s downward. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Glide")
	float GlideDescentRate = 200.f;

	/** How quickly vertical speed converges on GlideDescentRate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Glide")
	float GlideDescentInterpSpeed = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Glide")
	float GlideAcceleration = 1024.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Glide")
	float GlideFriction = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Glide")
	float GlideBrakingDeceleration = 200.f;

	/** Diving steepens the glide and trades height for speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Glide")
	float GlideDiveSpeedBonus = 400.f;

	// --- Climb tuning ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbMaxSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbAcceleration = 512.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbBrakingDeceleration = 1024.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbFriction = 4.f;

	/** How far ahead of the capsule to look for a wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbDetectionDistance = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbTraceRadius = 28.f;

	/** Distance held between the capsule centre and the wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbWallOffset = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbSnapSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	float ClimbRotationRate = 360.f;

	/**
	 * Actors or components must carry this tag to be climbable. Set on the actor
	 * (Actor Tags) or on the specific primitive (Component Tags).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	FName ClimbableTag = FName("Climbable");

	/**
	 * Trace channel that defines climbable geometry. Set a mesh to block this
	 * channel and it is climbable, which scales to a landscape far better than
	 * tagging every actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	TEnumAsByte<ECollisionChannel> ClimbTraceChannel = ECC_Climbable;

	/** Set false to climb any sufficiently steep surface, ignoring ClimbableTag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Climb")
	bool bRequireClimbableTag = false;

	// --- Traversal tuning ---

	/** Obstacles up to this high are vaulted over. Logs, fences, low walls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Traversal")
	float MaxVaultHeight = 120.f;

	/** Obstacles up to this high are mantled onto instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Traversal")
	float MaxMantleHeight = 220.f;

	/** Anything deeper than this is mantled rather than vaulted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Traversal")
	float MaxVaultDepth = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Traversal")
	float TraversalDetectionDistance = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Traversal")
	float VaultDuration = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Traversal")
	float MantleDuration = 0.75f;

	/** Extra clearance required above a landing spot before it is accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Traversal")
	float TraversalClearance = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

protected:

	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	void PhysGlide(float DeltaTime, int32 Iterations);
	void PhysClimb(float DeltaTime, int32 Iterations);
	void PhysTraversal(float DeltaTime, int32 Iterations);

private:

	/** Refreshes ClimbSurfaceNormal; false when the wall is gone. */
	bool UpdateClimbSurface();

	/** Traces above the capsule to see whether the wall has topped out. */
	bool CheckForClimbLedge() const;

	void RotateTowardClimbSurface(float DeltaTime);

	bool IsSurfaceClimbable(const FHitResult& Hit) const;

	/** Capsule fits at Location with nothing overlapping. */
	bool HasRoomAt(const FVector& Location) const;

	/** Captured in the constructor so leaving a slide can put friction back. */
	float DefaultGroundFriction = 8.f;

	FVector ClimbSurfaceNormal = FVector::ZeroVector;
	bool bAtClimbLedge = false;

	// Scripted traversal path, valid only while in the Traversal custom mode.
	FVector TraversalStart = FVector::ZeroVector;
	FVector TraversalMid = FVector::ZeroVector;
	FVector TraversalEnd = FVector::ZeroVector;
	float TraversalDuration = 0.f;
	float TraversalElapsed = 0.f;
};
