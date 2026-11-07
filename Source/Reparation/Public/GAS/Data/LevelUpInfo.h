// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"


struct FRSourceXP;
enum class ERXPSource : uint8;

USTRUCT(BlueprintType)
struct FRLevelUpInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int32 LevelUpRequirement = 0;
	
	UPROPERTY(EditDefaultsOnly)
	int32 AttributePointReward = 1;
	
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityPointReward = 1;
	
};

USTRUCT(BlueprintType)
struct FRItemXPRow 
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	FName Source;
	
	UPROPERTY(EditDefaultsOnly)
	int32 Chapter = 1;
	
	UPROPERTY(EditDefaultsOnly)
	int32 XP = 0;
	
	UPROPERTY(EditDefaultsOnly)
	bool bCatchUpEligible = false;
};

/**
 * 
 */
UCLASS()
class REPARATION_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	TArray<FRLevelUpInfo> LevelUpInfo;

	UPROPERTY(EditDefaultsOnly, Category="XP")
	TArray<FRSourceXP> SourceXP;
	
	UPROPERTY(EditDefaultsOnly, Category="XP")
	TArray<int32> ChapterEndLevels; // 5,9,13,17,21,25,29,33,37,40
	
	UPROPERTY(EditDefaultsOnly, Category="XP")
	int32 CatchUpTrigger = 1;
	
	UPROPERTY(EditDefaultsOnly, Category="XP")
	float CatchUpPerLevel = 0.08f;
	
	UPROPERTY(EditDefaultsOnly, Category="XP")
	float CatchUpCap = 0.30f;

	int32 FindLevelForXP(int32 XP) const;
	int32 GetMaxLevel() const { return FMath::Max(1, LevelUpInfo.Num() - 1); }
	float GetProgressForXP(int32 XP) const;
	int32 GetAttributeReward(int32 FromLevel) const;
	int32 GetAbilityReward(int32 FromLevel) const;
	int32 GetItemXP(ERXPSource Source, int32 Chapter) const;
	float GetCatchUpMultiplier(int32 Chapter, int32 CurrentLevel) const;
	int32 GetAwardedXP(ERXPSource Source, int32 Chapter, int32 CurrentLevel) const;
	int32 GetXPIntoLevel(int32 XP) const;
	int32 GetXPToNextLevel(int32 XP) const;
	
};
