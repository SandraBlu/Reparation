// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScalableFloat.h"
#include "RNPCData.generated.h"

class UGameplayEffect;
class UGameplayAbility;

UENUM(BlueprintType)
enum class ENPCClass : uint8
{
	Elementalist, 
	Warrior,
	Ranger
};

USTRUCT(BlueprintType)
struct FNPCClassInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	FScalableFloat XPReward = FScalableFloat();

};
/**
 * 
 */
UCLASS()
class REPARATION_API URNPCData : public UDataAsset
{
	GENERATED_BODY()

	public:

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ENPCClass, FNPCClassInfo> NPCClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults")
	TSubclassOf<UGameplayEffect> ResistanceAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;
	
	FNPCClassInfo GetClassInfo(ENPCClass NPCClass);

	UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> SharedAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Shared Class Defaults|Damage")
	UCurveTable* DamageCalcCoefficients;
	
};
