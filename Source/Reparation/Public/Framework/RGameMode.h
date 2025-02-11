// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	URCharacterClassInfo* CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	UAbilityInfo* AbilityInfo;

	FORCEINLINE ERGameDifficulty GetCurrentGameDifficulty() const { return CurrentGameDifficulty;}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Settings")
	ERGameDifficulty CurrentGameDifficulty;
};
