// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS/Data/AbilityInfo.h"
#include "RAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);
DECLARE_MULTICAST_DELEGATE(FAbilityGiven);
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChange, const FGameplayTag&, const FGameplayTag&, int32);
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag&, const FGameplayTag&, const FGameplayTag&, const FGameplayTag&);
DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /*AbilityTag*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FActivatePassiveEffect, const FGameplayTag&, bool);

USTRUCT()
struct FRSkillPurchaseRecord
{
	GENERATED_BODY()
	UPROPERTY() FGameplayTag Skill;
	UPROPERTY() int32 BaselineRank = 0;
	UPROPERTY() int32 Points = 0;
	UPROPERTY() int32 CorePoints = 0;
};
/**
 * 
 */
UCLASS()
class REPARATION_API URAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skills") TObjectPtr<class UAbilityInfo> SkillDefinitions;
	const UAbilityInfo* GetSkillData() const;
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Skills|Progression") int32 CoreSkillPointsSpent = 0;
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Skills|Progression") TArray<FName> ProgressionUnlocks;
	UFUNCTION(BlueprintPure, Category="Skills|Progression") bool IsSkillQuestEligible(FName UnlockId) const;
	/** Call from authoritative quest completion. This does not create or complete a quest. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Skills|Progression") void GrantProgressionUnlock(FName UnlockId);
	void PreserveDebuffPeriod(FActiveGameplayEffectHandle Handle, float NextTickDelay);
	
	void AbilityActorInfoSet();
	FEffectAssetTags EffectAssetTags;
	FAbilityGiven AbilityGivenDelegate;
	FAbilityStatusChange AbilityStatusChangeDelegate;
	FAbilityEquipped AbilityEquippedDelegate;
	FDeactivatePassiveAbility DeactivatePassiveAbilityDelegate;
	FActivatePassiveEffect ActivatePassiveEffectDelegate;
	
	void AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities);
	void AddPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& PassiveAbilities);
	bool bGrantedAbilitiesGiven = false;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void ForEachAbility(const FForEachAbility& Delegate);
	
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);
	FGameplayTag GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag);
	bool InputSlotIsEmpty(const FGameplayTag& InputSlot);
	static bool AbilityHasInputSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& InputSlot);
	static bool AbilityHasAnySlot(const FGameplayAbilitySpec& Spec);
	FGameplayAbilitySpec* GetSpecWithInputSlot(const FGameplayTag& InputSlot);
	bool IsPassiveAbility(const FGameplayAbilitySpec& Spec) const;
	static void AssignInputSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& InputSlot);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastActivatePassiveEffect(const FGameplayTag& AbilityTag, bool bActivate);
	
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
	
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);

	void UpdateAbilityStatus(int32 Level);
	UFUNCTION(BlueprintPure, Category="Skills") bool IsTierRewardEarned();
	void RefreshTierReward();
	/** Shared by the menu and authoritative point spending. An empty result means spending is allowed. */
	UFUNCTION(BlueprintPure, Category="Skills")
	FText GetSkillUpgradeIssue(const FGameplayTag& AbilityTag);
	UFUNCTION(BlueprintPure, Category="Skills")
	int32 GetLearnedSkillRank(const FGameplayTag& AbilityTag);
	void BuildSkillDescriptions(const FGameplayTag& AbilityTag, FString& Current, FString& Next);
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Skills") FGameplayTag SelectedMeleeElement;
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Skills")
	void ServerSelectMeleeElement(FGameplayTag DamageType);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerSpendAbilityPoint(const FGameplayTag& AbilityTag);

	UFUNCTION(Server, Reliable)
	void ServerQueueMeleeCombo(FGameplayAbilitySpecHandle Handle);

	UFUNCTION(Server, Reliable)
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);
	UFUNCTION(BlueprintPure, Category="Skills|Loadout") FText GetEquipIssue(FGameplayTag AbilityTag, FGameplayTag Slot);
	UFUNCTION(BlueprintPure, Category="Skills|Loadout") int32 GetPassiveSlotUnlockLevel(int32 Index) const;
	UFUNCTION(BlueprintPure, Category="Skills|Loadout") bool IsLoadoutSlotUnlocked(FGameplayTag Slot) const;
	UFUNCTION(Server, Reliable) void ServerUnequipAbility(FGameplayTag AbilityTag);
	void RefreshPassiveSlotLimits();
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Melee|Counter") void RecordSuccessfulBlock(AActor* Attacker);
	UFUNCTION(BlueprintPure, Category="Melee|Counter") AActor* GetCounterstrikeTarget() const;
	bool ConsumeCounterstrike();
	UPROPERTY(EditDefaultsOnly, Category="Melee|Counter", meta=(ClampMin="0.1")) float CounterstrikeWindow = 1.25f;
	UFUNCTION(BlueprintPure, Category="Skills|Progression") bool IsBranchTierUnlocked(ERSkillDiscipline Branch, int32 Tier, int32 Required = 1);
	UFUNCTION(BlueprintPure, Category="Skills|Respec") FText GetBranchRespecIssue(ERSkillDiscipline Branch);
	UFUNCTION(BlueprintPure, Category="Skills|Respec") int32 GetBranchRefund(ERSkillDiscipline Branch) const;
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Skills|Respec") void ServerRespecBranch(ERSkillDiscipline Branch);
	/** Exact paid amounts, rather than rank times current tuning; no refunds for free grants. Session-only until progression saving exists. */
	UPROPERTY(Replicated) TArray<FRSkillPurchaseRecord> SkillPurchases;

	UFUNCTION(Client, Reliable)
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot);

	bool GetDescriptionByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);

	static void ClearSlot(FGameplayAbilitySpec* Spec);
	void ClearAbilityFromSlot(const FGameplayTag& Slot);
	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot);
	
protected:
	TWeakObjectPtr<AActor> CounterstrikeAttacker;
	double CounterstrikeExpires = -1;

	virtual void OnRep_ActivateAbilities() override;

	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
	
	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);
};
