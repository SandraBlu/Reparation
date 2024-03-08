// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "AOEnemy.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API AAOEnemy : public AAOCharacter
{
	GENERATED_BODY()

public:

	AAOEnemy();

	//Combat Interface

	virtual int32 GetPlayerLevel() override;

protected:

	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Settings")
	int32 Level = 1;
};
