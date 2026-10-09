// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/REquipItem.h"
#include <Components/REquipmentComponent.h>
#include "Engine/SkinnedAssetCommon.h"
#include "Actors/RWeapon.h"

#define LOCTEXT_NAMESPACE "EquipItem"

UREquipItem::UREquipItem()
{
	bStackable = false;
	Weight = 1.f;
	bCanActivate = true;
	bToggleActiveOnUse = true;
	UseActionText = LOCTEXT("ItemUseActionText", "Equip");
}

void UREquipItem::HandleEquip_Implementation()
{

}

void UREquipItem::HandleUnequip_Implementation()
{

}

void UREquipItem::Activated_Implementation()
{
	if (GetOwningPawn())
	{
		UseActionText = LOCTEXT("UnequipText", "Unequip");

		if (UREquipmentComponent* EquipmentComponent = Cast<UREquipmentComponent>(GetOwningPawn()->GetComponentByClass(UREquipmentComponent::StaticClass())))
		{
			EquipmentComponent->EquipItem(this);
		}
	}
}

void UREquipItem::Deactivated_Implementation()
{
	if (GetOwningPawn())
	{
		UseActionText = LOCTEXT("EquipText", "Equip");

		if (UREquipmentComponent* EquipmentComponent = Cast<UREquipmentComponent>(GetOwningPawn()->GetComponentByClass(UREquipmentComponent::StaticClass())))
		{
			EquipmentComponent->UnequipItem(this);
		}
	}
}

UREquipItem_Clothing::UREquipItem_Clothing()
{

}

#if WITH_EDITOR
void UREquipItem_Clothing::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UREquipItem_Clothing, ClothingMesh))
	{
		if (ClothingMesh)
		{
			ClothingMaterials.Empty();

			for (auto& ClothingMeshMat : ClothingMesh->GetMaterials())
			{
				ClothingMaterials.Add(ClothingMeshMat.MaterialInterface);
			}
		}
	}
}
#endif

void UREquipItem_Clothing::HandleUnequip_Implementation()
{
	if (UREquipmentComponent* EquipmentComponent = Cast<UREquipmentComponent>(GetOwningPawn()->GetComponentByClass(UREquipmentComponent::StaticClass())))
	{
		if (EquipmentComponent->EquippableComponents.Contains(Slot) && EquipmentComponent->DefaultClothing.Contains(Slot) && EquipmentComponent->DefaultClothingMaterials.Contains(Slot))
		{
			//Set the clothing back to its default mesh and materials
			if (USkeletalMeshComponent* SlotComponent = *EquipmentComponent->EquippableComponents.Find(Slot))
			{
				if (USkeletalMesh* DefaultClothingMesh = *EquipmentComponent->DefaultClothing.Find(Slot))
				{
					SlotComponent->SetSkeletalMesh(DefaultClothingMesh);

					if (FDefaultClothingMeshMaterials* DefaultMaterials = EquipmentComponent->DefaultClothingMaterials.Find(Slot))
					{
						int32 Idx = 0;

						for (auto& DefaultMat : DefaultMaterials->Materials)
						{
							SlotComponent->SetMaterial(Idx, DefaultMat);
							++Idx;
						}
					}
				}
				else
				{
					SlotComponent->SetSkeletalMesh(nullptr);
				}

				EquipmentComponent->ConfigureSlotComponent(Slot);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Equipment tried unequipping item %s but the Equipment Component doesn't have a skeletal mesh component added. Have you called initialize?"), *GetNameSafe(this));
		}

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipment tried unequipping your item, but your pawn doesn't have an Equipment Component added. Please add one."));
	}
}

void UREquipItem_Clothing::HandleEquip_Implementation()
{
	if (UREquipmentComponent* EquipmentComponent = Cast<UREquipmentComponent>(GetOwningPawn()->GetComponentByClass(UREquipmentComponent::StaticClass())))
	{
		if (ClothingMesh)
		{
			if (EquipmentComponent->EquippableComponents.Contains(Slot))
			{
				//Set the clothing mesh to the new mesh
				if (USkeletalMeshComponent* SlotComponent = *EquipmentComponent->EquippableComponents.Find(Slot))
				{
					SlotComponent->SetSkeletalMesh(ClothingMesh);

					int32 Idx = 0;
					for (auto& Mat : ClothingMaterials)
					{
						SlotComponent->SetMaterial(Idx, Mat);
						++Idx;
					}

					EquipmentComponent->ConfigureSlotComponent(Slot);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Equipment tried equipping %s but the Equipment Component doesn't have a skeletal mesh component added. Have you called Initialize?"), *GetNameSafe(this));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipment tried putting an item on, but your pawn doesn't have an Equipment Component added. Please add one."));
	}
}

UREquipItem_Weapon::UREquipItem_Weapon()
{
	Slot = EEquipSlot::ES_Weapon;
}

void UREquipItem_Weapon::HandleUnequip_Implementation()
{
	if (!GetOwningPawn()) return;
	UREquipmentComponent* EquipmentComponent = Cast<UREquipmentComponent>(GetOwningPawn()->GetComponentByClass(UREquipmentComponent::StaticClass()));
	if (EquipmentComponent && IsValid(EquipmentComponent->EquippedWeapon))
	{
		EquipmentComponent->EquippedWeapon->OnUnEquip();
		EquipmentComponent->EquippedWeapon->Destroy();
		EquipmentComponent->EquippedWeapon = nullptr;
		EquipmentComponent->OnWeaponChanged.Broadcast();
	}
}

void UREquipItem_Weapon::HandleEquip_Implementation()
{
	if (!GetOwningPawn()) return;
	if (WeaponClass)
	{
		UREquipmentComponent* EquipmentComponent = Cast<UREquipmentComponent>(GetOwningPawn()->GetComponentByClass(UREquipmentComponent::StaticClass()));
		if (!EquipmentComponent || IsValid(EquipmentComponent->EquippedWeapon)) return;
		//Spawn in Weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = SpawnParams.Instigator = GetOwningPawn();

		if (ARWeapon* Weapon = GetWorld()->SpawnActor<ARWeapon>(WeaponClass, SpawnParams))
		{
			EquipmentComponent->EquippedWeapon = Weapon;
			//Attach Weapon to Pawn
			Weapon->IsEquipped = false;
			if (EquipmentComponent->bArmNewWeapon) Weapon->OnEquip();
			else Weapon->OnUnEquip();
			EquipmentComponent->OnWeaponChanged.Broadcast();
		}
	}
}


#undef LOCTEXT_NAMESPACE

bool UREquipItem_Weapon::ShouldUseOnAdd_Implementation() const
{
	APawn* Pawn = GetOwningPawn();
	UREquipmentComponent* Equipment = Pawn ? Pawn->FindComponentByClass<UREquipmentComponent>() : nullptr;
	return WeaponClass && Equipment && !Equipment->GetEquippedItemAtSlot(Slot) && !IsValid(Equipment->GetEquippedWeapon());
}
