// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawn/AOSpawn.h"
#include "Item/AOItem.h"
#include "Item/AOPickup.h"

AAOSpawn::AAOSpawn()
{
	PrimaryActorTick.bCanEverTick = false;
	RespawnDelayRange = FIntPoint(10, 30);
}

void AAOSpawn::BeginPlay()
{
	SpawnItem();
}

void AAOSpawn::SpawnItem()
{
	if (LootTable)
	{
		TArray<FLootTableRow*> SpawnItems;
		LootTable->GetAllRows("", SpawnItems);

		const FLootTableRow* LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];

		ensure(LootRow);

		float ProbabilityRoll = FMath::FRandRange(0.f, 1.f);

		while (ProbabilityRoll > LootRow->Probability)
		{
			LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
			ProbabilityRoll = FMath::FRandRange(0.f, 1.f);
		}
		if (LootRow && LootRow->Items.Num() && PickupClass)
		{
			float Angle = 0.f;

			for (auto& ItemClass : LootRow->Items)
			{
				const FVector LocationOffset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * 50.f;

				FActorSpawnParameters SpawnParams;
				SpawnParams.bNoFail = true;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				const int32 ItemQuantity = ItemClass->GetDefaultObject<UAOItem>()->GetQuantity();

				FTransform SpawnTransform = GetActorTransform();
				SpawnTransform.AddToTranslation(LocationOffset);

				AAOPickup* Pickup = GetWorld()->SpawnActor<AAOPickup>(PickupClass, SpawnTransform, SpawnParams);
				Pickup->InitializePickup(ItemClass, ItemQuantity);
				Pickup->OnDestroyed.AddUniqueDynamic(this, &AAOSpawn::OnItemTaken);

				SpawnedPickups.Add(Pickup);

				Angle += (PI * 2.f) / LootRow->Items.Num();

			}
		}
	}
}

void AAOSpawn::OnItemTaken(AActor* DestroyedActor)
{
	SpawnedPickups.Remove(DestroyedActor);

	if (SpawnedPickups.Num() <= 0)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnItem, this, &AAOSpawn::SpawnItem, FMath::RandRange(RespawnDelayRange.GetMin(), RespawnDelayRange.GetMax()), false);
	}
}
