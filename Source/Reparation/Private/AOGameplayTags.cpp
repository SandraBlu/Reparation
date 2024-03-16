// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameplayTags.h"
#include "GameplayTagsManager.h"

FAOGameplayTags FAOGameplayTags::GameplayTags;

void FAOGameplayTags::InitializeNativeGameplayTags()
{
	//primary attributes
	GameplayTags.Attributes_Primary_Agility = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Agility"), FString("Improves dodge and stealth skills"));
	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Intelligence"), FString("Improves Energy tech damage"));
	GameplayTags.Attributes_Primary_Perception = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Perception"), FString("Allows you to see AI better, improves stealth"));
	GameplayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Resilience"), FString("Increases Armor and Armor Penetration"));
	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Strength"), FString("Increases  physical damage"));
	GameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Vigor"), FString("Increases health"));

	//secondary attributes
	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.Armor"), FString("Reduces health damage"));
	GameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.ArmorPenetration"), FString("Increases damage to enemy"));
	GameplayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.BlockChance"), FString("Decreases damage to self"));
	GameplayTags.Attributes_Secondary_DodgeChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.DodgeChance"), FString("Negates damage"));
	GameplayTags.Attributes_Secondary_Stealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.Stealth"), FString("Negates AI perception"));
	GameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitChance"), FString("Increases damage to enemy"));
	GameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitDamage"), FString("Increases hit damage to enemy"));
	GameplayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitResistance"), FString("Decreases damage to self"));
	GameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxHealth"), FString("Clamps Health"));
	GameplayTags.Attributes_Secondary_MaxStamina = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxStamina"), FString("Clamps Stamina"));
	GameplayTags.Attributes_Secondary_MaxEnergy = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxEnergy"), FString("Clamps Energy"));
	GameplayTags.Attributes_Secondary_HealthRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.HealthRegen"), FString("Rate of Health Regeneration"));
	GameplayTags.Attributes_Secondary_StaminaRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.StaminaRegen"), FString("Rate of Stamina Regeneration"));
	GameplayTags.Attributes_Secondary_EnergyRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.EnergyRegen"), FString("Rate of Energy Regeneration"));

	//Input Tags
	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.LMB"), FString("LMB"));
	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.RMB"), FString("RMB"));
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.1"), FString("1"));
	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.2"), FString("2"));
	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.3"), FString("3"));
	GameplayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.4"), FString("4"));

	//Damage Meta Att
	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage"),FString("Damage"));
	GameplayTags.Damage_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Physical"), FString("Physical Damage"));
	GameplayTags.Damage_Poison = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Poison"), FString("Poison Damage"));
	GameplayTags.Damage_Elemental_Dark = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Elemental.Dark"), FString("Dark Damage"));
	GameplayTags.Damage_Elemental_Electric = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Elemental.Electric"), FString("Electric Damage"));
	GameplayTags.Damage_Elemental_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Elemental.Fire"), FString("Fire Damage"));
	GameplayTags.Damage_Elemental_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Elemental.Ice"), FString("Ice Damage"));

	//Resistance Tags
	GameplayTags.Attributes_Resistance_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Physical"),FString("Resistance to Physical damage"));
	GameplayTags.Attributes_Resistance_Poison = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Poison"), FString("Resistance to Poison damage"));
	GameplayTags.Attributes_Resistance_Dark = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Dark"), FString("Resistance to Dark damage"));
	GameplayTags.Attributes_Resistance_Electric = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Electric"), FString("Resistance to Electric damage"));
	GameplayTags.Attributes_Resistance_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Fire"), FString("Resistance to Fire damage"));
	GameplayTags.Attributes_Resistance_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Ice"), FString("Resistance to Cold damage"));
	
	//Ability
	GameplayTags.Ability_Melee = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Melee"), FString("Melee Attack"));

	//Montage
	GameplayTags.Montage_Attack_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.Weapon"), FString("Weapon"));
	GameplayTags.Montage_Attack_RHand = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.RHand"), FString("RHand"));
	GameplayTags.Montage_Attack_LHand = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.LHand"), FString("LHand"));


	/*
	 * Map of Damage Types to Resistances
	 */
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Poison, GameplayTags.Attributes_Resistance_Poison);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Elemental_Dark, GameplayTags.Attributes_Resistance_Dark);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Elemental_Electric, GameplayTags.Attributes_Resistance_Electric);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Elemental_Fire, GameplayTags.Attributes_Resistance_Fire);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Elemental_Ice, GameplayTags.Attributes_Resistance_Ice);

	//Effects
	GameplayTags.Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Effect.HitReact"), FString("Hit React Tag"));
}