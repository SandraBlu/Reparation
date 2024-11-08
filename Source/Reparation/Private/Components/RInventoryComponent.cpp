// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RInventoryComponent.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "Items/RItem.h"

#define LOCTEXT_NAMESPACE "Inventory"

URInventoryComponent::URInventoryComponent(const FObjectInitializer& ObjectInitializer)
{
    SetIsReplicatedByDefault(true);

    WeightCapacity = 100.f;
    Capacity = 50;
}

bool URInventoryComponent::UseItem(URItem* Item)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerUseItem(Item);
	}

	if (GetOwnerRole() >= ROLE_Authority)
	{
		// Server should validate the inventory actually contains the item the player is attempting to use 
		if (!IsValid(FindItemByClass(Item->GetClass())))
		{
			return false;
		}
	}

	if (Item && Item->CanUse())
	{
		const float UseTime = GetWorld()->GetTimeSeconds();
		if (UseTime - Item->GetLastUseTime() > Item->UseRechargeDuration)
		{
			//If the item is consumable like food, but it can't be removed from the inventory then disallow using the item 
			if (!Item->CanBeRemoved() && Item->bConsumeOnUse)
			{
				return false;
			}
			Item->OnUse();
			Item->Use();
			OnItemUsed.Broadcast(Item);

			if (GetOwnerRole() < ROLE_Authority)
			{
				return true;
			}
			Item->SetLastUseTime(UseTime);
			if (Item->bConsumeOnUse)
			{
				ConsumeItem(Item, 1);
			}
			if (Item->bToggleActiveOnUse)
			{
				Item->SetActive(!Item->bActive);
			}
			return true;
		}
	}
	return false;
}

FItemAddResult URInventoryComponent::TryAddItemFromClass(TSubclassOf<class URItem> ItemClass, const int32 Quantity)
{
	if (ItemClass && Quantity > 0)
	{
		FItemAddResult AddResult = TryAddItem_Internal(ItemClass, Quantity);

		if (AddResult.AmountGiven > 0)
		{
			OnItemAdded.Broadcast(AddResult);
		}

		return AddResult;
	}
	return FItemAddResult::AddedNone(Quantity, FText::FromString("Failed to add item to inventory."));
}

int32 URInventoryComponent::ConsumeItem(class URItem* Item, const int32 Quantity)
{
	if (Item && Item->CanBeRemoved() && Quantity > 0)
	{
		if (GetOwnerRole() < ROLE_Authority)
		{
			ServerConsumeItem(Item, Quantity);
			return 0;
		}

		if (GetOwner() && GetOwner()->HasAuthority())
		{
			const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());

			//We shouldn't have a negative amount of the item after the drop
			ensure(!(Item->GetQuantity() - RemoveQuantity < 0));

			//We now have zero of this item, remove it from the inventory
			Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

			if (Item->GetQuantity() <= 0)
			{
				RemoveItem(Item);
			}

			OnItemRemoved.Broadcast(Item, RemoveQuantity);

			return RemoveQuantity;
		}
	}

	return 0;
}

bool URInventoryComponent::RemoveItem(class URItem* Item)
{
	if (Item && Item->CanBeRemoved())
	{
		if (GetOwnerRole() < ROLE_Authority)
		{
			ServerRemoveItem(Item);
			return false;
		}

		if (GetOwner() && GetOwner()->HasAuthority())
		{
			if (Item)
			{
				Item->RemovedFromInventory(this);

				Items.RemoveSingle(Item);

				//Consume item will have called this delegate, if we've called RemoveItem we need to call it 
				if (Item->GetQuantity() != 0)
				{
					OnItemRemoved.Broadcast(Item, Item->GetQuantity());
				}

				OnRep_Items(Items);

				ReplicatedItemsKey++;

				return true;
			}
		}

	}

	return false;
}

