// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RInventoryComponent.generated.h"

//Represents result of Adding item to Inventory
USTRUCT(BlueprintType)
struct FItemAddResult
{
	GENERATED_BODY()

public:

	FItemAddResult() {};
	FItemAddResult(int32 InItemQuantity) : AmountToGive(InItemQuantity), AmountGiven(0) {};
	FItemAddResult(int32 InItemQuantity, int32 InQuantityAdded) : AmountToGive(InItemQuantity), AmountGiven(InQuantityAdded) {};

	//The item class we were attempting to add 
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	TSubclassOf<class URItem> ItemClass = TSubclassOf<class URItem>();

	//Any new stacks that were created
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	TArray<class URItem*> Stacks = {};

	//Amount of item try to add
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountToGive = 0;

	//Amount Actually added
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountGiven = 0;

	//Error message explaining why could not be added; ie too many, too heavy
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ErrorText = FText::GetEmpty();

	//Helper Functions for Results to try add item
	static FItemAddResult AddedNone(const int32 InItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult(InItemQuantity);
		AddedNoneResult.ErrorText = ErrorText;

		return AddedNoneResult;
	}

	static FItemAddResult AddedSome(TArray<class URItem*> InItems, const int32 InItemQuantity, const int32 ActualAmountGiven, const FText& ErrorText)
	{
		FItemAddResult AddedSomeResult(InItemQuantity, ActualAmountGiven);
		AddedSomeResult.Stacks = InItems;
		AddedSomeResult.ErrorText = ErrorText;

		return AddedSomeResult;
	}

	static FItemAddResult AddedAll(TArray<class URItem*> InItems,const int32 InItemQuantity)
	{
		FItemAddResult AddAllResult(InItemQuantity, InItemQuantity);
		AddAllResult.Stacks = InItems;
		
		return AddAllResult;
	}
};

//Called when Inventory is changed to update UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

/**Called on server when an item is added to this inventory*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, const FItemAddResult&, AddResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, class URItem*, Item, const int32, Amount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginLooting, class URInventoryComponent*, LootSource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndLooting, class URInventoryComponent*, LootSource);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, class URItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyChanged, const int32, OldCurrency, const int32, NewCurrency);

//Define an item along with some amount of the item 
USTRUCT(BlueprintType)
struct FItemWithQuantity
{
	GENERATED_BODY()

public:

	FItemWithQuantity()
	{
		Item = nullptr;
		Quantity = 1;
	};

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Item")
	TSoftClassPtr<class URItem> Item;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Item")
	int32 Quantity;
};

/**
 * Data asset with a collection of items. Useful for grouping things together like armor sets, weapon sets, etc. 
 */
UCLASS()
class REPARATION_API UItemCollection : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UItemCollection()
	{

	};

	//The items to grant if this row is given to the inventory
	UPROPERTY(EditAnywhere, Category = "Loot Table Row")
	TArray<FItemWithQuantity> Items;

};

//Defines a roll of a loot table 
USTRUCT(BlueprintType)
struct FLootTableRoll
{
	GENERATED_BODY()
public:

	FLootTableRoll()
	{
		NumRolls = 1;
		Chance = 1.f;
	}


	//The items to grant if this row is given to the inventory 
	UPROPERTY(EditAnywhere, Category = "Loot Table Row", meta = (RequiredAssetDataTags = "RowStructure=/Script/Reparation.LootTableRow"))
	TObjectPtr<UDataTable> TableToRoll;

	//The number of times we should roll the table 
	UPROPERTY(EditAnywhere, Category = "Loot Table Row", meta = (ClampMin = 0, ClampMax = 100))
	int32 NumRolls;

	//The chance of each roll succeeding 
	UPROPERTY(EditAnywhere, Category = "Loot Table Row", meta = (ClampMin=0.0, ClampMax=1.0))
	float Chance;

	bool CanRoll() const
	{
		return IsValid(TableToRoll) && NumRolls > 0 && Chance > 0.f;
	}

};

//Defines a row in a loot table
USTRUCT(BlueprintType)
struct FLootTableRow : public FTableRowBase
{
	GENERATED_BODY()
public:

	FLootTableRow()
	{
		Chance = 1.f;
	}

	//The items to grant if this row is given to the inventory
	UPROPERTY(EditAnywhere, Category = "Loot Table Row")
	TArray<FItemWithQuantity> ItemsToGrant;

	//Item collections to grant
	UPROPERTY(EditAnywhere, Category = "Loot Table Row")
	TArray<TObjectPtr<UItemCollection>> ItemCollectionsToGrant;

	//Subloot tables that this loot table will roll 
	UPROPERTY(EditAnywhere, Category = "Loot Table Row")
	TArray<struct FLootTableRoll> SubTablesToRoll;

