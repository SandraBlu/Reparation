// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RNPC.h"

#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"

ARNPC::ARNPC()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<URAttributeSet>("AttributeSet");

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}
