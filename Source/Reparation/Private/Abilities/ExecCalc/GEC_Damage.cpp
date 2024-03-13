// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/ExecCalc/GEC_Damage.h"
#include "Attributes/AOAttributeSet.h"
#include "AOGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

struct AODamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);

	AODamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, BlockChance, Target, false);
	}
};

static const AODamageStatics& DamageStatics()
{
	static AODamageStatics DStatics;
	return DStatics;
}

UGEC_Damage::UGEC_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
}

void UGEC_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvalParameters;
	EvalParameters.SourceTags = SourceTags;
	EvalParameters.TargetTags = TargetTags;
	//-----------------------------------------------------------------------------------

	// Get Damage Set by Caller Magnitude
	float Damage = Spec.GetSetByCallerMagnitude(FAOGameplayTags::Get().Damage);

	// Capture BlockChance on Target, and determine if there was a successful Block
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvalParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	// If Block, halve the damage.	
	Damage = bBlocked ? Damage / 2.f : Damage;

	//Target Armor
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvalParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);

	//Source ArmorPenetration
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvalParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);
	// ArmorPenetration ignores a percentage of the Target's Armor.	
	const float EffectiveArmor = TargetArmor *= (100 - SourceArmorPenetration * 0.25f) / 100.f;
	// Armor ignores a percentage of incoming Damage.
	Damage *= (100 - EffectiveArmor * 0.333f) / 100.f;

	const FGameplayModifierEvaluatedData EvaluatedData(UAOAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
		
}
