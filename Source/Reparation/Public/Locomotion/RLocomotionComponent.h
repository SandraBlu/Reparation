// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Locomotion/RLocomotionTypes.h"
#include "RLocomotionComponent.generated.h"

class ACharacter;
class UAbilitySystemComponent;
class URCharacterMovementComponent;
class URLocomotionConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLocomotionStateChanged, ERLocomotionState, PreviousState, ERLocomotionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLandedWithImpact, float, ImpactSpeed, float, FallDuration);

/**
 * Owns the character's locomotion state machine.
 *
 * This is the only thing permitted to change movement mode or gait. It reads the
 * movement component each tick, decides what state the character should be in,
 * validates the edge against URLocomotionConfig, and publishes the result three
 * ways: a delegate, gameplay tags on the owner's ASC, and a plain snapshot the
 * anim instance can read off the game thread.
 */
UCLASS(ClassGroup = (Locomotion), meta = (BlueprintSpawnableComponent))
class REPARATION_API URLocomotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	URLocomotionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Input surface. Call these from the owning pawn's input handlers. ---

	/** Raw 2D movement input, already consumed by AddMovementInput. Used for gait and anim. */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void SetMovementInput(const FVector2D& InInput);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void SetSprintHeld(bool bHeld);

	/** Forces the walk tier while held, for slow approaches and stealth. */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void SetWalkHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void ToggleCrouch();

	/** Jumps on land, or pushes off the surface when swimming. Returns false if refused. */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	bool TryJump();

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void StopJumping();

	/** -1 dives, +1 surfaces. Ignored outside water. */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void SetSwimVerticalInput(float InVertical);

	/** Hold to keep the glider deployed. Releasing folds it and resumes falling. */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void SetGlideHeld(bool bHeld);

	/** Hold to cling to a climbable surface; releasing lets go. */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void SetClimbHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void ToggleFlight();

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void SetFlying(bool bEnabled);

	/** Vaults or mantles the obstacle ahead. False when there is nothing to cross. */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	bool TryTraversal();

	/** True when a glider is carried and the character has fallen long enough to deploy. */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool CanDeployGlider() const;

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	ERLocomotionState GetLocomotionState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	ERGait GetGait() const { return CurrentGait; }

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	ERStance GetStance() const { return CurrentStance; }

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	const FRLocomotionAnimData& GetAnimData() const { return AnimData; }

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool IsInWater() const;

	/** True while clinging to a wall. The pawn defers movement input when set. */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool IsClimbing() const;

	/**
	 * True only in Idle, Walk, Run and Sprint. Climbing, gliding, swimming and
	 * traversal each steer the character themselves, so a target lock there would
	 * fight them for rotation.
	 */
	UFUNCTION(BlueprintPure, Category = "Locomotion")
	bool CanTarget() const;

	/** Requests a state directly. Still validated against the transition table. */
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	bool RequestState(ERLocomotionState NewState);

	UPROPERTY(BlueprintAssignable, Category = "Locomotion")
	FOnLocomotionStateChanged OnLocomotionStateChanged;

	/**
	 * Every touchdown, including the light ones that enter no landing state at
	 * all. Fires after the impact figures are written, so a fall damage effect
	 * can hang off this rather than racing the character's own LandedDelegate
	 * for the same frame.
	 *
	 * FallDuration is passed because TimeFalling is reset on touchdown and would
	 * otherwise be gone by the time anyone could read it.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Locomotion")
	FOnLandedWithImpact OnLandedImpact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
	TObjectPtr<URLocomotionConfig> Config;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);

	UFUNCTION()
	void HandleMovementModeChanged(ACharacter* Character, EMovementMode PreviousMode, uint8 PreviousCustomMode);

private:

	/** Falls back to the class defaults when no config asset is assigned. */
	const URLocomotionConfig& GetConfigRef() const;

	/** What the character should be in right now, before the edge is validated. */
	ERLocomotionState EvaluateDesiredState() const;

	/**
	 * MontagePlayRate scales the edge's transition montage. Traversal uses it to
	 * stretch a fixed length in place clip onto a move whose duration gameplay
	 * decides, so the feet finish exactly when the capsule does.
	 */
	bool EnterState(ERLocomotionState NewState, float MontagePlayRate = 1.f);

	/** Starts and stops glide, climb and traversal before the state is evaluated. */
	void UpdateVolumetricModes(float DeltaTime);

	/** Mirrors the character's actual crouch state, which can refuse to change. */
#if !UE_BUILD_SHIPPING
	/** On screen state readout, toggled with r.Reparation.DebugLocomotion 1. */
	void DrawDebugState() const;
#endif

	void UpdateStance();
	void UpdateGait();
	void ApplyMovementSettings();

	/** Faces the control rotation while the owner is targeting, so the mesh can strafe. */
	void UpdateStrafe();

	/** Pushes the character down the slope, which walking mode will not do. */
	void UpdateSlide(float DeltaTime);
	void UpdateAnimData(float DeltaTime);
	void ApplyStateTags(ERLocomotionState PreviousState, ERLocomotionState NewState);

	static bool IsGroundState(ERLocomotionState State);
	static bool IsWaterState(ERLocomotionState State);

	FGameplayTagContainer GetTagsForState(ERLocomotionState State) const;

	UAbilitySystemComponent* GetOwnerASC() const;

	UPROPERTY()
	TObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY()
	TObjectPtr<URCharacterMovementComponent> MovementComponent;

	ERLocomotionState CurrentState = ERLocomotionState::Idle;
	ERGait CurrentGait = ERGait::Run;
	ERStance CurrentStance = ERStance::Standing;

	FRLocomotionAnimData AnimData;

	FVector2D MovementInput = FVector2D::ZeroVector;
	float SwimVerticalInput = 0.f;
	bool bSprintHeld = false;
	bool bWalkHeld = false;
	bool bGlideHeld = false;
	bool bClimbHeld = false;
	bool bIsStrafing = false;

	/** Previous frame actor yaw, for the turn rate published to the anim data. */
	float PreviousYaw = 0.f;

	/** Seconds spent continuously falling, used to gate glider deployment. */
	float TimeFalling = 0.f;

	/** Which traversal is running, so the state machine can report Vault or Mantle. */
	ERLocomotionState ActiveTraversalType = ERLocomotionState::Vault;

	/** Seconds remaining before another state change is considered. */
	float StateLockRemaining = 0.f;

	/** Set on jump so the ascent reads as Jump rather than Fall. */
	bool bJumpInitiated = false;

	/** When the jump key was last pressed, for detecting the glide double tap. */
	float LastJumpPressTime = 0.f;

	/** Tags currently pushed onto the ASC, so they can be removed cleanly. */
	FGameplayTagContainer AppliedTags;
};
