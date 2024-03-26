// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/AOEquipItem.h"
#include "AOWeaponItem.generated.h"

class AAOPlayerBase;
class AAOWeapon;
/**
 * 
 */
UCLASS(Blueprintable)
class REPARATION_API UAOWeaponItem : public UAOEquipItem
{
	GENERATED_BODY()

public:

	UAOWeaponItem();

	virtual bool Equip(class AAOPlayerBase* Character) override;
	virtual bool Unequip(class AAOPlayerBase* Character) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AAOWeapon> WeaponClass;
	
};