bool URInventoryComponent::HasItem(TSubclassOf<URItem> ItemClass, const int32 Quantity,
	const bool bCheckVisibility) const
{
	if (IsValid(ItemClass))
	{
		if (URItem* ItemCDO = ItemClass->GetDefaultObject<URItem>())
		{
			if (ItemCDO->bStackable)
			{
				//if the item is stackable we can just check the stack quantity
				if (URItem* ItemToFind = FindItemByClass(ItemClass))
				{
					if (!bCheckVisibility || ItemToFind->ShouldShowInInventory())
					{
						return ItemToFind->GetQuantity() >= Quantity;
					}
				}
			}//For non stackable items get all of them and check quantity	
			else return FindItemsByClass(ItemClass, bCheckVisibility).Num() >= Quantity;
		}
	}

	return false;
}

URItem* URInventoryComponent::FindItemByClass(TSubclassOf<URItem> ItemClass, const bool bCheckVisibility) const
{
	for (auto& InvItem : Items)
	{
		//Find item by class checks for an exact class match instead of child of
		if (InvItem && (!bCheckVisibility || InvItem->ShouldShowInInventory()) && InvItem->GetClass() == ItemClass)
		{
			return InvItem;
		}
	}
	return nullptr;
}

TArray<URItem*> URInventoryComponent::FindItemsByClass(TSubclassOf<URItem> ItemClass,
	const bool bCheckVisibility) const
{
	TArray<URItem*> ItemsOfClass;

	for (auto& InvItem : Items)
	{
		if (InvItem && (!bCheckVisibility || InvItem->ShouldShowInInventory()) && InvItem->GetClass()->IsChildOf(ItemClass))
		{
			ItemsOfClass.Add(InvItem);
		}
	}

	return ItemsOfClass;
}

int32 URInventoryComponent::GetSpaceForItem(TSubclassOf<URItem> ItemClass, FText& NoSpaceReason) const
{
	if (!IsValid(ItemClass))
	{
		return 0;
	}

	//Figure out how many of a given item we can add 
	if (const URItem* ItemCDO = GetDefault<URItem>(ItemClass))
	{
		const int32 MaxStackSize = ItemCDO->bStackable ? ItemCDO->MaxStackSize : 1;
		int32 ExistingStackSize = 0;

		//Check if we already have the item or not
		if (URItem* ExistingItem = FindItemByClass(ItemClass))
		{
			ExistingStackSize = ExistingItem->GetQuantity();
		}

		const int32 WeightSpace = FMath::IsNearlyZero(ItemCDO->Weight) ? INT_MAX : FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / ItemCDO->Weight);
		const int32 CapacitySpace = ((Capacity - Items.Num()) * MaxStackSize) + (MaxStackSize - ExistingStackSize);

		if (WeightSpace < CapacitySpace)
		{
			if (WeightSpace <= 0)
			{
				NoSpaceReason = LOCTEXT("NoSpaceReason_WeightFull", "You're carrying too much weight.");
			}

			return WeightSpace;
		}

		if (CapacitySpace <= 0)
		{
			NoSpaceReason = LOCTEXT("NoSpaceReason_CapacitySpace", "You don't have any inventory slots left for this item.");
		}

		return CapacitySpace;
	}

	return 0;
}

bool URInventoryComponent::AllowLootItem(URInventoryComponent* Taker, TSubclassOf<URItem> ItemClass,
	const int32 Quantity, FText& ErrorText) const
{
	if (Taker)
	{
		const bool bTakerHasSpace = Taker->GetSpaceForItem(ItemClass, ErrorText) >= Quantity;
		const bool bShopHasItem = HasItem(ItemClass, Quantity);
		if (!bTakerHasSpace)
		{
			ErrorText = LOCTEXT("AllowLootItem_NotEnoughSpace", "You don't have enough space to carry this item. ");
		}
		else if (!bShopHasItem)
		{
			ErrorText = LOCTEXT("AllowLootItem_ShopHasItem", "There isn't enough of the item to take! ");
		}
		return bTakerHasSpace && bShopHasItem;
	}
	return false;
}

