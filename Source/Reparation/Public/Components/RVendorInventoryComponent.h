// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RInventoryComponent.h"
#include "RVendorInventoryComponent.generated.h"

/**
 * A special type of inventory you should use for vendors. Vendors are essentially just lootable containers, however looting/storing items
 * will consume/remove currency from the player and vendors when the looting occurs. The UI will also display prices. 
 */
UCLASS(ClassGroup=(Inventory), DisplayName = "Vendor Inventory", meta=(BlueprintSpawnableComponent))
class REPARATION_API URVendorInventoryComponent : public URInventoryComponent
{
	GENERATED_BODY()

protected:

	URVendorInventoryComponent(const FObjectInitializer& ObjectInitializer);

	virtual bool AllowLootItem(class URInventoryComponent* Taker, TSubclassOf <class URItem> ItemClass, const int32 Quantity, FText& ErrorText) const override;
	virtual bool AllowStoreItem(class URInventoryComponent* Storer, TSubclassOf <class URItem> ItemClass, const int32 Quantity, FText& ErrorText) const override;

	/** Remove the item from us, and give it to the taker */
	virtual FItemAddResult PerformLootItem(class URInventoryComponent* Taker, TSubclassOf <class URItem> ItemClass, const int32 Quantity = 1) override;

	/** Remove the item from the storer, and give it to us */
	virtual FItemAddResult PerformStoreItem(class URInventoryComponent* Storer, TSubclassOf <class URItem> ItemClass, const int32 Quantity = 1) override;


	/**
	* Return the price we'll buy the given item for
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Item")
	int32 GetBuyPrice(TSubclassOf<class URItem> Item, int32 Quantity = 1) const;
	virtual int32 GetBuyPrice_Implementation(TSubclassOf<class URItem> Item, int32 Quantity = 1) const;

	/**
	* Return the price we'll sell the given item for
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Item")
	int32 GetSellPrice(TSubclassOf<class URItem> Item, int32 Quantity = 1) const;
	virtual int32 GetSellPrice_Implementation(TSubclassOf<class URItem> Item, int32 Quantity = 1) const;

	//The percentage of the items value we'll buy items for
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float BuyItemPct;

	//The percentage of the items value we'll sell items for 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float SellItemPct;
	
};
