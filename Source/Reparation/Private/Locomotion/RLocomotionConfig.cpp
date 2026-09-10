// Fill out your copyright notice in the Description page of Project Settings.


#include "Locomotion/RLocomotionConfig.h"

URLocomotionConfig::URLocomotionConfig()
{
	WalkSettings.MaxSpeed = 175.f;
	WalkSettings.MaxAcceleration = 1024.f;
	WalkSettings.BrakingDeceleration = 1024.f;
	WalkSettings.RotationRate = 540.f;

	RunSettings.MaxSpeed = 450.f;
	RunSettings.MaxAcceleration = 2048.f;
	RunSettings.BrakingDeceleration = 1024.f;
	RunSettings.RotationRate = 540.f;

	SprintSettings.MaxSpeed = 700.f;
	SprintSettings.MaxAcceleration = 2048.f;
	SprintSettings.BrakingDeceleration = 512.f;
	SprintSettings.RotationRate = 300.f;

	CrouchSettings.MaxSpeed = 200.f;
	CrouchSettings.MaxAcceleration = 1024.f;
	CrouchSettings.BrakingDeceleration = 1024.f;
	CrouchSettings.RotationRate = 360.f;
}

const FRGaitSettings& URLocomotionConfig::GetGaitSettings(ERGait Gait, ERStance Stance) const
{
	if (Stance == ERStance::Crouching)
	{
		return CrouchSettings;
	}

	switch (Gait)
	{
	case ERGait::Walk:		return WalkSettings;
	case ERGait::Sprint:	return SprintSettings;
	case ERGait::Run:
	default:				return RunSettings;
	}
}

const FRLocomotionTransition* URLocomotionConfig::FindTransition(ERLocomotionState From, ERLocomotionState To) const
{
	const FRLocomotionTransition* Best = nullptr;

	for (const FRLocomotionTransition& Transition : Transitions)
	{
		if (Transition.ToState != To)
		{
			continue;
		}

		// An empty FromStates list matches any source state.
		if (Transition.FromStates.Num() > 0 && !Transition.FromStates.Contains(From))
		{
			continue;
		}

		if (!Best || Transition.Priority > Best->Priority)
		{
			Best = &Transition;
		}
	}

	return Best;
}

bool URLocomotionConfig::IsTransitionAllowed(ERLocomotionState From, ERLocomotionState To) const
{
	const FRLocomotionTransition* Transition = FindTransition(From, To);
	return !Transition || !Transition->bBlocked;
}
