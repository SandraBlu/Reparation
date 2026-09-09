// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RDamageAbility.h"
#include "RFireGrenade.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URFireGrenade : public URDamageAbility
{
	GENERATED_BODY()

public:
	
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
	
};
