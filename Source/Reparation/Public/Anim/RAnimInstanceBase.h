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
