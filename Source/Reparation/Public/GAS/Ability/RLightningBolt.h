// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RMissileAbility.h"
#include "RLightningBolt.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URLightningBolt : public URMissileAbility
{
	GENERATED_BODY()

public:
	
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};
