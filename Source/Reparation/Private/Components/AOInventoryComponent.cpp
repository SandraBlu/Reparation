// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AOInventoryComponent.h"
#include "Item/AOItem.h"

#define LOCTEXT_NAMESPACE "Inventory"

UAOInventoryComponent::UAOInventoryComponent()
{
	Capacity = 20;
}

FItemAddResult UAOInventoryComponent::TryAddItem(class UAOItem* Item)
{
	return TryAddItem_Internal(Item);
}

FItemAddResult UAOInventoryComponent::TryAddItemFromClass(TSubclassOf<class UAOItem> ItemClass, const int32 Quantity)
{
	UAOItem* Item = NewObject<UAOItem>(GetOwner(), ItemClass);
	Item->SetQuantity(Quantity);
	return TryAddItem_Internal(Item);
}

int32 UAOInventoryComponent::ConsumeItem(class UAOItem* Item)
{
	if (Item)
	{
		ConsumeItem(Item, Item->GetQuantity());
	}
	return 0;
}

int32 UAOInventoryComponent::ConsumeItem(class UAOItem* Item, const int32 Quantity)
{
	if (GetOwner() && Item)
	{
		const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());
		//prevent having less than zero items after drop or consume
		ensure(!(Item->GetQuantity() - RemoveQuantity < 0));

		//We now have Zero of item, remove it from inventory
		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

		if (Item->GetQuantity() <= 0)
		{
			RemoveItem(Item);
			OnInventoryUpdated.Broadcast();
		}
		else
		{
			RefreshInventory();
		}
		return RemoveQuantity;
	}
	return 0;
}

bool UAOInventoryComponent::RemoveItem(class UAOItem* Item)
{
	if (GetOwner())
	{
		if (Item)
		{
			Items.RemoveSingle(Item);
			return true;
		}
	}
	return false;
}

bool UAOInventoryComponent::HasItem(TSubclassOf<class UAOItem> ItemClass, const int32 Quantity /*= 1*/) const
{
	if (UAOItem* ItemToFind = FindItemByClass(ItemClass))
	{
		return ItemToFind->GetQuantity() >= Quantity;
	}
	return false;
}

UAOItem* UAOInventoryComponent::FindItem(class UAOItem* Item) const
{
	if (Item)
	{
		for (auto& InvItem : Items)
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass())
			{
				return InvItem;
			}
		}
	}
	return nullptr;
}

UAOItem* UAOInventoryComponent::FindItemByClass(TSubclassOf<class UAOItem> ItemClass) const
{
	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{
			return InvItem;
		}
	}
	return nullptr;
}

TArray<UAOItem*> UAOInventoryComponent::FindItemsByClass(TSubclassOf<class UAOItem> ItemClass) const
{
	TArray<UAOItem*> ItemsOfClass;

	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass()->IsChildOf(ItemClass))
		{
			ItemsOfClass.Add(InvItem);
		}
	}
	return ItemsOfClass;
}

float UAOInventoryComponent::GetCurrentWeight() const
{
	float Weight = 0.f;

	for (auto& Item : Items)
	{
		if (Item)
		{
			Weight += Item->GetStackWeight();
		}
	}
	return Weight;
}

void UAOInventoryComponent::SetWeightCapacity(const float NewWeightCapacity)
{
	WeightCapacity = NewWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void UAOInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryUpdated.Broadcast();
}

void UAOInventoryComponent::RefreshInventory()
{
	OnInventoryUpdated.Broadcast();

	for (auto& Item : Items)
	{
		if (Item && !Item->World)
		{
			Item->World = GetWorld();
		}
	}
}

UAOItem* UAOInventoryComponent::AddItem(class UAOItem* Item)
{
	if (GetOwner())
	{
		UAOItem* NewItem = NewObject<UAOItem>(GetOwner(), Item->GetClass());
		NewItem->World = GetWorld();
		NewItem->SetQuantity(Item->GetQuantity());
		NewItem->OwningInventory = this;
		NewItem->AddedToInventory(this);
		Items.Add(NewItem);

		return NewItem;
	}
	return nullptr;
}

FItemAddResult UAOInventoryComponent::TryAddItem_Internal(class UAOItem* Item)
{
	if (GetOwner())
	{
		const int32 AddAmount = Item->GetQuantity();

		if (Items.Num() + 1 > GetCapacity())
		{
			return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryCapacityFullText", "Item not added. Inventory is FULL"));
		}

		//Items weighing zero skip weight check.
		if (!FMath::IsNearlyZero(Item->Weight))
		{
			if (GetCurrentWeight() + Item->Weight > GetWeightCapacity())
			{
				return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryTooMuchWeightText", "Item not added. OVER WEIGHT capacity"));
			}
		}
		//Is Stackable?
		if (Item->bStackable)
		{
			//check weight never goes over capacity
			ensure(Item->GetQuantity() <= Item->MaxStackSize);
			//check if item already exists, if it does, modify quantity instead of adding
			if (UAOItem* ExistingItem = FindItem(Item))
			{
				if (ExistingItem->GetQuantity() < ExistingItem->MaxStackSize)
				{
					//calculate how much of item to add based on max stack size
					const int32 CapacityMaxAddAmount = ExistingItem->MaxStackSize - ExistingItem->GetQuantity();
					//find out which is smaller amount and use that number
					int32 ActualAddAmount = FMath::Min(AddAmount, CapacityMaxAddAmount);

					FText ErrorText = LOCTEXT("InventoryErrorText", "Could not add all items. Reached MAX AMOUNT.");

					//Adjust based on weight capacity
					if (!FMath::IsNearlyZero(Item->Weight))
					{
						const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight);
						ActualAddAmount = FMath::Min(ActualAddAmount, WeightMaxAddAmount);

						if (ActualAddAmount < AddAmount)
						{
							ErrorText = FText::Format(LOCTEXT("InventoryTooMuchWeightText", "Could not add entire stack of {ItemName}. Too many in inventory."), Item->ItemDisplayName);
						}
					}
					else if (ActualAddAmount < AddAmount)
					{
						ErrorText = FText::Format(LOCTEXT("InventoryCapacityFullText", "Could not add enire stack of {ItemName}, as Inventory is FULL."), Item->ItemDisplayName);
					}
					//Could not add any of item
					if (ActualAddAmount <= 0)
					{
						return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryErrorText", "Could not add item to Inventory."));
					}
					ExistingItem->SetQuantity(ExistingItem->GetQuantity() + ActualAddAmount);

					ensure(ExistingItem->GetQuantity() <= ExistingItem->MaxStackSize);

					if (ActualAddAmount < AddAmount)
					{
						return FItemAddResult::AddedSome(AddAmount, ActualAddAmount, ErrorText);
					}
					else
					{

						return FItemAddResult::AddedAll(AddAmount);
					}
				}
				else
				{
					return FItemAddResult::AddedNone(AddAmount, FText::Format(LOCTEXT("InventoryFullStackText", "Could not add {ItemName}. Stack is full."), Item->ItemDisplayName));
				}
			}
			//new Item, so we add full stack
			else
			{
				AddItem(Item);
				return FItemAddResult::AddedAll(AddAmount);
			}
		}
		else  //item not stackable
		{
			ensure(Item->GetQuantity() == 1);

			AddItem(Item);
			return FItemAddResult::AddedAll(AddAmount);
		}
	}
	check(false);
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));
}

#undef  LOCTEXT_NAMESPACE

