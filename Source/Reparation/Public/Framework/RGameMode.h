// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RGameMode.generated.h"

class URNPCData;
class URAbilityInfo;

/**
 * 
 */
UCLASS()
class REPARATION_API ARGameMode : public AGameModeBase
{
	GENERATED_BODY()

	public:

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	URNPCData* CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	URAbilityInfo* AbilityInfo;
	
};
