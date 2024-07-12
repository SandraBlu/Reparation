// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RFireBolt.h"

FString URFireBolt::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float EnergyCost = FMath::Abs(GetEnergyCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Fire Bolt</>\n\n"
		//Details: Level, EnergyCost, cooldown
		"<Small>Level: </><Level>%d</>\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Description: 
		"<Default>Launches a firey bolt.</>\n\n"
		//Damage
		"<Default>Spreads on impact dealing:</> <Damage>%d</> <Default>fire damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Fire Bolt</>\n\n"
		//Details: Level, EnergyCost, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Num Projectiles
		"<Default>Launches %d fire bolts.</>\n\n"
		//Damage
		"<Default>Spreads rapidly on impact dealing:</> <Damage>%d</> <Default>fire damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, FMath::Min(Level, NumProjectiles), ScaledDamage);
	}
}

FString URFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float EnergyCost = FMath::Abs(GetEnergyCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		//Title
		"<Title>Next Level:</>\n\n"
		//Details: Level, EnergyCost, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Num Projectiles
		"<Default>Launches %d fire bolts.</>\n\n"
		//Damage
		"<Default>Spreads rapidly on impact dealing:</> <Damage>%d</> <Default>fire damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, FMath::Min(Level, NumProjectiles), ScaledDamage);
}
