// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AOWeaponItem.h"
#include "Character/AOPlayerBase.h"
#include "Framework/AOPlayerController.h"

UAOWeaponItem::UAOWeaponItem()
{

}

bool UAOWeaponItem::Equip(class AAOPlayerBase* Character)
{
    bool bEquipSuccessful = Super::Equip(Character);
    if (bEquipSuccessful && Character)
    {
        Character->EquipWeapon(this);
    }
    return bEquipSuccessful;
}

bool UAOWeaponItem::Unequip(class AAOPlayerBase* Character)
{
    bool bUnequipSuccessful = Super::Unequip(Character);
    if (bUnequipSuccessful && Character)
    {
        Character->UnequipWeapon();
    }
    return bUnequipSuccessful;
}
