// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_Energy.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UMMC_Energy : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:

	UMMC_Energy();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:

	FGameplayEffectAttributeCaptureDefinition IntDef;
	
};
