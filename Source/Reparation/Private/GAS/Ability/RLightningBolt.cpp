// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RLightningBolt.h"

#include "RGameplayTags.h"

FString URLightningBolt::GetDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FRGameplayTags::Get().Damage_Elemental_Electric);
	const float EnergyCost = FMath::Abs(GetEnergyCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Electric Bolt</>\n\n"
		//Details: Level, EnergyCost, cooldown
		"<Small>Level: </><Level>%d</>\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Description: 
		"<Default>Launches a shocking bolt.</>\n\n"
		//Damage
		"<Default>Spreads on impact dealing:</> <Damage>%d</> <Default>electric damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, Damage);
	}
	else
	{
		return FString::Printf(TEXT(
		//Title
		"<Title>Electric Bolt</>\n\n"
		//Details: Level, EnergyCost, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Num Projectiles
		"<Default>Launches %d electrifying bolts.</>\n\n"
		//Damage
		"<Default>Spreads rapidly on impact dealing:</> <Damage>%d</> <Default>electric damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, FMath::Min(Level, NumProjectiles), Damage);
	}
}

FString URLightningBolt::GetNextLevelDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(Level, FRGameplayTags::Get().Damage_Elemental_Electric);
	const float EnergyCost = FMath::Abs(GetEnergyCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		//Title
		"<Title>Electric Bolt</>\n\n"
		//Details: Level, EnergyCost, Cooldown
		"<Small>Level: </><Level>%d</>\n\n"
		"<Small>EnergyCost: </><EnergyCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		//Num Projectiles
		"<Default>Launches %d electrifying bolts.</>\n\n"
		//Damage
		"<Default>Spreads rapidly on impact dealing:</> <Damage>%d</> <Default>electric damage.</>\n\n"
		//Debuff
		" <Default> Chance to burn</>\n\n"),
		//Values
		Level, EnergyCost, Cooldown, FMath::Min(Level, NumProjectiles), Damage);
}
