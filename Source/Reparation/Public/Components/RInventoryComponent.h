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
	TArray<class URItem*> CreatedStacks = {};

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
		AddedSomeResult.CreatedStacks = InItems;
		AddedSomeResult.ErrorText = ErrorText;

		return AddedSomeResult;
	}

	static FItemAddResult AddedAll(TArray<class URItem*> InItems,const int32 InItemQuantity)
	{
		FItemAddResult AddAllResult(InItemQuantity, InItemQuantity);
		AddAllResult.CreatedStacks = InItems;
		
		return AddAllResult;
	}
};

//Called when Inventory is changed to update UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

/**Called on server when an item is added to this inventory*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, const FItemAddResult&, AddResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, class URItem*, Item, const int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, class URItem*, Item);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginLooting, class URInventoryComponent*, LootSource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndLooting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyChanged, const int32, OldCurrency, const int32, NewCurrency);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPARATION_API URInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class URItem;

public:	

	URInventoryComponent(const FObjectInitializer& ObjectInitializer);

UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool UseItem(class URItem* Item);
	
	//Add Item To Inventory using class instead of Instance/ for items that don't yet exist
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<class URItem> ItemClass, const int32 Quantity = 1);
	
	//for using more than one of item like ammo or health potions
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 ConsumeItem(class URItem* Item, const int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(class URItem* Item);

	//Has Item? Find Item Functions
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSubclassOf<class URItem> ItemClass, const int32 Quantity = 1,  const bool bCheckVisibility = false) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	URItem* FindItemByClass(TSubclassOf<class URItem> ItemClass, const bool bCheckVisibility = false) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<URItem*> FindItemsByClass(TSubclassOf<class URItem> ItemClass, const bool bCheckVisibility = false) const;
	
	//Return the amount of a given item that we have space for based on its weight, max stack size, etc. 
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual int32 GetSpaceForItem(TSubclassOf<class URItem> ItemClass, FText& NoSpaceReason) const;

	/**Return true if the taker can loot the item from this inventory */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual bool AllowLootItem(class URInventoryComponent* Taker, TSubclassOf <class URItem> ItemClass, const int32 Quantity, FText& ErrorText) const;

	/**Return true if the storer can store the given item in this inventory */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual bool AllowStoreItem(class URInventoryComponent* Storer, TSubclassOf <class URItem> ItemClass, const int32 Quantity, FText& ErrorText) const;

	/** Remove the item from us, and give it to the taker. Return how much of the item was able to be looted */
	virtual FItemAddResult PerformLootItem(class URInventoryComponent* Taker, TSubclassOf <class URItem> ItemClass, const int32 Quantity = 1);

	/** Remove the item from the storer, and give it to us. Return how much of the item was able to be stored*/
	virtual FItemAddResult PerformStoreItem(class URInventoryComponent* Storer, TSubclassOf <class URItem> ItemClass, const int32 Quantity = 1);

	/** Return the amount of currency we have */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCurrency() const;

	/** Add some currency to the players inventory. Can accept negative values. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Currency")
	virtual void AddCurrency(const int32 Amount);

	/** set the amount of currency the player has . */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Currency")
	virtual void SetCurrency(const int32 Amount);
	
	//Weight/Capacity Functions
	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightCapacity(const float NewWeightCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacity(const int32 NewCapacity);

	//Core Functions
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const { return WeightCapacity; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return Capacity; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE TArray<class URItem*> GetItems() const { return Items; };

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "Narrative")
	virtual APlayerController* GetOwningController() const;

	UPROPERTY()
	class APlayerController* OwnerPC;

	UFUNCTION(Client, Reliable)
	void ClientRefreshInventory();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemUsed OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnBeginLooting OnBeginLooting;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEndLooting OnEndLooting;

	//Notify when our currency balance changes 
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnCurrencyChanged OnCurrencyChanged;
	

	/**
* Create a save file with all the players items in it which can be loaded back in by calling the Load function.
* 
@param SaveName the name of the save game. */
	//UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
	//virtual bool Save(const FString& SaveName = "NarrativeInventorySaveData", const int32 Slot = 0);

	/**Load narratives state back in from disk
	@param SaveName the name of the save game. */
	//UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
	//virtual bool Load(const FString& SaveName = "NarrativeInventorySaveData", const int32 Slot = 0);

	/**Deletes a saved game from disk. USE THIS WITH CAUTION. Return true if save file deleted, false if delete failed or file didn't exist.*/
//	UFUNCTION(BlueprintCallable, Category = "Saving")
//	virtual bool DeleteSave(const FString& SaveName = "NarrativeInventorySaveData", const int32 Slot = 0);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (Units = "Kilograms"))
	float WeightCapacity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0, ClampMax = 100))
	int32 Capacity;

	/**The inventory we're currently looting from*/
	UPROPERTY(ReplicatedUsing = OnRep_LootSource, VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory")
	class URInventoryComponent* LootSource;
	
	UPROPERTY(ReplicatedUsing = OnRep_Currency, EditDefaultsOnly, Category = "Inventory")
	int32 Currency;

	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleAnywhere, Category = "Inventory")
	TArray<class URItem*> Items;

	/**Items that should be put in our inventory on BeginPlay */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Inventory")
	TArray<class URItem*> DefaultItems;
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION(Server, Reliable)
	virtual void ServerUseItem(class URItem* Item);
	
	UFUNCTION(Server, Reliable)
	virtual void ServerConsumeItem(class URItem* Item, const int32 Quantity);

	UFUNCTION(Server, Reliable)
	virtual void ServerRemoveItem(class URItem* Item);

	//Internal, not exposed to BP;  Try Add item or Try add item from class functions will call this internal function
	FItemAddResult TryAddItem_Internal(TSubclassOf<class URItem> ItemClass, const int32 Quantity = 1);

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

	/**Sets our loot source. Only the server can call this for obvious reasons! */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	virtual void SetLootSource(class URInventoryComponent* NewLootSource);
	
private:

	// Don't call Items.Add() directly. Use this instead, as it handles ownership
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
