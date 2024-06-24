// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RGameMode.generated.h"

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
	
};
