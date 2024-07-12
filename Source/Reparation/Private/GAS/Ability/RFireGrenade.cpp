// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RFireGrenade.h"

FString URFireGrenade::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float Cooldown = GetCooldown(Level);
	const int32 Radius = Level * 2500;
	if (Level == 1)
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Blast Grenade</>\n\n"
		//Details: Level, Radius, cooldown
		"<Small>Level: </><Level>%d</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Description: 
		"<Default>Thrown Blast grenade.</>\n\n"
		//Damage
		"<Default>Blast affects targets within </> <Radius>%d</> <Default>meter radius, dealing:</> <Damage>%d</> <Default>fire damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, Cooldown, Radius, ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Blast Grenade</>\n\n"
		//Details: Level, Radius, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Damage
		"<Default>Blast affects targets within </> <Radius>%d</> <Default>meter radius, dealing:</> <Damage>%d</> <Default>fire damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, Cooldown, Radius, ScaledDamage);
	}
}

FString URFireGrenade::GetNextLevelDescription(int32 Level)
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
		"<Default>Blast affects targets within </> <Radius>%d</> <Default>meter radius, dealing:</> <Damage>%d</> <Default>fire damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, Cooldown, Radius, ScaledDamage);
}
