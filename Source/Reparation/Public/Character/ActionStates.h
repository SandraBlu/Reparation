#pragma once

UENUM(BlueprintType)
enum class EEquipState : UINT8
{
	EES_Unequipped UMETA(DisplayName = "Unequipped"),
	EES_EquippedMelee UMETA(DisplayName = "EquippedMelee"),
	EES_EquippedRanged UMETA(DisplayName = "EquippedRanged")
};

UENUM(BlueprintType)
enum class EActionState : UINT8
{
	EAS_Idle UMETA(DisplayName = "Idle"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_EquippingWeapon UMETA(DisplayName = "Equipping")
};

UENUM(BlueprintType)
enum class EFoot : uint8
{
	Left UMETA(DisplayName = "Left"), 
	Right UMETA(DisplayName = "Right")
};