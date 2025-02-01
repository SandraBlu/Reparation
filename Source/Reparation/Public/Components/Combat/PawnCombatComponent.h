// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/PawnExtensionComponentBase.h"
#include "PawnCombatComponent.generated.h"

class UREquipmentComponent;

UENUM(BlueprintType)
enum class EToggleDamageType : uint8
{
	CurrentArmedWeapon,
	LeftHand,
	RightHand
};
class ARWeapon;
/**
 * 
 */
UCLASS()
class REPARATION_API UPawnCombatComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RegisterSpawnedWeapon(FGameplayTag InWeaponTag, ARWeapon* InWeapon, bool bRegisterAsEquippedWeapon =false);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
	FGameplayTag CurrentEquippedWeaponTag;

	
private:
	TMap<FGameplayTag, ARWeapon*> CharacterWeaponMap;
	
};
