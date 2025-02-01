// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/RAnimInstanceBase.h"

#include "Characters/RCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"


void URAnimInstanceBase::NativeInitializeAnimation()
{
	OwningCharacter = Cast<ARCharacterBase>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
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
}
