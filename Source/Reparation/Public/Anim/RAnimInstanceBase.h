// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "Locomotion/RLocomotionTypes.h"
#include "RAnimInstanceBase.generated.h"

class UCharacterMovementComponent;
class URLocomotionComponent;
class ARCharacterBase;
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
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds);
	
protected:
	UPROPERTY()
	ARCharacterBase* OwningCharacter;
	UPROPERTY()
	UCharacterMovementComponent* OwningMovementComponent;

	UPROPERTY()
	URLocomotionComponent* LocomotionComponent;

	/** Snapshot copied on the game thread, safe to read from the worker thread. */
	UPROPERTY(BlueprintReadOnly, Category = "AnimData|Locomotion")
	FRLocomotionAnimData Locomotion;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AnimData|LocomotionData")
	float GroundSpeed;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bHasAcceleration;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AnimData|LocomotionData")
	float LocomotionDirection;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool DoesOwnerHaveTag(FGameplayTag TagToCheck) const;

	
};
