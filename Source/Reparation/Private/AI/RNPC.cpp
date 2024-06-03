// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RNPC.h"

#include "AbilitySystem/RAbilitySystemComponent.h"
#include "AbilitySystem/RAttributeSet.h"

ARNPC::ARNPC()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<URAttributeSet>("AttributeSet");

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Combat");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARNPC::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}