bool URInventoryComponent::AllowStoreItem(URInventoryComponent* Storer, TSubclassOf<URItem> ItemClass,
	const int32 Quantity, FText& ErrorText) const
{
	if (Storer)
	{
		const bool bWeHaveSpace = GetSpaceForItem(ItemClass, ErrorText) >= Quantity;
		const bool bStorerHasItem = Storer->HasItem(ItemClass, Quantity);
		if (!bWeHaveSpace)
		{
			ErrorText = LOCTEXT("AllowStoreItem_WeHaveSpace", "You don't have enough space to carry this item. ");
		}
		else if (!bStorerHasItem)
		{
			//ErrorText = LOCTEXT("AllowLootItem_SellerHasItem", "You no longer have the item in your inventory.");
		}
		return bWeHaveSpace && bStorerHasItem;
	}
	return false;
}

FItemAddResult URInventoryComponent::PerformLootItem(URInventoryComponent* Taker, TSubclassOf<URItem> ItemClass,
	const int32 Quantity)
{
	if (Taker)
	{
		//Give the item to the looter
		const FItemAddResult AddResult = Taker->TryAddItemFromClass(ItemClass, Quantity);
		//However many we gave, take them from our inventory 
		if (AddResult.AmountGiven > 0)
		{
			if (URItem* Item = FindItemByClass(ItemClass))
			{
				//However much of the item we looted, remove from the loot source
				ConsumeItem(Item, AddResult.AmountGiven);
			}
		}
		return AddResult;
	}
	return FItemAddResult::AddedNone(0, FText::GetEmpty());
}

FItemAddResult URInventoryComponent::PerformStoreItem(URInventoryComponent* Storer, TSubclassOf<URItem> ItemClass,
	const int32 Quantity)
{
	if (Storer)
	{
		//Store the item in our inventory
		const FItemAddResult AddResult = TryAddItemFromClass(ItemClass);
		if (AddResult.AmountGiven > 0)
		{
			if (URItem* Item = Storer->FindItemByClass(ItemClass))
			{
				//However much of the item we managed to store, remove from the storers inventory
				Storer->ConsumeItem(Item, AddResult.AmountGiven);
			}
		}
		return AddResult;
	}
	return FItemAddResult::AddedNone(0, FText::GetEmpty());
}

int32 URInventoryComponent::GetCurrency() const
{
	return Currency;
}

void URInventoryComponent::AddCurrency(const int32 Amount)
{
}

void URInventoryComponent::SetCurrency(const int32 Amount)
{
}

float URInventoryComponent::GetCurrentWeight() const
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

void URInventoryComponent::SetWeightCapacity(const float NewWeightCapacity)
{
	WeightCapacity = NewWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void URInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryUpdated.Broadcast();
}

APawn* URInventoryComponent::GetOwningPawn() const
{
	if (OwnerPC)
	{
		return OwnerPC->GetPawn();
	}

	if (APlayerState* OwningPS = Cast<APlayerState>(GetOwner()))
	{
		return OwningPS->GetPawn();
	}

	if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
	{
		return OwningPawn;
	}

	if (APlayerController* OwningController = Cast<APlayerController>(GetOwner()))
	{
		return OwningController->GetPawn();
	}

	return nullptr;
}

APlayerController* URInventoryComponent::GetOwningController() const
{
	//We cache this on beginplay as to not re-find it every time 
	if (OwnerPC)
	{
		return OwnerPC;
	}

	APlayerController* OwningController = Cast<APlayerController>(GetOwner());
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	APlayerState* OwningPS = Cast<APlayerState>(GetOwner());

	if (OwningController)
	{
		return OwningController;
	}

	if (!OwningController && OwningPawn)
	{
		return Cast<APlayerController>(OwningPawn->GetController());
	}

	if (OwningPS)
	{
		return OwningPS->GetPlayerController();
	}

	return nullptr;
}

void URInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}

void URInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPC = GetOwningController();

	for (auto& Item : DefaultItems)
	{
		if (Item)
		{
			TryAddItemFromClass(Item->GetClass(), Item->GetQuantity());
		}
	}
	DefaultItems.Empty();
}

void URInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(URInventoryComponent, Items);
	DOREPLIFETIME(URInventoryComponent, Currency);

	//If your game requires players to know if other players are looting you can remove 
	//DOREPLIFETIME_CONDITION(UAOInventoryComponent, LootSource, COND_OwnerOnly);
}

bool URInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	//Check if the array of items needs to replicate
	if (Channel->KeyNeedsToReplicate(0, ReplicatedItemsKey))
	{
		for (auto& Item : Items)
		{
			if (Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
			{
				bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}
	return bWroteSomething;
}

void URInventoryComponent::ServerUseItem_Implementation(URItem* Item)
{
	UseItem(Item);
}

void URInventoryComponent::ServerConsumeItem_Implementation(URItem* Item, const int32 Quantity)
{
	if (Item)
	{
		ConsumeItem(Item, Quantity);
	}
}

void URInventoryComponent::ServerRemoveItem_Implementation(URItem* Item)
{
	if (Item)
	{
		RemoveItem(Item);
	}
}

void URInventoryComponent::SetLootSource(URInventoryComponent* NewLootSource)
{
	if (GetOwnerRole() >= ROLE_Authority)
	{
		if (LootSource != NewLootSource)
		{
			URInventoryComponent* OldSource = LootSource;
			LootSource = NewLootSource;
			OnRep_LootSource(OldSource);
		}
	}
}

URItem* URInventoryComponent::AddItem(TSubclassOf<URItem> ItemClass, const int32 Quantity)
{
	if (Quantity > 0 && GetOwner() && GetOwner()->HasAuthority())
	{
		URItem* NewItem = NewObject<URItem>(GetOwner(), ItemClass);
		NewItem->World = GetWorld();
		NewItem->OwningInventory = this;
		NewItem->SetQuantity(Quantity);
		Items.Add(NewItem);

		NewItem->AddedToInventory(this);
		NewItem->MarkDirtyForReplication();
		OnRep_Items(Items);

		return NewItem;
	}
	return nullptr;
}

FItemAddResult URInventoryComponent::TryAddItem_Internal(TSubclassOf<URItem> ItemClass, const int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority() && IsValid(ItemClass))
	{
		if (const URItem* ItemCDO = GetDefault<URItem>(ItemClass))
		{
			FText ErrorText;
			const int32 NeedAddAmount = FMath::Min(Quantity, GetSpaceForItem(ItemClass, ErrorText));
			int32 LeftToAdd = NeedAddAmount;

			FItemAddResult AddResult;
			AddResult.ItemClass = ItemClass;
			AddResult.ErrorText = ErrorText;
			AddResult.AmountToGive = Quantity;
			AddResult.AmountGiven = NeedAddAmount; // GetSpaceForItem will ensure we add this amount 

			if (LeftToAdd <= 0)
			{
				return AddResult;
			}

			//Top up any existing stacks with the add amount, stopping if we ran out of items to give 
			for (auto& Stack : FindItemsByClass(ItemClass))
			{
				const int32 StackSpace = Stack->GetStackSpace();

				if (StackSpace > 0)
				{
					const int32 OldQuantity = Stack->GetQuantity();
					Stack->SetQuantity(Stack->GetQuantity() + LeftToAdd);

					//subtract the amount we added
					const int32 AmountGiven = Stack->GetQuantity() - OldQuantity;
					//UE_LOG(LogTemp, Warning, TEXT("Adding %s adding %d to existing stack"), *ItemCDO->DisplayName.ToString(), AmountGiven);
					LeftToAdd -= AmountGiven;
				}

				//Paranoia check - if LeftToAdd went negative we probably gave more items than we were supposed to 
				check(LeftToAdd >= 0);

				if (LeftToAdd <= 0)
				{
					return AddResult;
				}
			}

			//UE_LOG(LogTemp, Warning, TEXT("Initially needed %d, now we still have %d left to add. "), *ItemCDO->DisplayName.ToString(), NeedAddAmount, LeftToAdd);

			//We essentially figure out how many full stacks we need to add, and how many leftover we have 
			const int32 MaxStackSize = ItemCDO->GetMaxStackSize();
			const int32 FullStacksToAdd = FMath::FloorToInt((double)LeftToAdd / (double)MaxStackSize);
			int32 Remainder = LeftToAdd % MaxStackSize;

			//FString RoleString = HasAuthority() ? "Server" : "Client";
			//UE_LOG(LogTemp, Warning, TEXT("Adding %s, Max Size: %d, StacksToAdd: %d, Remainder: %d, Add amount: %d"), *ItemCDO->DisplayName.ToString(), MaxStackSize, FullStacksToAdd, Remainder, LeftToAdd);

			//Create any new required stacks 
			for (int32 i = 0; i < FullStacksToAdd; ++i)
			{
				if (URItem* NewItem = AddItem(ItemClass, MaxStackSize))
				{
					AddResult.CreatedStacks.Add(NewItem);
				}
			}

			//Add the remainder first
			if (Remainder > 0)
			{
				if (URItem* NewItem = AddItem(ItemClass, Remainder))
				{
					AddResult.CreatedStacks.Add(NewItem);
				}
			}

			return AddResult;
		}
	}
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));
}

