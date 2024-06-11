// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "RAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DELEGATE_RetVal(FGameplayAttribute, FAttributeSignature);

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(){}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

/**
 * 
 */
UCLASS()
class REPARATION_API URAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	URAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
	
private:
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;
	void ShowFloatingText(const FEffectProperties& Props, float DamageAmount, bool bBlockedHit, bool bDodgedHit, bool bCriticalHit) const;

public:

	//Vital Attributes-----------------------
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Base Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Base Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Energy, Category = "Base Attributes")
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Energy)

	//Meta Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Damage)
	

	//Primary Attributes
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Agility, Category = "Attributes")
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Agility)
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Intelligence)
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Perception, Category = "Attributes")
	FGameplayAttributeData Perception;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Perception)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Resilience, Category = "Attributes")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Resilience)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Strength)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Vigor)
	
	//Secondary Attributes
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Base Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Base Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(URAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxEnergy, Category = "Base Attributes")
	FGameplayAttributeData MaxEnergy;
	ATTRIBUTE_ACCESSORS(URAttributeSet, MaxEnergy)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Armor)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArmorPenetration, Category = "Attributes")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(URAttributeSet, ArmorPenetration)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BlockChance, Category = "Attributes")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, BlockChance)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DodgeChance, Category = "Attributes")
	FGameplayAttributeData DodgeChance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, DodgeChance)
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritHitChance, Category = "Attributes")
	FGameplayAttributeData CritHitChance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, CritHitChance)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritHitDamage, Category = "Attributes")
	FGameplayAttributeData CritHitDamage;
	ATTRIBUTE_ACCESSORS(URAttributeSet, CritHitDamage)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritHitResistance, Category = "Attributes")
	FGameplayAttributeData CritHitResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, CritHitResistance)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegen, Category = "Base Attributes")
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(URAttributeSet, HealthRegen)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaminaRegen, Category = "Attributes")
	FGameplayAttributeData StaminaRegen;
	ATTRIBUTE_ACCESSORS(URAttributeSet, StaminaRegen)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EnergyRegen, Category = "Attributes")
	FGameplayAttributeData EnergyRegen;
	ATTRIBUTE_ACCESSORS(URAttributeSet, EnergyRegen)
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stealth, Category = "Attributes")
	FGameplayAttributeData Stealth;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Stealth)
	
	//Resistances
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PhysicalResistance, Category = "Resistance Attributes")
	FGameplayAttributeData PhysicalResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, PhysicalResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PoisonResistance, Category = "Resistance Attributes")
	FGameplayAttributeData PoisonResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, PoisonResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DarkResistance, Category = "Resistance Attributes")
	FGameplayAttributeData DarkResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, DarkResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ElectricResistance, Category = "Resistance Attributes")
	FGameplayAttributeData ElectricResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, ElectricResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FireResistance, Category = "Resistance Attributes")
	FGameplayAttributeData FireResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, FireResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IceResistance, Category = "Resistance Attributes")
	FGameplayAttributeData IceResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, IceResistance);


	//Vital
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;

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
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;
	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldEnergy) const;
	UFUNCTION()
	void OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const;
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
	void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) const;
	UFUNCTION()
	void OnRep_StaminaRegen(const FGameplayAttributeData& OldStaminaRegen) const;
	UFUNCTION()
	void OnRep_EnergyRegen(const FGameplayAttributeData& OldEnergyRegen) const;
	UFUNCTION()
	void OnRep_Stealth(const FGameplayAttributeData& OldStealth) const;

	//Resistances
	UFUNCTION()
	void OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const;
	UFUNCTION()
	void OnRep_PoisonResistance(const FGameplayAttributeData& OldPoisonResistance) const;
	UFUNCTION()
	void OnRep_DarkResistance(const FGameplayAttributeData& OldDarkResistance) const;
	UFUNCTION()
	void OnRep_ElectricResistance(const FGameplayAttributeData& OldElectricResistance) const;
	UFUNCTION()
	void OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const;
	UFUNCTION()
	void OnRep_IceResistance(const FGameplayAttributeData& OldIceResistance) const;
};
