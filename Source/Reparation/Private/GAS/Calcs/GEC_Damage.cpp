// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Calcs/GEC_Damage.h"

#include "RGameplayTags.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/RCharacterClassInfo.h"
#include "Interfaces/RCombatInterface.h"

struct RDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DodgeChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitDamage);

	/*DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PoisonResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DarkResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ElectricResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(IceResistance);*/
	

	RDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, DodgeChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, CritHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, CritHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, CritHitDamage, Source, false);

		/*DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, PhysicalResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, PoisonResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, DarkResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, ElectricResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, IceResistance, Target, false);*/
		
	}
};

static const RDamageStatics& DamageStatics()
{
	static RDamageStatics DStatics;
	return DStatics;
}

UGEC_Damage::UGEC_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().DodgeChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritHitDamageDef);
}

void UGEC_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	int32 SourceCharacterLevel = 1;
	if (SourceAvatar->Implements<URCombatInterface>())
	{
		SourceCharacterLevel = IRCombatInterface::Execute_GetPLayerLevel(SourceAvatar);
	}

	int32 TargetCharacterLevel = 1;
	if (TargetAvatar->Implements<URCombatInterface>())
	{
		TargetCharacterLevel = IRCombatInterface::Execute_GetPLayerLevel(TargetAvatar);
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags; 
	//-----------------------------------------------------------------------------------

	float Damage = Spec.GetSetByCallerMagnitude(FRGameplayTags::Get().Damage);
	
	//Block
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvalParams, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	//If block, halve the damage
	Damage = bBlocked ? Damage / 2.f : Damage;

	// Capture DodgeChance on Target, and determine if there was a successful Dodge
	float TargetDodgeChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DodgeChanceDef, EvalParams, TargetDodgeChance);
	TargetDodgeChance = FMath::Max<float>(TargetDodgeChance, 0.f);

	//Dodge, Damage reduced by 90%
	const bool bDodged = FMath::RandRange(1, 100) < TargetDodgeChance;
	//URAbilitySystemLibrary::SetIsDodgedHit(EffectContextHandle, bDodged);
	Damage = bDodged ? Damage / 10.f : Damage;
	
	//Armor--------------------
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvalParams, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);

	//Armor Penetration
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvalParams, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);

	const URCharacterClassInfo* CharClassInfo = URAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	const FRealCurve* ArmorPenetrationCurve = CharClassInfo->DamageCalcCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourceCharacterLevel);
	
	//Armor penetration ignores percent of target Armor
	const float EffectiveArmor = TargetArmor*= (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;

	const FRealCurve* EffectiveArmorCurve = CharClassInfo->DamageCalcCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetCharacterLevel);
	
	// Armor ignores a percentage of incoming Damage.
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;

	//Critical Chances
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritHitChanceDef, EvalParams, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);

	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritHitResistanceDef, EvalParams, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritHitDamageDef, EvalParams, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

	const FRealCurve* CriticalHitResistanceCurve = CharClassInfo->DamageCalcCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetCharacterLevel);

	// Critical Hit Resistance reduces Critical Hit Chance by a certain percentage
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;

	//URAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	// Double damage, plus bonus if critical hit
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;
	
	const FGameplayModifierEvaluatedData EvaluatedData(URAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
