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

	// Locked on: slower than a walk, and turning fast enough to keep facing a
	// circling target.
	TargetingSettings.MaxSpeed = 150.f;
	TargetingSettings.MaxAcceleration = 1024.f;
	TargetingSettings.BrakingDeceleration = 1024.f;
	TargetingSettings.RotationRate = 720.f;

	// A wing with the lift taken out. Steep and quick, still steerable, and much
	// less horizontal reach than a glider: surviving the drop, not crossing a
	// valley.
	SkydiveSettings.MaxSpeed = 400.f;
	SkydiveSettings.DescentRate = 1400.f;
	SkydiveSettings.DescentInterpSpeed = 4.f;
	SkydiveSettings.Acceleration = 700.f;
	SkydiveSettings.Friction = 0.6f;
	SkydiveSettings.BrakingDeceleration = 200.f;
	SkydiveSettings.RotationRate = 120.f;
	SkydiveSettings.DiveSpeedBonus = 400.f;
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

const FRTraversalAction* URLocomotionConfig::FindTraversalAction(const FRTraversalQuery& Query) const
{
	// First match rather than best match. Ranking rows by how tightly they fit
	// would make the outcome depend on bounds a designer set for readability,
	// which is a poor thing to hinge an animation choice on. Order is explicit.
	for (const FRTraversalAction& Action : TraversalActions)
	{
		if (Action.Entry != Query.Entry)
		{
			continue;
		}

		if (Query.ObstacleHeight < Action.MinObstacleHeight || Query.ObstacleHeight > Action.MaxObstacleHeight)
		{
			continue;
		}

		if (Query.ObstacleDepth < Action.MinObstacleDepth || Query.ObstacleDepth > Action.MaxObstacleDepth)
		{
			continue;
		}

		if (Query.ApproachSpeed < Action.MinApproachSpeed)
		{
			continue;
		}

		if (Action.bRequiresFarSideGround && !Query.bHasFarSideGround)
		{
			continue;
		}

		// A clip that climbs over cannot be used where there is nowhere to land,
		// nor one that stands on top where the capsule would not fit.
		if ((Action.Finish == ERTraversalFinish::OnTop && !Query.bCanFinishOnTop)
			|| (Action.Finish == ERTraversalFinish::FarSide && !Query.bCanFinishFarSide))
		{
			continue;
		}

		return &Action;
	}

	return nullptr;
}

const FRLandingAction* URLocomotionConfig::FindLandingAction(ERLocomotionState FromState, float ImpactSpeed, float GroundSpeed) const
{
	for (const FRLandingAction& Action : LandingActions)
	{
		if (Action.FromStates.Num() > 0 && !Action.FromStates.Contains(FromState))
		{
			continue;
		}

		if (ImpactSpeed < Action.MinImpactSpeed || ImpactSpeed > Action.MaxImpactSpeed)
		{
			continue;
		}

		if (GroundSpeed < Action.MinGroundSpeed)
		{
			continue;
		}

		return &Action;
	}

	return nullptr;
}
