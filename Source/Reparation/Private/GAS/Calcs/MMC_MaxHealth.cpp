// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Calcs/MMC_MaxHealth.h"

#include "GAS/RAttributeSet.h"
#include "Interfaces/RCombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	VigorDef.AttributeToCapture = URAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvalParams, Vigor);
	//Clamp min to zero
	Vigor = FMath::Max<float>(Vigor, 0.f);

	int32 CharacterLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<URCombatInterface>())
	{
		CharacterLevel = IRCombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	return 80.f + 2.5 * Vigor + 10.f * CharacterLevel;
}
