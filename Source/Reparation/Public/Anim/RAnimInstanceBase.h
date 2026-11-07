// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "Locomotion/RLocomotionTypes.h"
#include "RAnimInstanceBase.generated.h"

class URLocomotionComponent;
class ARCharacterBase;
class UAnimSequenceBase;
/**
 * 
 */
UCLASS()
class REPARATION_API URAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

public:
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** Enable only on the main player ABP, never its linked layer instances. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon Layers") bool bManageWeaponLayers = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Weapon Layers") TSubclassOf<UAnimInstance> KatanaAnimationLayers;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Weapon Layers") TSubclassOf<UAnimInstance> BowAnimationLayers;
	UPROPERTY(BlueprintReadOnly, Transient, Category="Weapon Layers") float WeaponGroundSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly, Transient, Category="Weapon Layers") float WeaponDirection = 0.f;
    UPROPERTY(BlueprintReadOnly, Transient, Category="Bow|Aim") float BowAimAlpha = 0.f;
    UPROPERTY(BlueprintReadOnly, Transient, Category="Bow|Aim") float BowAimYaw = 0.f;
    UPROPERTY(BlueprintReadOnly, Transient, Category="Bow|Aim") float BowAimPitch = 0.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Bow|Aim", meta=(ClampMin="0.1")) float BowAimBlendSpeed = 10.f;
	UPROPERTY(BlueprintReadOnly, Transient, Category="Weapon Layers") TSubclassOf<UAnimInstance> ActiveWeaponAnimationLayers;
	/** Also callable after an equipment change; NativeUpdate handles direct equip/holster paths. */
	UFUNCTION(BlueprintCallable, Category="Weapon Layers") void RefreshWeaponAnimationLayers();
	/** Override automatic equipment selection. None restores the original movement poses until reset. */
	UFUNCTION(BlueprintCallable, Category="Weapon Layers") void SetWeaponAnimationLayerOverride(TSubclassOf<UAnimInstance> LayerClass);
	/** Return control to automatic equipment selection. */
	UFUNCTION(BlueprintCallable, Category="Weapon Layers") void ResetWeaponAnimationLayerOverride();
	UPROPERTY(BlueprintReadOnly, Transient, Category="Weapon Layers") bool bHasWeaponAnimationLayerOverride = false;

	/** Opt in only on animation blueprints that contain the terrain foot-placement graph. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Foot IK")
	bool bEnableFootIK = false;

	/** Blend back into terrain alignment after landing or a montage finishes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Foot IK", meta=(ClampMin="0.1", ClampMax="30"))
	float FootIKBlendInSpeed = 8.f;

	/** Ground-only weight. Montages/root motion/traversal bypass IK to preserve authored foot lifts. */
	UPROPERTY(BlueprintReadOnly, Transient, Category="Foot IK")
	float FootIKAlpha = 0.f;

private:
	friend class FRTerrainFootIKTest;

	/** Caches the owner and its locomotion component. Safe to call repeatedly. */
	void ResolveLocomotionComponent();
	void UpdateFootIK(float DeltaSeconds);
	UPROPERTY(Transient) TSubclassOf<UAnimInstance> WeaponAnimationLayerOverride;
	
protected:
	UPROPERTY()
	ARCharacterBase* OwningCharacter;

	UPROPERTY()
	URLocomotionComponent* LocomotionComponent;

	/** Snapshot copied on the game thread, safe to read from the worker thread. */
	UPROPERTY(BlueprintReadOnly, Category = "AnimData|Locomotion")
	FRLocomotionAnimData Locomotion;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool DoesOwnerHaveTag(FGameplayTag TagToCheck) const;

	/**
	 * Explicit time for a sequence evaluator playing the current vault or mantle.
	 *
	 * The traversal clips carry no root motion, so the capsule owns the movement
	 * and the clip is stretched onto it. Reading the length from the asset keeps
	 * the graph correct if the clip is ever retimed.
	 */
	UFUNCTION(BlueprintPure, Category = "AnimData|Locomotion", meta = (BlueprintThreadSafe))
	float GetTraversalExplicitTime(const UAnimSequenceBase* Sequence) const;

	/** As above, for a length the graph already knows rather than an asset. */
	UFUNCTION(BlueprintPure, Category = "AnimData|Locomotion", meta = (BlueprintThreadSafe))
	float GetTraversalExplicitTimeForLength(float SequenceLength) const;

	
};
