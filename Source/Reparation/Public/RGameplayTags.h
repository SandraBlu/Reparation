// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	//Attributes
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
	FGameplayTag InputTag_Rage;
	FGameplayTag InputTag_Passive01;
	FGameplayTag InputTag_Passive02;
	FGameplayTag InputTag_Passive03;
	FGameplayTag InputTag_Passive04;
	FGameplayTag InputTag_Special_Light;
	FGameplayTag InputTag_Special_Heavy;
	//Block
	FGameplayTag InputTag_MustBeHeld;
	FGameplayTag InputTag_MustBeHeld_Blocked;
	//Targeting
	FGameplayTag InputTag_Toggle;
	FGameplayTag InputTag_Toggle_TargetLock;
	
	
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
	FGameplayTag Debuff_Dark;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Freeze;

	FGameplayTag Debuff_Damage;
	FGameplayTag Debuff_Chance;
	FGameplayTag Debuff_Frequency;
	FGameplayTag Debuff_Duration;

	//Abilities NPC
	FGameplayTag ability_none;
	FGameplayTag ability_Attack_Melee;
	FGameplayTag ability_Attack_Ranged;

	//Abilities Player
	FGameplayTag ability_Melee;
	FGameplayTag ability_Melee_Light;
	FGameplayTag ability_Melee_Heavy;
	FGameplayTag ability_Melee_Thrust;
	FGameplayTag ability_Ranged;
	FGameplayTag ability_HitReact;
	FGameplayTag ability_HitPause;
	FGameplayTag ability_block;
	FGameplayTag ability_death;
	FGameplayTag ability_evade;
	FGameplayTag ability_targetLock;
	FGameplayTag ability_stunned;
	FGameplayTag ability_missile_dark;
	FGameplayTag ability_missile_electric;
	FGameplayTag ability_missile_fire;
	FGameplayTag ability_missile_ice;
	FGameplayTag ability_dart_poison;
	FGameplayTag ability_dart_knockout;
	FGameplayTag ability_grenade_stun;
	FGameplayTag ability_grenade_fireBlast;
	FGameplayTag ability_grenade_iceBlast;
	FGameplayTag ability_summonEnemies;
	FGameplayTag ability_rage;
	FGameplayTag ability_spawn_pickup;

	//Ability Menu Tags
	FGameplayTag ability_status_locked;
	FGameplayTag ability_status_unlocked;
	FGameplayTag ability_status_available;
	FGameplayTag ability_status_equipped;

	FGameplayTag ability_type_dynamic;
	FGameplayTag ability_type_passive;
	FGameplayTag ability_type_none;

	//Passive Abilities
	FGameplayTag ability_passive_protectiveCloak;
	FGameplayTag ability_passive_lifeSiphon;
	FGameplayTag ability_passive_lifeForce;
	FGameplayTag ability_passive_healthRegen;
	FGameplayTag ability_passive_staminaSiphon;
	FGameplayTag ability_passive_staminaRegen;
	
	//cooldown
	FGameplayTag cooldown_Melee;
	FGameplayTag cooldown_melee_light;
	FGameplayTag cooldown_melee_heavy;
	FGameplayTag cooldown_missile_dark;
	FGameplayTag cooldown_missile_electric;
	FGameplayTag cooldown_missile_fire;
	FGameplayTag cooldown_missile_ice;
	FGameplayTag cooldown_dart_poison;
	FGameplayTag cooldown_dart_knockout;
	FGameplayTag cooldown_grenade_stun;
	FGameplayTag cooldown_grenade_fireBlast;
	FGameplayTag cooldown_grenade_iceBlast;

	//Weapons
	FGameplayTag weapon_melee_katana;
	FGameplayTag weapon_melee_staff;
	FGameplayTag weapon_throwable;
	FGameplayTag weapon_crossbow;
	
	//Combat Socket
	FGameplayTag combatSocket_weapon;
	FGameplayTag combatSocket_handL;
	FGameplayTag combatSocket_handR;
	FGameplayTag combatSocket_jaw;

	//Attack Montages
	FGameplayTag Montage_Attack01;
	FGameplayTag Montage_Attack02;
	FGameplayTag Montage_Attack03;
	FGameplayTag Montage_Attack04;

	//Events
	FGameplayTag Event_EquipKatana;
	FGameplayTag Event_UnequipKatana;
	FGameplayTag Event_HitMelee;
	FGameplayTag Event_HitPause;
	FGameplayTag Event_Death;
	FGameplayTag Event_Strafing;
	FGameplayTag Event_Blocking_Successful;
	FGameplayTag Event_Blocking_Unblockable;
	FGameplayTag Event_SwitchTarget_Left;
	FGameplayTag Event_SwitchTarget_Right;
	FGameplayTag Event_SummonEnemies;
	FGameplayTag Event_Rage_Activate;
	
	//HitReact Events
	FGameplayTag Event_HitReact;
	FGameplayTag Event_HitReact_Front;
	FGameplayTag Event_HitReact_Left;
	FGameplayTag Event_HitReact_Right;
	FGameplayTag Event_HitReact_Back;

	//status
	FGameplayTag status_evading;
	FGameplayTag status_blocking;
	FGameplayTag status_targeting;
	FGameplayTag status_invincible;
	//Rage Event Tags
	FGameplayTag status_Rage_Activating;
	FGameplayTag status_Rage_Active;
	FGameplayTag status_Rage_Full;
	FGameplayTag status_Rage_None;
	
	
private:

	static FRGameplayTags GameplayTags;
};
