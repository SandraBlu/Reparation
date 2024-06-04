// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/REquipmentComponent.h"

#include "Actors/RWeapon.h"
#include "Components/SkeletalMeshComponent.h"


// Sets default values for this component's properties
UREquipmentComponent::UREquipmentComponent()
{

}

void UREquipmentComponent::Initialize(TMap<EEquipSlot, USkeletalMeshComponent*> ClothingMeshes, class USkeletalMeshComponent* InLeaderPoseComponent)
{
	LeaderPoseComponent = InLeaderPoseComponent;

	EquippableComponents = ClothingMeshes;

	for (auto& ClothingMeshKP : ClothingMeshes)
	{
		if (ClothingMeshKP.Value)
		{
			DefaultClothing.Add(ClothingMeshKP.Key, ClothingMeshKP.Value->GetSkeletalMeshAsset());
			DefaultClothingMaterials.Add(ClothingMeshKP.Key, FDefaultClothingMeshMaterials(ClothingMeshKP.Value->GetMaterials()));
		}
	}
}

UREquipItem* UREquipmentComponent::GetEquippedItemAtSlot(const EEquipSlot Slot)
{
	if (EquippedItems.Contains(Slot))
	{
		return *EquippedItems.Find(Slot);
	}
	else
	{
		return nullptr;
	}
}


float UREquipmentComponent::GetEquippedItemsWeight() const
{
	float TotalWeight = 0.f;

	for (auto& EquippedItemKP : EquippedItems)
	{
		if (EquippedItemKP.Value)
		{
			TotalWeight += EquippedItemKP.Value->Weight;
		}
	}

	return TotalWeight;
}

void UREquipmentComponent::EquipItem(class UREquipItem* Equippable)
{
	if (Equippable)
	{
		//Remove the old item from our equipped items if one is already equipped
		if (EquippedItems.Contains(Equippable->Slot))
		{
			if (UREquipItem* AlreadyEquippedItem = *EquippedItems.Find(Equippable->Slot))
			{
				AlreadyEquippedItem->SetActive(false);
			}
		}

		EquippedItems.Add(Equippable->Slot, Equippable);

		Equippable->HandleEquip();

		OnItemEquipped.Broadcast(Equippable->Slot, Equippable);
	}
}

void UREquipmentComponent::UnequipItem(class UREquipItem* Equippable)
{
	if (Equippable)
	{
		EquippedItems.Remove(Equippable->Slot);

		Equippable->HandleUnequip();

		OnItemUnequipped.Broadcast(Equippable->Slot, Equippable);
	}
}

void UREquipmentComponent::OnWeaponEquipped()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->OnEquip();
	}
}

