// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Calcs/GEC_Damage.h"

#include "RAbilityTypes.h"
#include "RGameplayTags.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/RCharacterClassInfo.h"
#include "Interfaces/RCombatInterface.h"
#include "Kismet/GameplayStatics.h"

struct RDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DodgeChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritHitDamage);

	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PoisonResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DarkResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ElectricResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(IceResistance);

	RDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, DodgeChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, CritHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, CritHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, CritHitDamage, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, PhysicalResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, PoisonResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, DarkResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, ElectricResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAttributeSet, IceResistance, Target, false);

		
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

	RelevantAttributesToCapture.Add(DamageStatics().DarkResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ElectricResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().IceResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PoisonResistanceDef);
}

void UGEC_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& Spec, FAggregatorEvaluateParameters EvalParams,
	const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const
{
	const FRGameplayTags& Tag = FRGameplayTags::Get();
	for (TTuple<FGameplayTag, FGameplayTag> Pair : Tag.DamageTypeToDebuff)
	{
		const FGameplayTag& DamageType = Pair.Key;
		const FGameplayTag& DebuffType = Pair.Value;
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1.f);
		if (TypeDamage > -.5f)// padding for float imprecision
		{
			//determine if there was a successful debuff
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(Tag.Debuff_Chance, false, -1.f);
			float TargetDebuffResist = 0.f;
			const FGameplayTag& ResistanceTag = Tag.DamageTypeToResist[DamageType];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(InTagsToDefs[ResistanceTag], EvalParams, TargetDebuffResist);
			TargetDebuffResist = FMath::Max<float>(TargetDebuffResist, 0.f);
			const float EffectiveDebuffChance = SourceDebuffChance * (100 - TargetDebuffResist) / 100.f;
			const bool bDebuff  = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			if (bDebuff)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
				URAbilitySystemLibrary::SetIsSucessfulDebuff(ContextHandle, true);
				URAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);
				const float DebuffDamage = Spec.GetSetByCallerMagnitude(Tag.Debuff_Damage, false, -1.f);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(Tag.Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(Tag.Debuff_Frequency, false, -1.f);

				URAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
				URAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
				URAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
			}
		}
	}
}

void UGEC_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	const FRGameplayTags& Tags = FRGameplayTags::Get();
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, DamageStatics().ArmorDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, DamageStatics().BlockChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, DamageStatics().CritHitChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, DamageStatics().CritHitDamageDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, DamageStatics().CritHitResistanceDef);

	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Dark, DamageStatics().DarkResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Electric, DamageStatics().ElectricResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Ice, DamageStatics().IceResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Poison, DamageStatics().PoisonResistanceDef);
	
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	int32 SourceCharacterLevel = 1;
	if (SourceAvatar->Implements<URCombatInterface>())
	{
		SourceCharacterLevel = IRCombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}

	int32 TargetCharacterLevel = 1;
	if (TargetAvatar->Implements<URCombatInterface>())
	{
		TargetCharacterLevel = IRCombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags; 
	//-----------------------------------------------------------------------------------

	//Debuff
	DetermineDebuff(ExecutionParams, Spec, EvalParams, TagsToCaptureDefs);
	
	float Damage = 0.f;
	for (const auto& Pair  : FRGameplayTags::Get().DamageTypeToResist)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key);
		if (DamageTypeValue <= 0)
		{
			continue;// skips rest of code inside for loop
		}
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvalParams, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		DamageTypeValue *= (100.f - Resistance) / 100.f;

		//Radial Falloff Scales Damage Down further from center of blast
		if (URAbilitySystemLibrary::IsRadialDamage(EffectContextHandle))
		{
			if (IRCombatInterface* CombatInterface = Cast<IRCombatInterface>(TargetAvatar))
			{
				CombatInterface->GetOnDamageSignature().AddLambda([&](float DamageAmount)
				{
					DamageTypeValue = DamageAmount;
				});
			}
			UGameplayStatics::ApplyRadialDamageWithFalloff(TargetAvatar, DamageTypeValue, 0.f, URAbilitySystemLibrary::GetRadialDamageOrigin(EffectContextHandle),
				URAbilitySystemLibrary::GetRadialDamageInnerRadius(EffectContextHandle), URAbilitySystemLibrary::GetRadialDamageOuterRadius(EffectContextHandle),
				1.f, UDamageType::StaticClass(), TArray<AActor*>(), SourceAvatar, nullptr);
		}
		
		Damage += DamageTypeValue;
	}
	
	//Block
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvalParams, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	URAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);
	//If block, halve the damage
	Damage = bBlocked ? Damage / 2.f : Damage;

	// Capture DodgeChance on Target, and determine if there was a successful Dodge
	float TargetDodgeChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DodgeChanceDef, EvalParams, TargetDodgeChance);
	TargetDodgeChance = FMath::Max<float>(TargetDodgeChance, 0.f);
	const bool bDodged = FMath::RandRange(1, 100) < TargetDodgeChance;
	URAbilitySystemLibrary::SetIsDodgedHit(EffectContextHandle, bDodged);
	//Dodge, Damage reduced by 90%
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
	URAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);
	// Double damage, plus bonus if critical hit
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;
	
	const FGameplayModifierEvaluatedData EvaluatedData(URAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
