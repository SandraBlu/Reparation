// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AOItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);

UENUM(BlueprintType, Blueprintable)
enum class EItemRarity : uint8
{
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_UnCommon UMETA(DisplayName = "UnCommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Epic UMETA(DisplayName = "Epic"),
	EIR_Legendary UMETA(DisplayName = "Legendary")
};
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class REPARATION_API UAOItem : public UObject
{
	GENERATED_BODY()

	protected:

	virtual class UWorld* GetWorld() const override;

	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:

	UAOItem();

	UPROPERTY(Transient)
	class UWorld* World;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundBase* PickupSFX;

	UPROPERTY()
	class UAOInventoryComponent* OwningInventory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText ItemDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	class UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	class UStaticMesh* PickupMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EItemRarity Rarity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	float Weight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	float RestoreAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	bool bStackable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta =(ClampMin = 2, EditCondition = bStackable))
	int32 MaxStackSize;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class UAOItemTooltip> ItemTooltip;*/

	UPROPERTY(EditAnywhere, Category = "Item", meta = (UIMin =1, EditCondition = bStackable))
	int32 Quantity;

	//Delegate
	UPROPERTY(BlueprintAssignable)
	FOnItemModified OnItemModified;

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetQuantity(const int32 NewQuantity);

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE int32 GetQuantity() const { return Quantity; };

	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE float GetStackWeight() const { return Quantity * Weight; };

	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool  ShouldShowInInventory() const;

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse(class AAOPlayerBase* Character);

	virtual void Use(class AAOPlayerBase* Character);
	virtual void AddedToInventory(class UAOInventoryComponent* Inventory);
	
};
