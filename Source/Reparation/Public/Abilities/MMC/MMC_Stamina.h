// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_Stamina.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UMMC_Stamina : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:

	UMMC_Stamina();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:

	FGameplayEffectAttributeCaptureDefinition ResilienceDef;

	
};
