// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RAbilitySystemComponent.generated.h"

//class UAOAbilityComp;
struct FGameplayTag;
struct FGameplayAbilitySpec;

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer&);
DECLARE_MULTICAST_DELEGATE(FAbilityGiven);
DECLARE_DELEGATE_OneParam(FCheckForAbility, const FGameplayAbilitySpec&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChange, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, int32 /*AbilityLevel*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityActivated, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, const FGameplayTag& /*Slot*/, const FGameplayTag& /*PrevSlot*/);
/**
 * 
 */
UCLASS()
class REPARATION_API URAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	void AbilityActorInfoInit();

	FEffectAssetTags EffectTags;
	//FAbilityGiven AbilityGivenDelegate;
	//FAbilityStatusChange AbilityStatusChange;
	//FAbilityActivated AbilityActivated;

	//bool bAbilityGranted = false;

	void AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities);
	//void AddPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& PassiveAbilities);
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
//	void CheckForAbility(const FCheckForAbility& Delegate);
//
//	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
//	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
//	static FGameplayTag GetStatusTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
//	FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);
//	FGameplayTag GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag);
//
//	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
//
	void UpgradeAttribute(const FGameplayTag& AttributeTag) const;
//
//	void UpdateAbilityStatus(int32 Level);
//
//	void SpendAbilityPoint(const FGameplayTag& AbilityTag);
//
//	void ActivateAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);
//	//void BroadcastActivatedAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot);
//
//	void ClearInputSlot(FGameplayAbilitySpec* Spec);
//
//	void ClearInputTagFromSlottedAbility(const FGameplayTag& Slot);
//
//	bool GetDescriptionsByAbilityType(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);
//	static bool AbilityHasInputSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot);
//
protected:
	
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* ASComp, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle) const;

	//void AbilityStatusUpdate(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);
	
};
