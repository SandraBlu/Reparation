// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AOPickup.h"

// Sets default values
AAOPickup::AAOPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAOPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAOPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

