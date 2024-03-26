// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AOGearItem.h"
#include "Character/AOPlayerBase.h"

UAOGearItem::UAOGearItem()
{
	DamageDefenseMult = .1f;
}

bool UAOGearItem::Equip(class AAOPlayerBase* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{
		Character->EquipGear(this);
	}
	return bEquipSuccessful;
}

bool UAOGearItem::Unequip(class AAOPlayerBase* Character)
{
	bool bUnEquipSuccessful = Super::Unequip(Character);

	if (bUnEquipSuccessful && Character)
	{
		Character->UnequipGear(Slot);
	}
	return bUnEquipSuccessful;
}
