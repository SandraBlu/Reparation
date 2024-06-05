// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Calcs/MMC_MaxEnergy.h"

#include "GAS/RAttributeSet.h"
#include "Interfaces/RCombatInterface.h"

UMMC_MaxEnergy::UMMC_MaxEnergy()
{
	IntelligenceDef.AttributeToCapture = URAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntelligenceDef);
}

float UMMC_MaxEnergy::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags;

	float Intelligence = 0.f;
	GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvalParams, Intelligence);
	Intelligence = FMath::Max<float>(Intelligence, 0.f);

	int32 CharacterLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<URCombatInterface>())
	{
		CharacterLevel = IRCombatInterface::Execute_GetPLayerLevel(Spec.GetContext().GetSourceObject());
	}
	
	return 25.f + 3 * Intelligence + 5.f * CharacterLevel;
}
