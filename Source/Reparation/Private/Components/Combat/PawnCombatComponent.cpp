// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PawnCombatComponent.h"
#include "RDebugHelper.h"
#include "RGameplayTags.h"
#include "Actors/RWeapon.h"
#include "Components/REquipmentComponent.h"

void UPawnCombatComponent::RegisterSpawnedWeapon(FGameplayTag InWeaponTag, ARWeapon* InWeapon, bool bRegisterAsEquippedWeapon)
{
	checkf (!CharacterWeaponMap.Contains(InWeaponTag), TEXT("A named %s has already been equipped"), *InWeaponTag.ToString());
	check (InWeapon);
	CharacterWeaponMap.Add(InWeaponTag, InWeapon);

	if (bRegisterAsEquippedWeapon)
	{
		CurrentEquippedWeaponTag = InWeaponTag;
	}
	const FString WeaponString = FString::Printf(TEXT("Weapon : %s is registered using tag %s"), *InWeapon->GetName(),*InWeaponTag.ToString());
	Debug::Print(WeaponString);
}
