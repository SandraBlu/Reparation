// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RLootable.h"

#include "Components/RInventoryComponent.h"

// Sets default values
ARLootable::ARLootable()
{
	LootContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>("LootContainerMesh");
	SetRootComponent(LootContainerMesh);

	Inventory = CreateDefaultSubobject<URInventoryComponent>("Inventory");
	Inventory->SetCapacity(20);
	Inventory->SetWeightCapacity(80.f);

	LootRolls = FIntPoint(2, 8);
}

// Called when the game starts or when spawned
void ARLootable::BeginPlay()
{
	Super::BeginPlay();
	
}

