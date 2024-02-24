// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AOInventoryComponent.generated.h"

//Called when Inventory is changed to update UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	EIAR_NoItemsAdded UMETA(DisplayName = "No Items Added"),
	EIAR_SomeItemsAdded UMETA(DisplayName = "Some Items Added"),
	EIAR_AllItemsAdded UMETA(DisplayName = "All Items Added")
};

//Represents result of Adding item to Inventory
USTRUCT(BlueprintType)
struct FItemAddResult
{
	GENERATED_BODY()

public:

	FItemAddResult() {};
	FItemAddResult(int32 InItemQuantity) : AmountToGive(InItemQuantity), ActualAmountGiven(0) {};
	FItemAddResult(int32 InItemQuantity, int32 InQuantityAdded) : AmountToGive(InItemQuantity), ActualAmountGiven(InQuantityAdded) {};


	//Amount of item try to add
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
		int32 AmountToGive;

	//Amount Actually added
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
		int32 ActualAmountGiven;

	//Result
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
		EItemAddResult Result;

	//Error message explaining why could not be added; ie too many, too heavy
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
		FText ErrorText;

	//Helper Functions for Results to try add item
	static FItemAddResult AddedNone(const int32 InItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult(InItemQuantity);
		AddedNoneResult.Result = EItemAddResult::EIAR_NoItemsAdded;
		AddedNoneResult.ErrorText = ErrorText;

		return AddedNoneResult;
	}

	static FItemAddResult AddedSome(const int32 InItemQuantity, const int32 ActualAmountGiven, const FText& ErrorText)
	{
		FItemAddResult AddedSomeResult(InItemQuantity, ActualAmountGiven);
		AddedSomeResult.Result = EItemAddResult::EIAR_SomeItemsAdded;
		AddedSomeResult.ErrorText = ErrorText;

		return AddedSomeResult;
	}

	static FItemAddResult AddedAll(const int32 InItemQuantity)
	{
		FItemAddResult AddAllResult(InItemQuantity, InItemQuantity);
		AddAllResult.Result = EItemAddResult::EIAR_AllItemsAdded;


		return AddAllResult;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPARATION_API UAOInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	//Allows UItem class to access private and protected functions
	friend class UAOItem;

public:	
	
	UAOInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItem(class UAOItem* Item);

//Add Item To Inventory using class instead of Instance/ for items that don't yet exist
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<class UAOItem> ItemClass, const int32 Quantity);

	//for using one item
	int32 ConsumeItem(class UAOItem* Item);
	//for using more than one of item like ammo or health potions
	int32 ConsumeItem(class UAOItem* Item, const int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(class UAOItem* Item);

	//Has Item? Find Item Functions
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSubclassOf<class UAOItem> ItemClass, const int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
		UAOItem* FindItem(class UAOItem* Item) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
		UAOItem* FindItemByClass(TSubclassOf<class UAOItem> ItemClass) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<UAOItem*> FindItemsByClass(TSubclassOf<class UAOItem> ItemClass) const;

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
	FORCEINLINE TArray<class UAOItem*> GetItems() const { return Items; };

	UFUNCTION()
	void RefreshInventory();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float WeightCapacity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0, ClampMax = 100))
	int32 Capacity;

	//UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleAnywhere, Category = "Inventory")
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<class UAOItem*> Items;

private:

	// Don't call Items.Add() directly. Use this instead, as it handles ownership
	UAOItem* AddItem(class UAOItem* Item);

	//Internal, not exposed to BP;  Try Add item or Try add item from class functions will call this internal function
	FItemAddResult TryAddItem_Internal(class UAOItem* Item);

		
};
