// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/RItem.h"
#include "REquipItem.generated.h"

UENUM(BlueprintType)
enum class EEquipSlot : uint8 
{
	ES_Head UMETA(DisplayName = "Head"),
	ES_Headwear UMETA(DisplayName = "Headwear"),
	ES_Torso UMETA(DisplayName = "Torso"),
	ES_Legs UMETA(DisplayName = "Legs"),
	ES_Feet UMETA(DisplayName = "Feet"),
	ES_Hands UMETA(DisplayName = "Hands"),
	ES_Backpack UMETA(DisplayName = "Backpack"),
	ES_Weapon UMETA(DisplayName = "Weapon"),
	ES_Sheath UMETA(DisplayName = "Sheath"),
	ES_Throwable UMETA(DisplayName = "Throwable")
	
};

/**
 * 
 */
UCLASS()
class REPARATION_API UREquipItem : public URItem
{
	GENERATED_BODY()

public:

	friend class UREquipmentComponent;

	UREquipItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	EEquipSlot Slot;

	/**Allows you to override what happens when the item Equips add effect that follows player or something */
	UFUNCTION(BlueprintNativeEvent, Category = "Equippable")
	void HandleEquip();
	virtual void HandleEquip_Implementation();

	/**Allows you to override what happens when the item un equips. */
	UFUNCTION(BlueprintNativeEvent, Category = "Equippable")
	void HandleUnequip();
	virtual void HandleUnequip_Implementation();

	virtual void Activated_Implementation() override;
	
	virtual void Deactivated_Implementation() override;
	
};

/**
 * Defines a clothing item. Will use the skeletal mesh set in your EquipmentComponent and set it to the clothing mesh you select.
 */
UCLASS()
class REPARATION_API UREquipItem_Clothing : public UREquipItem
{
	GENERATED_BODY()

public:

	UREquipItem_Clothing();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif 

	virtual void HandleEquip_Implementation() override;
	virtual void HandleUnequip_Implementation() override;

	/**The mesh to put on the player when they equip the item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable")
	class USkeletalMesh* ClothingMesh;

	/**The materials to apply to the clothing mesh*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable")
	TArray<class UMaterialInterface*> ClothingMaterials;

};

/**
 * Defines a weapon item. Will use AO Weapon set in your EquipmentComponent and set it to the weapon you select.
 */
UCLASS()
class REPARATION_API UREquipItem_Weapon : public UREquipItem
{
	GENERATED_BODY()

public:

	UREquipItem_Weapon();

	virtual void HandleEquip_Implementation() override;
	virtual void HandleUnequip_Implementation() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class ARWeapon> WeaponClass;
};
