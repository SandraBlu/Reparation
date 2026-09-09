// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RStunGrenade.h"

FString URStunGrenade::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float Cooldown = GetCooldown(Level);
	const int32 Radius = Level * 2500;
	if (Level == 1)
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Stun Grenade</>\n\n"
		//Details: Level, Radius, cooldown
		"<Small>Level: </><Level>%d</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Description: 
		"<Default>Thrown Stun grenade.</>\n\n"
		//Damage
		"<Default>Cloud of smoke affects targets within </> <Radius>%d</> <Default>meter radius, dealing:</> <Damage>%d</> <Default>throwback damage.</>\n\n"
		//Debuff
		" <Default> Chance to stun</>\n\n"),
		//Values
		Level, Cooldown, Radius, ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Electric Bolt</>\n\n"
		//Details: Level, Radius, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Damage
		"<Default>Cloud of smoke affects targets within</> <Radius>%d</> <Default>meter radius, dealing:</> <Damage>%d</> <Default>throwback damage.</>\n\n"
		//Debuff
		" <Default> Chance to stun</>\n\n"),
		//Values
		Level, Cooldown, Radius, ScaledDamage);
	}
}

FString URStunGrenade::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float Cooldown = GetCooldown(Level);
	const int32 Radius = Level * 2500;
	return FString::Printf(TEXT(
		//Title
		"<Title>Next Level:</>\n\n"
		//Details: Level, Radius, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Damage
		"<Default>Cloud of smoke affects targets within</> <Radius>%d</> <Default>meter radius, dealing:</> <Damage>%d</> <Default>throwback damage.</>\n\n"
		//Debuff
		" <Default> Chance to stun</>\n\n"),
		//Values
		Level, Cooldown, Radius, ScaledDamage);
}
