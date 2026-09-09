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

void URAnimPlayer::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (bHasAcceleration)
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
