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

UCLASS()
class REPARATION_API UAOAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UAOAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;
	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldEnergy) const;
	UFUNCTION()
	void OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const;

protected:

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

};
