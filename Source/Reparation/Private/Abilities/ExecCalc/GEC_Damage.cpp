// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/ExecCalc/GEC_Damage.h"
#include "Attributes/AOAttributeSet.h"
#include "AOGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Abilities/Data/CharacterClassInfo.h"
#include <Interfaces/CombatInterface.h>
#include <Abilities/BFLAbilitySystem.h>

struct AODamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitDamage);

	AODamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, CritHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, CritHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOAttributeSet, CritHitDamage, Source, false);
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
	RelevantAttributesToCapture.Add(DamageStatics().CritHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritHitDamageDef);
}

void UGEC_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvalParameters;
	EvalParameters.SourceTags = SourceTags;
	EvalParameters.TargetTags = TargetTags;
	//-----------------------------------------------------------------------------------

	// Get Damage //Set by Caller Magnitude
	float Damage = 0.f;
	for (auto& Pair : FAOGameplayTags::Get().DamageTypeToResist)
	{
		const float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key);
		Damage += DamageTypeValue;
	}

	// BlockChance //on Target, and determine if there was a successful Block
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvalParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	UBFLAbilitySystem::SetIsBlockedHit(EffectContextHandle, bBlocked);

	Damage = bBlocked ? Damage / 2.f : Damage;

	//Target Armor//
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvalParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);

	// ArmorPenetration //ignores a percentage of the Target's Armor.	Values taken from curve table
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvalParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);
	const UCharacterClassInfo* CharacterClassInfo = UBFLAbilitySystem::GetCharacterClassInfo(SourceAvatar);
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalcCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());

	//Effective Armor //ignores % of Armor penetration using curve table for values
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalcCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetCombatInterface->GetPlayerLevel());
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;

	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritHitChanceDef, EvalParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);

	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritHitResistanceDef, EvalParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritHitDamageDef, EvalParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalcCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetCombatInterface->GetPlayerLevel());

	// Critical Hit Resistance reduces Critical Hit Chance by a certain percentage
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;

	UBFLAbilitySystem::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	// Double damage plus a bonus if critical hit
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;

	const FGameplayModifierEvaluatedData EvaluatedData(UAOAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
		
}
