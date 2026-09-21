// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/RAnimInstanceBase.h"

#include "KismetAnimationLibrary.h"
#include "Characters/RCharacterBase.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "Locomotion/RLocomotionComponent.h"


void URAnimInstanceBase::NativeInitializeAnimation()
{
	OwningCharacter = Cast<ARCharacterBase>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
		LocomotionComponent = OwningCharacter->FindComponentByClass<URLocomotionComponent>();
	}
}

void URAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Copied here rather than in the thread safe pass so the worker thread never
	// touches the locomotion component directly.
	if (LocomotionComponent)
	{
		Locomotion = LocomotionComponent->GetAnimData();
		GroundSpeed = Locomotion.GroundSpeed;
		LocomotionDirection = Locomotion.Direction;
	}
}

void URAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}
	//GroundSpeed = OwningCharacter->GetVelocity().Size2D();
	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D()>0.f;
	//LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(OwningCharacter->GetVelocity(),OwningCharacter->GetActorRotation());
}

bool URAnimInstanceBase::DoesOwnerHaveTag(FGameplayTag TagToCheck) const
{
	if (APawn* OwningPawn = TryGetPawnOwner())
	{
		return URAbilitySystemLibrary::NativeDoesActorHaveTag(OwningPawn,TagToCheck);
	}

	return false;
}

float URAnimInstanceBase::GetTraversalExplicitTime(const UAnimSequenceBase* Sequence) const
{
	return Sequence ? GetTraversalExplicitTimeForLength(Sequence->GetPlayLength()) : 0.f;
}

float URAnimInstanceBase::GetTraversalExplicitTimeForLength(float SequenceLength) const
{
	// Alpha is already clamped by the movement component, but a sequence evaluator
	// fed a time past the clip end is a hard-to-spot pop, so clamp defensively.
	return FMath::Clamp(Locomotion.TraversalAlpha, 0.f, 1.f) * FMath::Max(0.f, SequenceLength);
}
