// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/RAnimInstanceBase.h"

#include "Characters/RCharacterBase.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Animation/AnimSequenceBase.h"
#include "Locomotion/RLocomotionComponent.h"


void URAnimInstanceBase::NativeInitializeAnimation()
{
	ResolveLocomotionComponent();
}

void URAnimInstanceBase::ResolveLocomotionComponent()
{
	OwningCharacter = Cast<ARCharacterBase>(TryGetPawnOwner());
	if (!OwningCharacter)
	{
		return;
	}

	LocomotionComponent = OwningCharacter->FindComponentByClass<URLocomotionComponent>();
}

void URAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Anim initialisation can run before the owning pawn is fully set up, which
	// left the snapshot at its defaults for the whole session: State reading Idle
	// and GroundSpeed zero no matter what the character did. Re-resolve instead of
	// trusting the one attempt at init.
	if (!LocomotionComponent)
	{
		ResolveLocomotionComponent();
	}

	// Copied here rather than in the thread safe pass so the worker thread never
	// touches the locomotion component directly.
	if (LocomotionComponent)
	{
		Locomotion = LocomotionComponent->GetAnimData();
	}
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
