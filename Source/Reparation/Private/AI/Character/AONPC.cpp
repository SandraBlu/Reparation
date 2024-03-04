// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Character/AONPC.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"

AAONPC::AAONPC()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAOAbilitySystemComponent>("AbilitySystemComponent");
	AttributeSet = CreateDefaultSubobject<UAOAttributeSet>("AttributeSet");
}

void AAONPC::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}
