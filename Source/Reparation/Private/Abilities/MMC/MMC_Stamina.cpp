// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/MMC/MMC_Stamina.h"
#include "Attributes/AOAttributeSet.h"
#include "Interfaces/CombatInterface.h"

UMMC_Stamina::UMMC_Stamina()
{
	ResilienceDef.AttributeToCapture = UAOAttributeSet::GetResilienceAttribute();
	ResilienceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	ResilienceDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(ResilienceDef);
}


float UMMC_Stamina::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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

	ICombatInterface* iCombat = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	const int32 PlayerLevel = iCombat->GetPlayerLevel();

	return 40.f + 2 * Resilience + 10.f * PlayerLevel;
}
