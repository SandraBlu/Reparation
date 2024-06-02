// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RCharacterClassData.generated.h"

class UGameplayEffect;
class UGameplayAbility;

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	Elementalist, 
	Warrior,
	Ranger
};

USTRUCT(BlueprintType)
struct FCharClassInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	//UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	//FScalableFloat XPReward = FScalableFloat();

};
/**
 * 
 */
UCLASS()
class REPARATION_API URCharacterClassData : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ECharacterClass, FCharClassInfo> CharClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults")
	TSubclassOf<UGameplayEffect> ResistanceAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;
	
	FCharClassInfo GetClassInfo(ECharacterClass CharacterClass);

	//UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults")
	//TArray<TSubclassOf<UGameplayAbility>> SharedAbilities;

	//UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults|Damage")
	//UCurveTable* DamageCalcCoefficients;
	
};
