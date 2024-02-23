// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AOItem.h"
#include "Components/AOInventoryComponent.h"

#define LOCTEXT_NAMESPACE "Item"

UAOItem::UAOItem()
{
	ItemDisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
	Weight = 0.f;
	bStackable = true;
	Quantity = 1;
	MaxStackSize = 2;
}

#if WITH_EDITOR

class UWorld* UAOItem::GetWorld() const
{
	return World;
}

void UAOItem::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UAOItem, Quantity))
	{
		Quantity = FMath::Clamp(Quantity, 1, bStackable ? MaxStackSize : 1);
	}
}
#endif

void UAOItem::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, bStackable ? MaxStackSize : 1);
		OnItemModified.Broadcast();
	}
}

bool UAOItem::ShouldShowInInventory() const
{
	return true;
}

void UAOItem::Use(class AAOPlayerBase* Character)
{
	UE_LOG(LogTemp, Warning, TEXT("Consumed Item"));
}

void UAOItem::AddedToInventory(class UAOInventoryComponent* Inventory)
{

}


#undef  LOCTEXT_NAMESPACE