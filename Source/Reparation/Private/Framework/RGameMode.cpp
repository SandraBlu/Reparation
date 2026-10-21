// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RGameMode.h"

FRCombatDifficultyProfile ARGameMode::GetCombatDifficultyProfile() const
{
	const URCombatBalanceSettings* Settings = CombatBalanceSettings ? CombatBalanceSettings.Get() : GetDefault<URCombatBalanceSettings>();
	return Settings->GetProfile(CurrentGameDifficulty);
}

