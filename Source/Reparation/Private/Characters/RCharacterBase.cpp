// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RCharacterBase.h"

// Sets default values
ARCharacterBase::ARCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