	//The chance this row will actually be given if selected 
	UPROPERTY(EditAnywhere, Category = "Loot Table Row", meta = (ClampMin=0.0, ClampMax=1.0))
	float Chance;
};

UCLASS( ClassGroup=(Reparation), DisplayName = "Reparation Inventory", meta=(BlueprintSpawnableComponent) )
class REPARATION_API URInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class URItem;

public:	

	URInventoryComponent(const FObjectInitializer& ObjectInitializer);

UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool UseItem(class URItem* Item);
	
	/**Add an item to our inventory using a loot table instead of an item class. Current loads the granted items syncrounously and may cause hitching depending on how heavy the item is. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void TryAddFromLootTable(FLootTableRoll LootTable, TArray<FItemAddResult>& OutItemAddResults);

	/**Add an item to the inventory using the item class
	@param ItemClass the item to add
	@param Quantity the amount of the item to add
	@param bCheckAutoUse if true, we'll check if the item wants to be auto-used on add. Clothing items do this for example to auto-equip. 
	@return the amount of the item that was added to the inventory */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	virtual FItemAddResult TryAddItemFromClass(TSubclassOf<class URItem> ItemClass, const int32 Quantity = 1, const bool bCheckAutoUse=true);

	/** Take some quantity away from the item, and remove it from the inventory when quantity reaches zero.
	Useful for things like eating food, using ammo, etc.*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual int32 ConsumeItem(class URItem* Item, const int32 Quantity = 1);

	/** Remove the item from the inventory. We do actually allow clients to call this unlike AddItem, however the server
	will still validate that the item is allowed to be removed before removing the item, so still cheat safe */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool RemoveItem(class URItem* Item);

	/**Return true if we have a given amount of an item. bCheckVisibility will only look for items that are visible in the inventory*/
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual bool HasItem(TSoftClassPtr<class URItem> ItemClass, const int32 Quantity = 1, const bool bCheckVisibility = false) const;

	/**Return true if the taker can loot the item from this inventory */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual bool AllowLootItem(class URInventoryComponent* Taker, TSubclassOf<class URItem> ItemClass, const int32 Quantity, FText& ErrorText) const;

	/**Return true if the storer can store the given item in this inventory */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual bool AllowStoreItem(class URInventoryComponent* Storer, TSubclassOf<class URItem> ItemClass, const int32 Quantity, FText& ErrorText) const;

	/** Remove the item from us, and give it to the taker. Return how much of the item was able to be looted */
	virtual FItemAddResult PerformLootItem(class URInventoryComponent* Taker, TSubclassOf <class URItem> ItemClass, const int32 Quantity = 1);

	/** Remove the item from the storer, and give it to us. Return how much of the item was able to be stored*/
	virtual FItemAddResult PerformStoreItem(class URInventoryComponent* Storer, TSubclassOf <class URItem> ItemClass, const int32 Quantity = 1);

	/**Return the first item with the same class as ItemClass.*/
	UFUNCTION(BlueprintPure, Category = "Inventory")
	URItem* FindItemByClass(TSoftClassPtr<class URItem> ItemClass, const bool bCheckVisibility = false) const;

	/**Get all inventory items that are children of the supplied ItemClass. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<URItem*> FindItemsOfClass(TSubclassOf<class URItem> ItemClass, const bool bCheckVisibility = false) const;

	/**Return the amount of a given item that we have space for based on its weight, max stack size, etc.
	@param NoSpaceReason If we have no space for the item, this is the reason why */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual int32 GetSpaceForItem(TSubclassOf<class URItem> ItemClass, FText& NoSpaceReason) const;

	/** Return the amount of currency we have */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCurrency() const;

	/** Add some currency to the players inventory. Can accept negative values. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Currency")
	virtual void AddCurrency(const int32 Amount);

	/** set the amount of currency the player has . */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Currency")
	virtual void SetCurrency(const int32 Amount);

	//Get the current weight of the inventory. To get the amount of items in the inventory, just do GetItems().Num()
	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightCapacity(const float NewWeightCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacity(const int32 NewCapacity);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryFriendlyName(const FText& Name);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetIsVendor(const bool bNewIsVendor);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const { return WeightCapacity; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return Capacity; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE TArray<class URItem*> GetItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "Narrative")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "Narrative")
	virtual APlayerController* GetOwningController() const;

	UFUNCTION(Client, Reliable)
	void ClientRefreshInventory();

	/**
	* Create a save file with all the players items in it which can be loaded back in by calling the Load function.
	* 
	@param SaveName the name of the save game. */
	 /*UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
	 virtual bool Save(const FString& SaveName = "InventorySaveData", const int32 Slot = 0);
	
	 /**Load narratives state back in from disk
	 @param SaveName the name of the save game. */
	/* UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
	 virtual bool Load(const FString& SaveName = "InventorySaveData", const int32 Slot = 0);
	
	 /**Deletes a saved game from disk. USE THIS WITH CAUTION. Return true if save file deleted, false if delete failed or file didn't exist.*/
	/* UFUNCTION(BlueprintCallable, Category = "Saving")
	 virtual bool DeleteSave(const FString& SaveName = "InventorySaveData", const int32 Slot = 0);*/

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	//Delegate for an item being added. Currently only called server side.
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	//Delegate for an item being removed. Called on server and client 
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;
		
	//Delegate for an item being Used. Currently only called server side.
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemUsed OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnBeginLooting OnBeginLooting;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEndLooting OnEndLooting;

	//Notify when our currency balance changes 
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnCurrencyChanged OnCurrencyChanged;

