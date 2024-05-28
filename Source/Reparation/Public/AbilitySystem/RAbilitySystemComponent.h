// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RAbilitySystemComponent.generated.h"

struct FGameplayTag;
struct FGameplayAbilitySpec;

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer&);
/**
 * 
 */
UCLASS()
class REPARATION_API URAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	void AbilityActorInfoSet();

	FEffectAssetTags EffectTags;

	void AddGrantedAbilities(const TArray<TSubclassOf<UGameplayAbility>>& GrantedAbilities);
	
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

protected:
	
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* ASComp, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle EffectHandle) const;
	
};
