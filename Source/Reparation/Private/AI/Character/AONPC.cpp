// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Character/AONPC.h"

void AAONPC::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
		ASC->InitAbilityActorInfo(this, this);
}
