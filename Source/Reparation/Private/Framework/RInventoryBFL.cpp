// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RInventoryBFL.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Components/RInventoryComponent.h"
#include "Items/RItem.h"

class URInventoryComponent* URInventoryBFL::GetInventoryComponentFromTarget(AActor* Target)
{
	if (!Target)
	{
		return nullptr;
	}

	if (URInventoryComponent* InventoryComp = Target->FindComponentByClass<URInventoryComponent>())
	{
		return InventoryComp;
	}

	//Try player state, then pawn, then controller 
	if (APawn* OwningPawn = Cast<APawn>(Target))
	{
		if (OwningPawn->GetPlayerState())
		{
			return OwningPawn->GetPlayerState()->FindComponentByClass<URInventoryComponent>();
		}

		if (OwningPawn->GetController())
		{
			return OwningPawn->GetController()->FindComponentByClass<URInventoryComponent>();
		}
	}

	//Same for controller based target 
	if (APlayerController* OwningController = Cast<APlayerController>(Target))
	{
		if (OwningController->GetPawn())
		{
			if (OwningController->GetPlayerState<APlayerState>())
			{
				return OwningController->GetPlayerState<APlayerState>()->FindComponentByClass<URInventoryComponent>();
			}


			return OwningController->GetPawn()->FindComponentByClass<URInventoryComponent>();
		}
	}

	return nullptr;
}

TArray<class URItem*> URInventoryBFL::SortItemArrayAlphabetical(TArray<class URItem*> InItems, const bool bReverse)
{
	TArray<class URItem*> RetItems = InItems;

	//Sort the replies by their Y position in the graph
	if (bReverse)
	{
		RetItems.Sort([](const URItem& ItemA, const URItem& ItemB) {
			return ItemA.DisplayName.ToString() < ItemB.DisplayName.ToString();
			});
	}
	else
	{
		RetItems.Sort([](const URItem& ItemA, const URItem& ItemB) {
			return ItemA.DisplayName.ToString() > ItemB.DisplayName.ToString();
			});
	}

	return RetItems;
}

TArray<class URItem*> URInventoryBFL::SortItemArrayWeight(TArray<class URItem*> InItems, const bool bReverse)
{
	TArray<class URItem*> RetItems = InItems;

	//Sort the replies by their Y position in the graph
	if (bReverse)
	{
		RetItems.Sort([](const URItem& ItemA, const URItem& ItemB) {
			return ItemA.GetStackWeight() > ItemB.GetStackWeight();
			});
	}
	else
	{
		RetItems.Sort([](const URItem& ItemA, const URItem& ItemB) {
			return ItemA.GetStackWeight() < ItemB.GetStackWeight();
			});
	}


	return RetItems;
}