void URInventoryComponent::StopLooting()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerStopLooting();
	}

	SetLootSource(nullptr);
}

bool URInventoryComponent::RequestLootItem(URItem* ItemToLoot, FText& ErrorText, const int32 Quantity)
{
	if (LootSource && ItemToLoot && ItemToLoot->CanBeRemoved())
	{
		if (LootSource && LootSource->AllowLootItem(this, ItemToLoot->GetClass(), Quantity, ErrorText))
		{
			if (GetOwnerRole() < ROLE_Authority)
			{
				ServerRequestLootItem(ItemToLoot, Quantity);
				return true;
			}

			LootSource->PerformLootItem(this, ItemToLoot->GetClass(), Quantity);
			return true;
		}

	}
	return false;
}

bool URInventoryComponent::RequestStoreItem(URItem* ItemToStore, FText& ErrorText, const int32 Quantity)
{
	if (ItemToStore && ItemToStore->CanBeRemoved())
	{
		if (LootSource && LootSource->AllowStoreItem(this, ItemToStore->GetClass(), Quantity, ErrorText))
		{
			if (GetOwnerRole() < ROLE_Authority)
			{
				ServerRequestStoreItem(ItemToStore, Quantity);
				return true;
			}

			LootSource->PerformStoreItem(this, ItemToStore->GetClass(), Quantity);
			return true;
		}
	}
	return false;
}

void URInventoryComponent::ServerRequestStoreItem_Implementation(URItem* ItemToLoot, const int32 Quantity)
{
	FText DummyText;
	RequestStoreItem(ItemToLoot, DummyText, Quantity);
}

void URInventoryComponent::ServerRequestLootItem_Implementation(URItem* ItemToLoot, const int32 Quantity)
{
	FText DummyText;
	RequestLootItem(ItemToLoot, DummyText, Quantity);
}

void URInventoryComponent::ServerStopLooting_Implementation()
{
	StopLooting();
}

void URInventoryComponent::OnRep_Items(const TArray<URItem*>& OldItems)
{
	OnInventoryUpdated.Broadcast();

	for (auto& Item : Items)
	{
		if (Item)
		{
			//Any items without a world set have been added
			if (!Item->World)
			{
				Item->World = GetWorld();
				Item->OwningInventory = this;
			}
		}
	}
}

void URInventoryComponent::OnRep_Currency(const int32 OldCurrency)
{
	OnCurrencyChanged.Broadcast(OldCurrency, Currency);
}

void URInventoryComponent::OnRep_LootSource(URInventoryComponent* OldLootSource)
{
	if (LootSource)
	{
		OnBeginLooting.Broadcast(LootSource);
	}
	else
	{
		OnEndLooting.Broadcast();
	}
}

#undef LOCTEXT_NAMESPACE