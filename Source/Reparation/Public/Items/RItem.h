// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_UnCommon UMETA(DisplayName = "UnCommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Epic UMETA(DisplayName = "Epic"),
	EIR_Legendary UMETA(DisplayName = "Legendary")
};

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class REPARATION_API URItem : public UObject
{
	GENERATED_BODY()
	
protected:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;
	virtual class UWorld* GetWorld() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	//Inspects owning inventory to check if this item instance is on server or client 
	bool HasAuthority() const;

public:

	URItem();

	UPROPERTY(Transient)
	class UWorld* World;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<class UStaticMesh> PickupMesh;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundBase* PickupSFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<class UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EItemRarity Rarity;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0, Units = "Kilograms"))
	float Weight;

	/** This is the value of the item, used by vendors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item - Trade")
	int32 BaseValue;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item - Stacking")
	bool bStackable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item - Usage")
	bool bConsumeOnUse;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	bool bFavorite;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item - Stacking", meta = (ClampMin = 2, EditCondition = bStackable, EditConditionHides))
	int32 MaxStackSize;

	/**The stats to show in the Item Preview UI - make sure to implement GetStringVariable function and return a value matching each stat you add.
	Items have 6 stats supported out of the box, Display Name, Weight, Stack Weight, Quantity, Recharge Duration, and Max Stack Size - if you want to add more simply
	override the GetStringVariable function and return a valid value for your desired variable*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item - UI")
	TArray<FString> Stats;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class URItemTooltip> ItemTooltip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	float RestoreAmount;
	
	/**The inventory that owns this item*/
    UPROPERTY(BlueprintReadOnly, Category = "Item")
    class URInventoryComponent* OwningInventory;
    	
    UPROPERTY()
    int32 RepKey;
    
    UPROPERTY(BlueprintAssignable)
    FOnItemModified OnItemModified;
	
	UFUNCTION()
	void OnRep_Quantity(const int32 OldQuantity);
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetQuantity(const int32 NewQuantity);
    
    UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE int32 GetQuantity() const { return Quantity; }
	
	//Allows you to define whether or not the item can be dropped, nice for if you want to implement special quest items that cannot be dropped for example 
	UFUNCTION(BlueprintCallable, Category = "Item")
    	void SetLastUseTime(const float NewLastUseTime);
    	
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE float GetLastUseTime() const { return LastUseTime; }
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE float GetStackWeight() const { return Quantity * Weight; };
	
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE int32 GetMaxStackSize() const{return bStackable ? MaxStackSize : 1;}

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE bool IsStackFull() const { return Quantity >= MaxStackSize; }

	//Return the amount of space the item stack has left 
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE int32 GetStackSpace() const{return GetMaxStackSize() - GetQuantity();}
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnUse();
    
	virtual void Use();
	virtual void AddedToInventory(class URInventoryComponent* Inventory);
	virtual void RemovedFromInventory(class URInventoryComponent* Inventory);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void MarkDirtyForReplication();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Item")
	bool ShouldShowInInventory() const;
	virtual bool ShouldShowInInventory_Implementation() const;
	
	//---------------NewAOItem-- Additions

	UPROPERTY(ReplicatedUsing = OnRep_bActive, BlueprintReadOnly, Category= "Item - Activation", meta = (EditCondition = bCanActivate, EditConditionHides))
	bool bActive;
	
    //Activated items will also show a check on the UI*/
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item - Activation")
    bool bCanActivate;
	
	/**If checked, using this item will cause it to activate/deactivate*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item - Activation")
	bool bToggleActiveOnUse;
	UFUNCTION()
	
	void OnRep_bActive(const bool bOldActive);
	/**Return true if this item is active */
    	
	UFUNCTION(BlueprintCallable, Category = "Item - Activation")
	void SetActive(const bool bNewActive);

	/**The amount of time you have to wait between uses of the item, 0 means no recharge delay at all*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item - Usage", meta = (ClampMin=0, Units = "Seconds"))
	float UseRechargeDuration;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Item")
	bool CanBeRemoved() const;
	virtual bool CanBeRemoved_Implementation() const;
	
	//If this item is activatable, this will be called when the item is activated
	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void Activated();
	virtual void Activated_Implementation();
    	
	//If this item is activatable, this will be called when the item is deactivated
	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void Deactivated();
	virtual void Deactivated_Implementation() ;
    	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Item")
	bool CanActivate() const;
	virtual bool CanActivate_Implementation() const;

	//Allows you to define whether the item can be used or not.
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Item")
	bool CanUse() const;
	virtual bool CanUse_Implementation() const;
	
//Return the description, with any string variables parsed out
	UFUNCTION(BlueprintPure, Category = "Item")
	virtual FText GetParsedDescription();
	
	//Allows you to use variables in your items description, ie Hello {Item Name}.
	//Simply override this function, then check the value of variable name and return whatever value you like!
	//ie if(VariableName == "PlayerName") {return OwningPawn->GetUsername();}; - then Hello {PlayerName}! will automatically become Hello xXNoobPwner420Xx! etc 
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Item")
	FString GetStringVariable(const FString& VariableName);
	virtual FString GetStringVariable_Implementation(const FString& VariableName);

	UFUNCTION(BlueprintPure, Category = "Item")
	APlayerController* GetOwningController() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	APawn* GetOwningPawn() const;

	/**Return the description with none of the string variables parsed*/
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE FText GetRawDescription() const {return Description;}
	
protected:

	/**The amount of the item*/
	UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, Category = "Item", meta = (UIMin = 1, EditCondition = bStackable, EditConditionHides))
	int32 Quantity;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Item")
	float LastUseTime;
};
