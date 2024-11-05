// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RVendorInventoryComponent.h"

#include "Items/RItem.h"

#define LOCTEXT_NAMESPACE "VendorInventoryComponent"

URVendorInventoryComponent::URVendorInventoryComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer) // This line calls the base class constructor
{
	
	//By default buy items for 50% of their value, and sell for 2x their value 
	BuyItemPct = 0.5f;
	SellItemPct = 2.f;
	Currency = 500;
}

bool URVendorInventoryComponent::AllowLootItem(class URInventoryComponent* Taker, TSubclassOf<class URItem> ItemClass, const int32 Quantity, FText& ErrorText) const
{
	if (Taker)
	{
		const bool bSuperAllows = Super::AllowLootItem(Taker, ItemClass, Quantity, ErrorText);
		const bool bTheyHaveCurrency = Taker->GetCurrency() >= GetSellPrice(ItemClass, Quantity);

		if (!bTheyHaveCurrency)
		{
			ErrorText = LOCTEXT("AllowLootItem_NotEnoughMoney", "You don't have enough money to complete this trade.");
		}

		return bSuperAllows && bTheyHaveCurrency;
	}

	return false;
}

bool URVendorInventoryComponent::AllowStoreItem(class URInventoryComponent* Storer, TSubclassOf<class URItem> ItemClass, const int32 Quantity, FText& ErrorText) const
{
	if (Storer)
	{
		//Ensure we have enough money to buy the item from the seller 
		const bool bSuperAllows = Super::AllowStoreItem(Storer, ItemClass, Quantity, ErrorText);
		const bool bWeHaveCurrency = GetCurrency() >= GetBuyPrice(ItemClass, Quantity);

		if (!bWeHaveCurrency)
		{
			ErrorText = LOCTEXT("AllowStoreItem_NotEnoughMoney", "The shop doesn't have enough money to complete the trade.");
		}

		return bSuperAllows && bWeHaveCurrency;
	}

	return false;
}

FItemAddResult URVendorInventoryComponent::PerformLootItem(class URInventoryComponent* Taker, TSubclassOf<class URItem> ItemClass, const int32 Quantity)
{
	FItemAddResult LootResult = Super::PerformLootItem(Taker, ItemClass, Quantity);

	if (Taker)
	{
		const int32 TransactionPrice = GetSellPrice(ItemClass, LootResult.AmountGiven);

		//We sold an item, need to remove cash from taker and store in our inventory 
		Taker->AddCurrency(-TransactionPrice);
		AddCurrency(TransactionPrice);
	}
	
	return LootResult;
}

FItemAddResult URVendorInventoryComponent::PerformStoreItem(class URInventoryComponent* Storer, TSubclassOf<class URItem> ItemClass, const int32 Quantity)
{
	FItemAddResult StoreResult = Super::PerformStoreItem(Storer, ItemClass, Quantity);
	
	if (Storer)
	{
		const int32 TransactionPrice = GetBuyPrice(ItemClass, StoreResult.AmountGiven);

		//We sold an item, need to remove cash from taker and store in our inventory 
		Storer->AddCurrency(TransactionPrice);
		AddCurrency(-TransactionPrice);
	}

	return StoreResult;
}

int32 URVendorInventoryComponent::GetBuyPrice_Implementation(TSubclassOf<class URItem> Item, int32 Quantity) const
{
	if (const URItem* ItemCDO = GetDefault<URItem>(Item))
	{
		return FMath::CeilToInt((ItemCDO->BaseValue * BuyItemPct) * Quantity);
	}
	return 0;
}

int32 URVendorInventoryComponent::GetSellPrice_Implementation(TSubclassOf<class URItem> Item, int32 Quantity) const
{
	if (const URItem* ItemCDO = GetDefault<URItem>(Item))
	{
		return FMath::CeilToInt((ItemCDO->BaseValue * SellItemPct) * Quantity);
	}
	return INT_MAX;
}

#undef LOCTEXT_NAMESPACE 