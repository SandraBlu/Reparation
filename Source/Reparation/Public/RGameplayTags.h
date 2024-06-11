// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * 
 */
 //Singleton  containing Native Gameplay Tags
struct FRGameplayTags
{

public:

	static const FRGameplayTags& Get() { return GameplayTags; }
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
	FGameplayTag InputTag_Passive01;
	FGameplayTag InputTag_Passive02;
	FGameplayTag InputTag_Passive03;

	//meta
	FGameplayTag Attributes_meta_xp;
	FGameplayTag Damage;

	FGameplayTag Damage_Physical;
	FGameplayTag Damage_Poison;
	FGameplayTag Damage_Dark;
	FGameplayTag Damage_Elemental_Electric;
	FGameplayTag Damage_Elemental_Fire;
	FGameplayTag Damage_Elemental_Ice;

	TMap<FGameplayTag, FGameplayTag> DamageTypeToResist;
	TMap<FGameplayTag, FGameplayTag> DamageTypeToDebuff;

	//Resistances
	FGameplayTag Attributes_Resistance_Physical;
	FGameplayTag Attributes_Resistance_Poison;
	FGameplayTag Attributes_Resistance_Dark;
	FGameplayTag Attributes_Resistance_Electric;
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Ice;

	//De-buff
	FGameplayTag Debuff_KnockOut;
	FGameplayTag Debuff_Paralyze;
	FGameplayTag Debuff_Dark;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Freeze;

	FGameplayTag Debuff_Damage;
	FGameplayTag Debuff_Chance;
	FGameplayTag Debuff_Frequency;
	FGameplayTag Debuff_Duration;

	//Abilities
	FGameplayTag ability_none;
	FGameplayTag ability_Attack;
	FGameplayTag ability_Melee;
	FGameplayTag ability_HitReact;
	FGameplayTag ability_missile_dark;
	FGameplayTag ability_missile_electric;
	FGameplayTag ability_missile_fire;
	FGameplayTag ability_missile_ice;
	FGameplayTag ability_dart_poison;
	FGameplayTag ability_dart_knockout;

	//Ability Menu Tags
	FGameplayTag ability_status_locked;
	FGameplayTag ability_status_unlocked;
	FGameplayTag ability_status_available;
	FGameplayTag ability_status_active;

	FGameplayTag ability_type_dynamic;
	FGameplayTag ability_type_passive;
	FGameplayTag ability_type_none;

	
	//cooldown
	FGameplayTag cooldown_Melee;
	FGameplayTag cooldown_missile_dark;
	FGameplayTag cooldown_missile_electric;
	FGameplayTag cooldown_missile_fire;
	FGameplayTag cooldown_missile_ice;
	FGameplayTag cooldown_dart_poison;
	FGameplayTag cooldown_dart_knockout;


	//Combat SocketKO
	FGameplayTag combatSocket_weapon;
	FGameplayTag combatSocket_handL;
	FGameplayTag combatSocket_handR;

	//Attack Montages
	FGameplayTag Montage_Attack01;
	FGameplayTag Montage_Attack02;
	FGameplayTag Montage_Attack03;
	FGameplayTag Montage_Attack04;

private:

	static FRGameplayTags GameplayTags;
};
