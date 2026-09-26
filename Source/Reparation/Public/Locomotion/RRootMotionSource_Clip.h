// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RootMotionSource.h"
#include "RRootMotionSource_Clip.generated.h"

class UAnimSequenceBase;

/**
 * Moves the character along a clip's root motion while the clip plays in the
 * animation graph, for clips root locked so the mesh stays on the capsule.
 *
 * A landing roll carries the body three metres; with the root locked and
 * nothing else moving the capsule it would roll on the spot. Running it as a
 * root motion source leaves the movement component in charge, so walls, slopes
 * and gravity all behave as they do for any other movement. Horizontal only.
 */
USTRUCT()
struct REPARATION_API FRRootMotionSource_Clip : public FRootMotionSource
{
	GENERATED_BODY()

	FRRootMotionSource_Clip();

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> Clip = nullptr;

	/**
	 * Takes the root track, which is in mesh space, into world space. Fixed at
	 * the start so the move keeps the heading it began with.
	 */
	UPROPERTY()
	FTransform MeshToWorld = FTransform::Identity;

	virtual FRootMotionSource* Clone() const override;
	virtual bool Matches(const FRootMotionSource* Other) const override;
	virtual void PrepareRootMotion(float SimulationTime, float MovementTickTime,
		const ACharacter& Character, const UCharacterMovementComponent& MoveComponent) override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
};

template<>
struct TStructOpsTypeTraits<FRRootMotionSource_Clip> : public TStructOpsTypeTraitsBase2<FRRootMotionSource_Clip>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
