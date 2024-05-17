// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RAttributeSet.generated.h"

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

	virtual void GetLifetimeReplicatedProps( TArray<class FLifetimeProperty> & OutLifetimeProps ) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	//Attribute Map
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> AttributeTagMap;

private:

	//void HandleDamage(const FEffectProperties& Props);
	void HandleXP(const FEffectProperties& Props);
	void HandleDebuff(const FEffectProperties& Props);
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;
	void ShowFloatingText(const FEffectProperties& Props, float DamageAmount, bool bBlockedHit, bool bCriticalHit) const;
	void SendXPEvent(const FEffectProperties& Props);
	bool bRefillHealth = false;
	bool bRefillStamina = false;
	bool bRefillEnergy = false;
	


public:

	//Base Attributes-----------------------
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Base Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Base Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Energy, Category = "Base Attributes")
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Energy)

	/*
	 * Meta Attributes
	 */

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Damage);

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData XP;
	ATTRIBUTE_ACCESSORS(URAttributeSet, XP);
	

	//RPG Attributes-----------------------------

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

	//Secondary Attributes------------------------
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

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stealth, Category = "Attributes")
	FGameplayAttributeData Stealth;
	ATTRIBUTE_ACCESSORS(URAttributeSet, Stealth)

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

	//Resistances
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes")
	FGameplayAttributeData PhysicalResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, PhysicalResistance);

	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes")
	FGameplayAttributeData PoisonResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, PoisonResistance);

	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes")
	FGameplayAttributeData DarkResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, DarkResistance);

	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes")
	FGameplayAttributeData ElectricResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, ElectricResistance);

	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes")
	FGameplayAttributeData FireResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, FireResistance);

	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes")
	FGameplayAttributeData IceResistance;
	ATTRIBUTE_ACCESSORS(URAttributeSet, IceResistance);

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;
	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldEnergy) const;
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
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;
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
	void OnRep_Stealth(const FGameplayAttributeData& OldStealth) const;
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
	
};
