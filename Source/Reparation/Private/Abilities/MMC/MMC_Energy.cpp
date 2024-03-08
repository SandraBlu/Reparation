// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/MMC/MMC_Energy.h"
#include "Attributes/AOAttributeSet.h"
#include "Interfaces/CombatInterface.h"

UMMC_Energy::UMMC_Energy()
{
	IntDef.AttributeToCapture = UAOAttributeSet::GetIntelligenceAttribute();
	IntDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntDef);
}

float UMMC_Energy::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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

	ICombatInterface* iCombat = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	const int32 PlayerLevel = iCombat->GetPlayerLevel();

	return 10.f + 3 * Intelligence + 5.f * PlayerLevel;
}
