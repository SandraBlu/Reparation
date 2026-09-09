// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RDamageAbility.h"
#include "RMeleeAbility.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URMeleeAbility : public URDamageAbility
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintPure, Category = "Ability")
	class UPawnCombatComponent* GetCombatComponentFromActorInfo();
	
};
