// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Calc/MMC_MaxEnergy.h"
#include "AbilitySystem/RAttributeSet.h"
#include "Interface/RCombatInterface.h"

UMMC_MaxEnergy::UMMC_MaxEnergy()
{
	IntDef.AttributeToCapture = URAttributeSet::GetIntelligenceAttribute();
	IntDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntDef);
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
	GetCapturedAttributeMagnitude(IntDef, Spec, EvalParams, Intelligence);
	Intelligence = FMath::Max<float>(Intelligence, 0.f);

	int32 CharacterLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<URCombatInterface>())
	{
		CharacterLevel = IRCombatInterface::Execute_GetCharacterLevel(Spec.GetContext().GetSourceObject());
	}

	return 25.f + 3 * Intelligence + 5.f * CharacterLevel;
}