protected:

	//The name of this inventory - used by the UI that ships with Narrative Inventory. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FText InventoryFriendlyName;

	//The maximum weight the inventory can hold. For players, backpacks and other items increase this limit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (Units = "Kilograms"))
	float WeightCapacity;

	//The maximum number of items the inventory can hold. For players, backpacks and other items increase this limit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0, ClampMax = 1000))
	int32 Capacity;

	/**The inventory we're currently looting from*/
	UPROPERTY(ReplicatedUsing = OnRep_LootSource, VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory")
	class URInventoryComponent* LootSource;

	/**The items currently in our inventory*/
	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleInstanceOnly, Category = "Inventory")
	TArray<class URItem*> Items;

	/**The amount of currency our player has*/
	UPROPERTY(ReplicatedUsing = OnRep_Currency, EditDefaultsOnly, Category = "Inventory")
	int32 Currency;

	/**Items that should be put in our inventory on BeginPlay. We savegame this as to only add the items once.  */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TArray<FItemWithQuantity> DefaultItems;

	/** Loot table we'll use to generate this inventories items. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FLootTableRoll> DefaultItemTables;

	/** Nice way to ensure we don't give default items multiple times.  */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Inventory")
	bool bGaveDefaultItems;

	/** If true, this inventory will be treated as a vendor, meaning taking/giving items will check you/the vendor has enough currency, and will 
	give you/the vendor the currency when the taking/giving occurs also. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bIsVendor;

	//The percentage of the items value we'll buy items for if we're a vendor. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor Settings")
	float BuyItemPct;

	//The percentage of the items value we'll sell items for 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor Settings")
	float SellItemPct;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	//Internal, non-BP exposed add item function. Don't call this directly, use TryAddItem(), or TryAddItemFromClass() instead.
	virtual FItemAddResult TryAddItem_Internal(TSubclassOf<class URItem> ItemClass, const int32 Quantity = 1);

public:

	/** Instead of calling this automatically on beginplay we let designers call this when they like.  */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void GiveDefaultItems();

	/**Sets our loot source. Only the server can call this for obvious reasons! */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	virtual void SetLootSource(class URInventoryComponent* NewLootSource);

protected:

	UFUNCTION(Server, Reliable)
	virtual void ServerRemoveItem(class URItem* Item);

	UFUNCTION(Server, Reliable)
	virtual void ServerConsumeItem(class URItem* Item, const int32 Quantity);

	UFUNCTION(Server, Reliable)
	virtual void ServerUseItem(class URItem* Item);

	UFUNCTION(Server, Reliable)
	virtual void ServerStopLooting();

	/**Stop looting. Clients will RPC to the server to end looting. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void StopLooting();

	/** Loot an item. Clients RPC to tell server they want to do this; server will validate to prevent cheating
		If we're client return true/false if we notified server to loot. Server returns true if loot actually happened. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool RequestLootItem(class URItem* ItemToLoot, FText& ErrorText, const int32 Quantity = 1);

	UFUNCTION(Server, Reliable)
	virtual void ServerRequestLootItem(class URItem* ItemToLoot, const int32 Quantity);

	/** Store an item. Clients RPC to tell server they want to do this; server will validate to prevent cheating
	If we're client return true/false if we notified server to store. Server returns true if store actually happened. 
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool RequestStoreItem(class URItem* ItemToStore, FText& ErrorText, const int32 Quantity = 1);

	UFUNCTION(Server, Reliable)
	virtual void ServerRequestStoreItem(class URItem* ItemToLoot, const int32 Quantity);

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY()
	class APlayerController* OwnerPC;

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

private:
	
	/**Don't call Items.Add() directly, use this function instead, as it handles replication and ownership*/
	URItem* AddItem(TSubclassOf<class URItem> ItemClass, const int32 Quantity);

	UFUNCTION()
	void OnRep_Items(const TArray<class URItem*>& OldItems);
	
	UFUNCTION()
	void OnRep_Currency(const int32 OldCurrency);

	UFUNCTION()
	void OnRep_LootSource(class URInventoryComponent* OldLootSource);

	UPROPERTY()
	int32 ReplicatedItemsKey;

};

