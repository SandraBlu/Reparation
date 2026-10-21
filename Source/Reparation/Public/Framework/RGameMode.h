// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GAS/Data/RCombatBalanceSettings.h"
#include "RGameMode.generated.h"

enum class ERGameDifficulty : uint8;
class UAbilityInfo;
class URCharacterClassInfo;
/**
 * 
 */
UCLASS()
class REPARATION_API ARGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat Balance")
	TObjectPtr<URCombatBalanceSettings> CombatBalanceSettings;
	UFUNCTION(BlueprintPure, Category="Combat Balance")
	FRCombatDifficultyProfile GetCombatDifficultyProfile() const;
	UFUNCTION(BlueprintCallable, Category="Game Settings")
	void SetGameDifficulty(ERGameDifficulty Difficulty) { CurrentGameDifficulty = Difficulty; }

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	URCharacterClassInfo* CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	UAbilityInfo* AbilityInfo;

	UFUNCTION(BlueprintPure, Category="Game Settings")
	ERGameDifficulty GetCurrentGameDifficulty() const { return CurrentGameDifficulty; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Settings")
	ERGameDifficulty CurrentGameDifficulty;
};
