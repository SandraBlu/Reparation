// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/RAnimPlayer.h"

#include "Characters/RPlayer.h"

void URAnimPlayer::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (OwningCharacter)
	{
		OwningPlayerCharacter = Cast<ARPlayer>(OwningCharacter);
	}
}

void URAnimPlayer::NativeUpdateAnimation(float DeltaSeconds)
{
	// Deliberately not the thread safe pass. This depends on the Locomotion
	// snapshot that NativeUpdateAnimation copies, and a linked anim graph runs as
	// its own instance, so relying on both passes firing there left the timer
	// counting against a snapshot that never updated.
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Relax only while genuinely standing still on the ground. Acceleration alone
	// would also read as idle mid-fall or while pinned against a wall.
	if (Locomotion.State != ERLocomotionState::Idle)
	{
		IdleElapsedTime = 0.f;
		bEnterRelaxState = false;
	}
	else
	{
		IdleElapsedTime += DeltaSeconds;
		bEnterRelaxState = (IdleElapsedTime >= EnterRelaxStateThreshold);
	}
}
