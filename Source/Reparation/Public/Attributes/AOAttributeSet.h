// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AOAttributeSet.generated.h"


	// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


USTRUCT(BlueprintType)
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectContextHandle ContextHandle;
	//Source
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAbilitySystemComponent* SourceASC = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APlayerController* SourcePC = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACharacter* SourceCharacter = nullptr;

	//Target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AController* TargetController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACharacter* TargetCharacter = nullptr;

	
};

//typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

UCLASS()
class REPARATION_API UAOAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UAOAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> AttributeTagMap;


private:

	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;
	void ShowFloatingText(const FEffectProperties& Props, float Damage) const;

public:
	//Base Attributes-----------------------
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Base Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Base Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Energy, Category = "Base Attributes")
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Energy)

	/*
	 * Meta Attributes
	 */

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Damage);

	//RPG Attributes-----------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Agility)

	//Stealth will decrease AI sight
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Intelligence)

	//Resistance will decrease damage - various types handled by gameplay tags
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Perception;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Perception)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Resilience)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Strength)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Vigor)

	//Secondary Attributes------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Armor)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, ArmorPenetration)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, BlockChance)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData DodgeChance;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, DodgeChance)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Stealth;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Stealth)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CritHitChance;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, CritHitChance)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CritHitDamage;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, CritHitDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CritHitResistance;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, CritHitResistance)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Base Attributes")
	FGameplayAttributeData MaxHealth = 0.0f;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Base Attributes")
	FGameplayAttributeData MaxStamina = 0.0f;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxEnergy, Category = "Base Attributes")
	FGameplayAttributeData MaxEnergy = 0.0f;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, MaxEnergy)
	
	UPROPERTY(BlueprintReadOnly, Category = "Base Attributes")
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, HealthRegen)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData StaminaRegen;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, StaminaRegen)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData EnergyRegen;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, EnergyRegen)

	//---------------------OnRep
	//Base
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;
	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldEnergy) const;
	
	//Primary
	UFUNCTION()
	void OnRep_Agility(const FGameplayAttributeData& OldAgility) const;
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
	UFUNCTION()
	void OnRep_Perception(const FGameplayAttributeData& OldPerception) const;
	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

	//Secondary

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;
	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;
	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;
	UFUNCTION()
	void OnRep_DodgeChance(const FGameplayAttributeData& OldDodgeChance) const;
	UFUNCTION()
	void OnRep_CritHitChance(const FGameplayAttributeData& OldCritHitChance) const;
	UFUNCTION()
	void OnRep_CritHitDamage(const FGameplayAttributeData& OldCritHitDamage) const;
	UFUNCTION()
	void OnRep_CritHitResistance(const FGameplayAttributeData& OldCritHitResistance) const;
	UFUNCTION()
	void OnRep_Stealth(const FGameplayAttributeData& OldStealth) const;
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;
	UFUNCTION()
	void OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const;
	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) const;
	UFUNCTION()
	void OnRep_StaminaRegen(const FGameplayAttributeData& OldStaminaRegen) const;
	UFUNCTION()
	void OnRep_EnergyRegen(const FGameplayAttributeData& OldEnergyRegen) const;

};
