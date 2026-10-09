

// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/REquipmentComponent.h"

#include "Actors/RWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/RInventoryComponent.h"
#include "Framework/RInventoryBFL.h"

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
			ConfigureSlotComponent(ClothingMeshKP.Key);
		}
	}
}

void UREquipmentComponent::ConfigureSlotComponent(EEquipSlot Slot)
{
	USkeletalMeshComponent* const* FoundComponent = EquippableComponents.Find(Slot);
	USkeletalMeshComponent* SlotComponent = FoundComponent ? *FoundComponent : nullptr;
	if (!SlotComponent || SlotComponent == LeaderPoseComponent) return;

	const bool bSocketAttachment = Slot == EEquipSlot::ES_Backpack || Slot == EEquipSlot::ES_Headwear;
	if (!bSocketAttachment)
	{
		if (LeaderPoseComponent) SlotComponent->SetLeaderPoseComponent(LeaderPoseComponent);
		return;
	}

	// Accessories follow a body socket rather than copying the body's skeleton.
	SlotComponent->SetLeaderPoseComponent(nullptr);
	if (!LeaderPoseComponent) return;

	FName Socket = Slot == EEquipSlot::ES_Backpack ? BackpackSocket : HeadwearSocket;
	if (Slot == EEquipSlot::ES_Backpack && !LeaderPoseComponent->DoesSocketExist(Socket))
	{
		Socket = BackpackFallbackSocket;
	}
	if (Socket.IsNone() || !LeaderPoseComponent->DoesSocketExist(Socket))
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipment on %s cannot attach slot %d: body socket/bone %s is missing."),
			*GetNameSafe(GetOwner()), static_cast<int32>(Slot), *Socket.ToString());
		return;
	}
	SlotComponent->AttachToComponent(LeaderPoseComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
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
		if (const UREquipItem_Weapon* WeaponItem = Cast<UREquipItem_Weapon>(Equippable))
		{
			if (!WeaponItem->WeaponClass) return;
		}
		if (GetEquippedItemAtSlot(Equippable->Slot) == Equippable) return;
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
	if (Equippable && GetEquippedItemAtSlot(Equippable->Slot) == Equippable)
	{
		EquippedItems.Remove(Equippable->Slot);

		Equippable->HandleUnequip();

		OnItemUnequipped.Broadcast(Equippable->Slot, Equippable);
	}
}

void UREquipmentComponent::OnWeaponEquipped() const
{
	if (EquippedWeapon)
	{
		EquippedWeapon->OnEquip();
	}
}

bool UREquipmentComponent::IsWeaponArmed() const
{
	return IsValid(EquippedWeapon) && EquippedWeapon->IsEquipped;
}

void UREquipmentComponent::DrawWeapon()
{
	const bool bWasArmed = IsWeaponArmed();
	if (!IsValid(EquippedWeapon))
	{
		if (UREquipItem_Weapon* Item = Cast<UREquipItem_Weapon>(GetEquippedItemAtSlot(EEquipSlot::ES_Weapon)))
		{
			Item->HandleEquip();
		}
		else if (URInventoryComponent* Inventory = URInventoryBFL::GetInventoryComponentFromTarget(GetOwner()))
		{
			for (URItem* Candidate : Inventory->GetItems())
			{
				UREquipItem_Weapon* WeaponItem = Cast<UREquipItem_Weapon>(Candidate);
				if (WeaponItem && WeaponItem->Slot == EEquipSlot::ES_Weapon && WeaponItem->WeaponClass)
				{
					WeaponItem->SetActive(true);
					break;
				}
			}
		}
	}
	if (IsValid(EquippedWeapon) && !bWasArmed) EquippedWeapon->ArmWithMontage();
	OnWeaponChanged.Broadcast();
}

void UREquipmentComponent::DisarmWeapon()
{
	if (IsWeaponArmed()) EquippedWeapon->DisarmWithMontage();
	OnWeaponChanged.Broadcast();
}

void UREquipmentComponent::ToggleWeapon()
{
	if (IsWeaponArmed()) DisarmWeapon();
	else DrawWeapon();
}

void UREquipmentComponent::SwitchWeapon()
{
	URInventoryComponent* Inventory = URInventoryBFL::GetInventoryComponentFromTarget(GetOwner());
	if (!Inventory) return;
	TArray<UREquipItem_Weapon*> Weapons;
	for (URItem* Item : Inventory->GetItems())
	{
		UREquipItem_Weapon* Candidate = Cast<UREquipItem_Weapon>(Item);
		if (Candidate && Candidate->Slot == EEquipSlot::ES_Weapon && Candidate->WeaponClass) Weapons.Add(Candidate);
	}
	if (Weapons.IsEmpty()) return;
	const int32 Current = Weapons.IndexOfByKey(Cast<UREquipItem_Weapon>(GetEquippedItemAtSlot(EEquipSlot::ES_Weapon)));
	if (Weapons.Num() == 1 && Current == 0) return;
	const bool bWasArmed = IsWeaponArmed();
	TGuardValue<bool> ArmGuard(bArmNewWeapon, bWasArmed);
	Weapons[(Current + 1) % Weapons.Num()]->SetActive(true);
}

