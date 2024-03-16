// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

//Singleton  containing Native Gameplay Tags
struct FAOGameplayTags
{

public:

	static const FAOGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	//GameplayTags
	FGameplayTag Attributes_Primary_Agility;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Perception;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Vigor;

	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_DodgeChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_Stealth;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxStamina;
	FGameplayTag Attributes_Secondary_MaxEnergy;
	FGameplayTag Attributes_Secondary_HealthRegen;
	FGameplayTag Attributes_Secondary_StaminaRegen;
	FGameplayTag Attributes_Secondary_EnergyRegen;

	//Input
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;

	//Resistances
	FGameplayTag Attributes_Resistance_Physical;
	FGameplayTag Attributes_Resistance_Poison;
	FGameplayTag Attributes_Resistance_Dark;
	FGameplayTag Attributes_Resistance_Electric;
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Ice;
	

	//Damage
	FGameplayTag Damage;

	FGameplayTag Damage_Physical;
	FGameplayTag Damage_Poison;
	FGameplayTag Damage_Elemental_Dark;
	FGameplayTag Damage_Elemental_Electric;
	FGameplayTag Damage_Elemental_Fire;
	FGameplayTag Damage_Elemental_Ice;

	TMap<FGameplayTag, FGameplayTag> DamageTypeToResist;

	//Abilities
	FGameplayTag Ability_Melee;

	FGameplayTag Montage_Attack_Weapon;
	FGameplayTag Montage_Attack_RHand;
	FGameplayTag Montage_Attack_LHand;

	//Effects
	FGameplayTag Effects_HitReact;

private:

	static FAOGameplayTags GameplayTags;

};