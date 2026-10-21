// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilityInfo.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum class ERSkillDiscipline : uint8 { Melee, Ranged, Mobility, Stealth, Elemental, Companion, Other };

UENUM(BlueprintType)
enum class ERRespecPayment : uint8 { Gold, SkillPoints, QuestItem };

USTRUCT(BlueprintType)
struct FRSkillPrerequisite
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag AbilityTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) int32 RequiredRank = 1;
};

/** Quest eligibility is level OR investment; learning still requires the completion unlock. */
USTRUCT(BlueprintType)
struct FRSkillQuestUnlock
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) int32 OfferAtLevel = 15;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) int32 OfferAtCorePointsSpent = 10;
};

USTRUCT(BlueprintType)
struct FRAbilityInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag = FGameplayTag();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityType = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMaterialInterface* BGMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LevelRequirement = 1;
	/** Zero uses the progression data's current cap; a positive value authors a lower rank cap. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0"))
	int32 MaximumAbilityLevel = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) ERSkillDiscipline Discipline = ERSkillDiscipline::Other;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText DisplayName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText Summary;
	/** Index zero describes rank one. Empty entries retain the ability's existing description. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TArray<FText> RankDescriptions;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TArray<FRSkillPrerequisite> Prerequisites;
	/** Tier zero is a legacy/shared technique. Higher tiers require learned skills in the preceding tier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", ClampMax="5")) int32 BranchTier = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="1")) int32 PreviousTierSkillsRequired = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="1",ClampMax="5")) int32 SkillPointCost = 1;
	/** Only purchases in the three lower branches contribute; starting grants do not. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bCountsTowardsCoreInvestment = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0")) int32 MinimumCorePointsSpent = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName RequiredProgressionUnlock;
	/** Retain legacy/gear definitions without offering them as skill-tree purchases. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bSkillTreePurchasable = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};
/**
 * 
 */
UCLASS()
class REPARATION_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Respec") ERRespecPayment RespecPayment = ERRespecPayment::Gold;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Respec", meta=(ClampMin="1")) int32 RespecCost = 100;
	/** Set a real quest reward item class before selecting QuestItem payment. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Respec") TSoftClassPtr<class URItem> RespecItem;
	/** First four entries map to passive01..04. Omitted entries or levels above 40 disable that slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout") TArray<int32> PassiveSlotUnlockLevels = {1, 10, 20};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInfo")
	TArray<FRAbilityInfo> AbilityInfo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Progression") TArray<FRSkillQuestUnlock> QuestUnlocks;
	/** Free fixed-control reward for completing a full authored tier. Planned skills count toward the required total. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tier Reward") FGameplayTag AutomaticTierReward;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tier Reward", meta=(ClampMin="1",ClampMax="5")) int32 RewardTier = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tier Reward", meta=(ClampMin="1")) int32 RewardSkillsPerBranch = 3;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tier Reward") bool bRewardRequiresAllBranches = false;

	FRAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;
	
};
