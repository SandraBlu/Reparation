// Fill out your copyright notice in the Description page of Project Settings.


#include "RGameplayTags.h"
#include "GameplayTagsManager.h"

FRGameplayTags FRGameplayTags::GameplayTags;


void FRGameplayTags::InitializeNativeGameplayTags()
{
	//primary attributes
	GameplayTags.Attributes_Primary_Agility = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.primary.agility"), FString("Improves dodge and stealth skills"));
	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.primary.intelligence"), FString("Improves Energy tech damage"));
	GameplayTags.Attributes_Primary_Perception = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.primary.perception"), FString("Allows you to see AI better, improves stealth"));
	GameplayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.primary.resilience"), FString("Increases Armor and Armor Penetration"));
	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.primary.strength"), FString("Increases  physical damage"));
	GameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.primary.vigor"), FString("Increases health"));

	//secondary attributes
	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.armor"), FString("Reduces health damage"));
	GameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.armorPenetration"), FString("Increases damage to enemy"));
	GameplayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.blockChance"), FString("Decreases damage to self"));
	GameplayTags.Attributes_Secondary_DodgeChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.dodgeChance"), FString("Negates damage"));
	GameplayTags.Attributes_Secondary_Stealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.stealth"), FString("Negates AI perception"));
	GameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.criticalHitChance"), FString("Increases damage to enemy"));
	GameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.criticalHitDamage"), FString("Increases hit damage to enemy"));
	GameplayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.criticalHitResistance"), FString("Decreases damage to self"));
	GameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.maxHealth"), FString("Clamps Health"));
	GameplayTags.Attributes_Secondary_MaxStamina = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.maxStamina"), FString("Clamps Stamina"));
	GameplayTags.Attributes_Secondary_MaxEnergy = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.maxEnergy"), FString("Clamps Energy"));
	GameplayTags.Attributes_Secondary_HealthRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.healthRegen"), FString("Rate of Health Regeneration"));
	GameplayTags.Attributes_Secondary_StaminaRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.staminaRegen"), FString("Rate of Stamina Regeneration"));
	GameplayTags.Attributes_Secondary_EnergyRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.secondary.energyRegen"), FString("Rate of Energy Regeneration"));

	GameplayTags.Attributes_meta_xp = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.meta.xp"), FString("XP Meta Attribute"));

	//Input Tags
	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.LMB"), FString("LMB"));
	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.RMB"), FString("RMB"));
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.1"), FString("1"));
	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.2"), FString("2"));
	GameplayTags.InputTag_Passive01 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.passive01"), FString("Passive01"));
	GameplayTags.InputTag_Passive02 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.passive02"), FString("Passive02"));
	GameplayTags.InputTag_Passive03 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.passive03"), FString("Passive03"));
	GameplayTags.InputTag_Passive04 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.passive04"), FString("Passive04"));
	GameplayTags.InputTag_MustBeHeld = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.MustBeHeld"), FString("HeldInput"));
	GameplayTags.InputTag_MustBeHeld_Blocked= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.block.MustBeHeld"), FString("BlockInput"));
	GameplayTags.InputTag_Toggle = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.toggle"), FString(" Toggle Input"));
	GameplayTags.InputTag_Toggle_TargetLock = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.toggle.targetLock"), FString("Target Lock"));
	//GameplayTags.InputTag_SwitchTarget= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("inputTag.switchTarget"), FString("Switch Target"));
	
	//Damage Meta Att
	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("damage"), FString("Damage"));
	GameplayTags.Damage_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("damage.physical"), FString("Physical Damage"));
	GameplayTags.Damage_Poison = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("damage.poison"), FString("Poison Damage"));
	GameplayTags.Damage_Dark = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("damage.dark"), FString("Dark Damage"));
	GameplayTags.Damage_Elemental_Electric = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("damage.elemental.electric"), FString("Electric Damage"));
	GameplayTags.Damage_Elemental_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("damage.elemental.fire"), FString("Fire Damage"));
	GameplayTags.Damage_Elemental_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("damage.elemental.ice"), FString("Ice Damage"));
	
	//Resistance Tags
	GameplayTags.Attributes_Resistance_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.resistance.physical"), FString("Resistance to Physical damage"));
	GameplayTags.Attributes_Resistance_Poison = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.resistance.poison"), FString("Resistance to Poison damage"));
	GameplayTags.Attributes_Resistance_Dark = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.resistance.dark"), FString("Resistance to Dark damage"));
	GameplayTags.Attributes_Resistance_Electric = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.resistance.electric"), FString("Resistance to Electric damage"));
	GameplayTags.Attributes_Resistance_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.resistance.fire"), FString("Resistance to Fire damage"));
	GameplayTags.Attributes_Resistance_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("attributes.resistance.ice"), FString("Resistance to Cold damage"));

	//De-buffs
	GameplayTags.Debuff_KnockOut = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.knockout"), FString("De-buff to Physical or Poison damage"));
	GameplayTags.Debuff_Dark = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.dark"), FString("De-buff to Dark Damage"));
	GameplayTags.Debuff_Stun = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.stun"), FString("De-buff to Electric damage"));
	GameplayTags.Debuff_Burn = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.burn"), FString("De-buff to fire damage"));
	GameplayTags.Debuff_Freeze = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.freeze"), FString("De-buff to ice damage"));

	GameplayTags.Debuff_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.damage"), FString("De-buff  Damage"));
	GameplayTags.Debuff_Chance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.chance"), FString("De-buff chance"));
	GameplayTags.Debuff_Frequency = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.frequency"), FString("De-buff Frequency"));
	GameplayTags.Debuff_Duration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("debuff.duration"), FString("De-buff duration"));

	// 
	//Ability
	GameplayTags.ability_none = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.none"), FString("No Ability"));
	GameplayTags.ability_Attack_Melee = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.attack.melee"), FString("Melee attack tag for Behavior Tree"));
	GameplayTags.ability_Attack_Ranged = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.attack.ranged"), FString("Ranged attack tag for Behavior Tree"));
	GameplayTags.ability_Melee = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.melee"), FString("Melee Attack tag"));
	GameplayTags.ability_Ranged = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.ranged"), FString("Ranger Attack tag"));
	GameplayTags.ability_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.HitReact"), FString("Hit React tag"));
	GameplayTags.ability_HitPause = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.HitPause"), FString("Hit Pause tag"));
	GameplayTags.ability_block = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.block"), FString("Block tag"));
	GameplayTags.ability_death = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.death"), FString("Dead tag"));
	GameplayTags.ability_targetLock= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.targetLock"), FString("Target Lock"));
	GameplayTags.ability_evade = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.evade"), FString("Evade tag"));
	GameplayTags.ability_stunned = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.stunned"), FString("Stunned tag"));
	GameplayTags.ability_missile_dark = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.missile.dark"), FString("Dark Missile tag"));
	GameplayTags.ability_missile_electric = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.missile.electric"), FString("Electric Missile tag"));
	GameplayTags.ability_missile_fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.missile.fire"), FString("Fire Missile tag"));
	GameplayTags.ability_missile_ice = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.missile.ice"), FString("Ice Missile tag"));
	GameplayTags.ability_dart_poison = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.dart.poison"), FString("Poison Dart tag"));
	GameplayTags.ability_dart_knockout = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.dart.knockout"), FString("Knockout Dart tag"));
	GameplayTags.ability_grenade_stun = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.grenade.stun"), FString("Stun grenade"));
	GameplayTags.ability_grenade_fireBlast = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.grenade.fireBlast"), FString("Fire Blast grenade"));
	GameplayTags.ability_grenade_iceBlast = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.grenade.iceBlast"), FString("Ice Blast Grenade"));

	//Ability Menu Tags
	GameplayTags.ability_status_locked = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.status.locked"), FString("Locked Status"));
	GameplayTags.ability_status_unlocked = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.status.unlocked"), FString("Unlocked Status"));
	GameplayTags.ability_status_available = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.status.available"), FString("Available Status"));
	GameplayTags.ability_status_equipped = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.status.equipped"), FString("Equipped Status"));

	GameplayTags.ability_type_dynamic = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.type.dynamic"), FString("Dynamic"));
	GameplayTags.ability_type_passive = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.type.passive"), FString("Passive"));
	GameplayTags.ability_type_none = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.type.none"), FString("None"));

	//PassiveAbiilties
	GameplayTags.ability_passive_protectiveCloak= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.passive.protectiveCloak"), FString("Protective Cloak"));
	GameplayTags.ability_passive_lifeSiphon= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.passive.lifeSiphon"), FString("Life Siphon"));
	GameplayTags.ability_passive_lifeForce= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.passive.lifeForce"), FString("Life Force"));
	GameplayTags.ability_passive_healthRegen= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.passive.healthRegen"), FString("Health Regen"));
	GameplayTags.ability_passive_staminaSiphon= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.passive.staminaSiphon"), FString("Stamina Siphon"));
	GameplayTags.ability_passive_staminaRegen= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("ability.passive.staminaRegen"), FString("Stamina Regen"));

	//cooldown//
	GameplayTags.cooldown_Melee = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.melee"), FString("Melee cooldown tag"));
	GameplayTags.cooldown_missile_dark = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.missile.dark"), FString("Dark Missile cooldown tag"));
	GameplayTags.cooldown_missile_electric = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.missile.electric"), FString("Electric Missile cooldown tag"));
	GameplayTags.cooldown_missile_fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.missile.fire"), FString("Fire Missile cooldown tag"));
	GameplayTags.cooldown_missile_ice = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.missile.ice"), FString("Ice Missile cooldown tag"));
	GameplayTags.cooldown_dart_poison = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.dart.poison"), FString("Poison Dart cooldown tag"));
	GameplayTags.cooldown_dart_knockout = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.dart.knockout"), FString("Knockout Dart cooldown tag"));
	GameplayTags.cooldown_grenade_stun = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.grenade.stun"), FString("Knockout grenade cooldown tag"));
	GameplayTags.cooldown_grenade_fireBlast = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.grenade.fireBlast"), FString("Fire Blast grenade cooldown tag"));
	GameplayTags.cooldown_grenade_iceBlast = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("cooldown.grenade.iceBlast"), FString("Ice Blast grenade cooldown tag"));

	//Weapons
	GameplayTags.weapon_melee_katana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("weapon.katana"), FString("katana"));
	GameplayTags.weapon_melee_staff = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("weapon.staff"), FString("staff"));
	GameplayTags.weapon_throwable = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("weapon.throwable"), FString("throwable"));
	GameplayTags.weapon_crossbow = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("weapon.crossbow"), FString("crossbow"));
	
	//Sockets
	GameplayTags.combatSocket_weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("combatSocket.weapon"), FString("weapon"));
	GameplayTags.combatSocket_handL = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("combatSocket.handR"), FString("handR"));
	GameplayTags.combatSocket_handR = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("combatSocket.handL"), FString("handL"));

	//Montages
	GameplayTags.Montage_Attack01 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("montage.attack01"), FString("attack01"));
	GameplayTags.Montage_Attack02 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("montage.attack02"), FString("attack02"));
	GameplayTags.Montage_Attack03 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("montage.attack03"), FString("attack03"));
	GameplayTags.Montage_Attack04 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("montage.attack04"), FString("attack04"));

	/*
	 * Map of Damage Types to Resistances
	 */
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Poison, GameplayTags.Attributes_Resistance_Poison);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Dark, GameplayTags.Attributes_Resistance_Dark);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Elemental_Electric, GameplayTags.Attributes_Resistance_Electric);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Elemental_Fire, GameplayTags.Attributes_Resistance_Fire);
	GameplayTags.DamageTypeToResist.Add(GameplayTags.Damage_Elemental_Ice, GameplayTags.Attributes_Resistance_Ice);

	/*
	 * Map of Damage Types to De-buffs
	 */
	GameplayTags.DamageTypeToDebuff.Add(GameplayTags.Damage_Physical, GameplayTags.Debuff_KnockOut);
	GameplayTags.DamageTypeToDebuff.Add(GameplayTags.Damage_Poison, GameplayTags.Debuff_KnockOut);
	GameplayTags.DamageTypeToDebuff.Add(GameplayTags.Damage_Dark, GameplayTags.Debuff_Dark);
	GameplayTags.DamageTypeToDebuff.Add(GameplayTags.Damage_Elemental_Electric, GameplayTags.Debuff_Stun);
	GameplayTags.DamageTypeToDebuff.Add(GameplayTags.Damage_Elemental_Fire, GameplayTags.Debuff_Burn);
	GameplayTags.DamageTypeToDebuff.Add(GameplayTags.Damage_Elemental_Ice, GameplayTags.Debuff_Freeze);

	/*
	 * PlayerEvents
	 */
	GameplayTags.Event_EquipKatana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.equip.katana"), FString("Equip Katana"));
	GameplayTags.Event_UnequipKatana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.unequip.katana"), FString("Unequip Katana"));
	GameplayTags.Event_HitMelee= UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.hit.melee"), FString("Melee Hit"));;
	GameplayTags.Event_HitPause = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.HitPause"), FString("Hit Pause"));
	GameplayTags.Event_Death = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.death"), FString("Death"));
	GameplayTags.Event_Strafing = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.strafing"), FString("Strafing"));
	GameplayTags.Event_Evading = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.evading"), FString("Evading"));
	GameplayTags.Event_Blocking = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.block"), FString("Blocking"));
	GameplayTags.Event_Blocking_Successful = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.block.success"), FString("Successful Block"));
	GameplayTags.Event_Blocking_Unblockable = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.block.unblockable"), FString("No Block"));
	GameplayTags.Event_TargetLock = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.targetLock"), FString("Target locking"));
	GameplayTags.Event_SwitchTarget_Left = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.target.left"), FString("Target left"));;
	GameplayTags.Event_SwitchTarget_Right = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.target.right"), FString("Target right"));;
	//Hit React
	GameplayTags.Event_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.HitReact"), FString("Hit React"));
	GameplayTags.Event_HitReact_Front = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.HitReact.front"), FString("Hit React Front"));
	GameplayTags.Event_HitReact_Left = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.HitReact.left"), FString("Hit React Left"));
	GameplayTags.Event_HitReact_Right = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.HitReact.right"), FString("Hit React Right"));
	GameplayTags.Event_HitReact_Back = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("event.HitReact.back"), FString("Hit React Back"));
	
}
