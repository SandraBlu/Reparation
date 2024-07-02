// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/TargetCircle.h"

#include "Components/DecalComponent.h"


ATargetCircle::ATargetCircle()
{

	PrimaryActorTick.bCanEverTick = true;
	TargetDecal = CreateDefaultSubobject<UDecalComponent>("TargetDecal");
	TargetDecal->SetupAttachment(GetRootComponent());
}

void ATargetCircle::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATargetCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

