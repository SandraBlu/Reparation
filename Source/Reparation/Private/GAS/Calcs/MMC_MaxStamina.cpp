// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Calcs/MMC_MaxStamina.h"

#include "GAS/RAttributeSet.h"
#include "Interfaces/RCombatInterface.h"

UMMC_MaxStamina::UMMC_MaxStamina()
{
	ResilienceDef.AttributeToCapture = URAttributeSet::GetResilienceAttribute();
	ResilienceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	ResilienceDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(ResilienceDef);
}

float UMMC_MaxStamina::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags;

	float Resilience = 0.f;
	GetCapturedAttributeMagnitude(ResilienceDef, Spec, EvalParams, Resilience);
	Resilience = FMath::Max<float>(Resilience, 0.f);

	int32 CharacterLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<URCombatInterface>())
	{
		CharacterLevel = IRCombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	return 40.f + 2 * Resilience + 10.f * CharacterLevel;
}
