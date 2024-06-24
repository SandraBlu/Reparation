// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);
DECLARE_MULTICAST_DELEGATE(FAbilityGiven);
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChange, const FGameplayTag&, const FGameplayTag&, int32);
/**
 * 
 */
UCLASS()
class REPARATION_API URAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	
	void AbilityActorInfoSet();
	FEffectAssetTags EffectAssetTags;
	FAbilityGiven AbilityGivenDelegate;
	FAbilityStatusChange AbilityStatusChangeDelegate;
	
	void AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities);
	void AddPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& PassiveAbilities);
	bool bGrantedAbilitiesGiven = false;

	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void ForEachAbility(const FForEachAbility& Delegate);
	
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
	
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);

	void UpdateAbilityStatus(int32 Level);

	UFUNCTION(Server, Reliable)
	void ServerSpendAbilityPoint(const FGameplayTag& AbilityTag);
	
protected:

	virtual void OnRep_ActivateAbilities() override;

	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
	
	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);
};
