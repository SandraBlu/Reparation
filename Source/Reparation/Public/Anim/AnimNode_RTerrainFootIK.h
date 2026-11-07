#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Engine/EngineTypes.h"
#include "AnimNode_RTerrainFootIK.generated.h"

/** Terrain deltas are measured against the capsule's floor, never last frame's corrected foot Z. */
USTRUCT(BlueprintInternalUseOnly)
struct REPARATION_API FAnimNode_RTerrainFootIK : public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Bones")
	FBoneReference Pelvis;
	UPROPERTY(EditAnywhere, Category="Bones")
	FBoneReference LeftFoot;
	UPROPERTY(EditAnywhere, Category="Bones")
	FBoneReference RightFoot;

	UPROPERTY(EditAnywhere, Category="Terrain", meta=(ClampMin="1", ClampMax="100"))
	float TraceAboveFloor = 45.f;
	UPROPERTY(EditAnywhere, Category="Terrain", meta=(ClampMin="1", ClampMax="100"))
	float TraceBelowFloor = 55.f;
	UPROPERTY(EditAnywhere, Category="Terrain", meta=(ClampMin="0", ClampMax="40"))
	float MaxFootOffset = 20.f;
	UPROPERTY(EditAnywhere, Category="Terrain", meta=(ClampMin="0", ClampMax="30"))
	float MaxPelvisDrop = 15.f;
	/** Leave this much leg reach unused on uneven ground, using bounded pelvis lowering. Zero disables it. */
	UPROPERTY(EditAnywhere, Category="Terrain", meta=(ClampMin="0", ClampMax="5", Units="cm"))
	float LegExtensionReserve = 2.f;
	UPROPERTY(EditAnywhere, Category="Terrain", meta=(ClampMin="0", ClampMax="40"))
	float MaxFootTiltDegrees = 25.f;
	UPROPERTY(EditAnywhere, Category="Terrain", meta=(ClampMin="0.1", ClampMax="30"))
	float InterpolationSpeed = 12.f;
	/** Reference knee hint and near-extension stabilization range, in cm. */
	UPROPERTY(EditAnywhere, Category="Terrain", meta=(ClampMin="0.1", ClampMax="15"))
	float KneeDirectionBias = 6.f;
	UPROPERTY(EditAnywhere, Category="Terrain")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDrawGroundTraces = false;

	FAnimNode_RTerrainFootIK();
	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;

private:
	friend class FRTerrainFootIKTest;
	float ReachSafePelvisDrop(const FVector& Hip, const FVector& Target, const FVector& Up,
		float LegLength, float Reserve, float CurrentDrop) const;
	FVector ReachSafeFootTarget(const FVector& Hip, const FVector& Target, float LegLength, float Reserve) const;
	FVector StableKneeDirection(const FVector& Hip, const FVector& Knee, const FVector& Foot,
		const FVector& DesiredAxis, const FQuat& PelvisRotation, int32 Leg) const;
	FVector ReferenceBendInPelvisSpace[2] = {FVector::ForwardVector,FVector::ForwardVector};
	float TargetOffsets[2] = {};
	float SmoothedOffsets[2] = {};
	FVector TargetNormals[2] = {FVector::UpVector,FVector::UpVector};
	FVector SmoothedNormals[2] = {FVector::UpVector,FVector::UpVector};
};
