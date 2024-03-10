// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AOGameModeBase.generated.h"


class UCharacterClassInfo;


UCLASS()
class REPARATION_API AAOGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	UCharacterClassInfo* CharacterClassInfo;
};
