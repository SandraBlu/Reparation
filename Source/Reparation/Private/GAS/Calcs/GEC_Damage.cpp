// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Calcs/GEC_Damage.h"

#include "RAbilityTypes.h"
#include "RGameplayTags.h"
#include "Characters/RPlayer.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Data/RCharacterClassInfo.h"
#include "Interfaces/RCombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "AI/REnemy.h"
#include "Framework/RGameMode.h"
#include "Engine/World.h"
#include "Characters/RCharacterBase.h"
#include "GAS/Data/RElementalResistanceProfile.h"

float UGEC_Damage::CalculateArmorMultiplier(float Armor, float Penetration, float PenetrationCoefficient, float ArmorCoefficient)
{
	const float IgnoredArmor = FMath::Clamp(FMath::Max(0.f, Penetration) * FMath::Max(0.f, PenetrationCoefficient), 0.f, 100.f);
	const float EffectiveArmor = FMath::Max(0.f, Armor) * (1.f - IgnoredArmor / 100.f);
	return 1.f - FMath::Clamp(EffectiveArmor * FMath::Max(0.f, ArmorCoefficient), 0.f, 100.f) / 100.f;
}

bool UGEC_Damage::RollChance(float Chance)
{
	return FMath::RandRange(1, 100) <= FMath::Clamp(Chance, 0.f, 100.f);
}

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
			TargetDebuffResist = FMath::Clamp(TargetDebuffResist, 0.f, 100.f);
			const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
			const ARCharacterBase* Target = TargetASC ? Cast<ARCharacterBase>(TargetASC->GetAvatarActor()) : nullptr;
			const float TypeMultiplier = Target && Target->ElementalResistanceProfile
				? Target->ElementalResistanceProfile->GetDamageMultiplier(DamageType, const_cast<ARCharacterBase*>(Target)->GetPlayerLevel_Implementation()) : 1.f;
			const float EffectMultiplier = (100 - TargetDebuffResist) / 100.f * TypeMultiplier;
			const float EffectiveDebuffChance = SourceDebuffChance * EffectMultiplier;
			const bool bDebuff = RollChance(EffectiveDebuffChance);
			if (bDebuff)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
				URAbilitySystemLibrary::SetIsSucessfulDebuff(ContextHandle, true);
				URAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);
				const float DebuffDamage = Spec.GetSetByCallerMagnitude(Tag.Debuff_Damage, false, -1.f);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(Tag.Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(Tag.Debuff_Frequency, false, -1.f);

				URAbilitySystemLibrary::SetDebuffDamage(ContextHandle, FMath::Max(0.f, DebuffDamage * EffectMultiplier));
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
	if (IsValid(SourceAvatar) && SourceAvatar->Implements<URCombatInterface>())
	{
		SourceCharacterLevel = IRCombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}

	int32 TargetCharacterLevel = 1;
	if (IsValid(TargetAvatar) && TargetAvatar->Implements<URCombatInterface>())
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

		// A single-type attack need not supply magnitudes for every other damage type.
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false, 0.f);
		if (DamageTypeValue <= 0)
		{
			continue;// skips rest of code inside for loop
		}
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvalParams, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		DamageTypeValue *= (100.f - Resistance) / 100.f;
		if (const ARCharacterBase* Character = Cast<ARCharacterBase>(TargetAvatar); Character && Character->ElementalResistanceProfile)
			DamageTypeValue *= Character->ElementalResistanceProfile->GetDamageMultiplier(DamageTypeTag, TargetCharacterLevel);

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
	const bool bBlocked = RollChance(TargetBlockChance);
	URAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);
	//If block, halve the damage
	Damage = bBlocked ? Damage / 2.f : Damage;

	// Capture DodgeChance on Target, and determine if there was a successful Dodge
	float TargetDodgeChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DodgeChanceDef, EvalParams, TargetDodgeChance);
	TargetDodgeChance = FMath::Max<float>(TargetDodgeChance, 0.f);
	const bool bDodged = RollChance(TargetDodgeChance);
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
	auto Coefficient = [CharClassInfo](FName Name, int32 Level)
	{
		const FRealCurve* Curve = CharClassInfo && CharClassInfo->DamageCalcCoefficients ? CharClassInfo->DamageCalcCoefficients->FindCurve(Name, FString(), false) : nullptr;
		return Curve ? FMath::Max(0.f, Curve->Eval(FMath::Max(1, Level))) : 0.f;
	};
	const float ArmorPenetrationCoefficient = Coefficient(TEXT("ArmorPenetration"), SourceCharacterLevel);
	
	//Armor penetration ignores percent of target Armor

	const float EffectiveArmorCoefficient = Coefficient(TEXT("EffectiveArmor"), TargetCharacterLevel);
	
	// Armor ignores a percentage of incoming Damage.
	Damage *= CalculateArmorMultiplier(TargetArmor, SourceArmorPenetration, ArmorPenetrationCoefficient, EffectiveArmorCoefficient);

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

	const float CriticalHitResistanceCoefficient = Coefficient(TEXT("CriticalHitResistance"), TargetCharacterLevel);

	// Critical Hit Resistance reduces Critical Hit Chance by a certain percentage
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = RollChance(EffectiveCriticalHitChance);
	URAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);
	// Double damage, plus bonus if critical hit
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;
	if (const AREnemy* Enemy = Cast<AREnemy>(SourceAvatar))
		if (const ARGameMode* GM = Enemy->GetWorld()->GetAuthGameMode<ARGameMode>()) Damage *= FMath::Clamp(GM->GetCombatDifficultyProfile().EnemyDamageMultiplier, 0.1f, 5.f);
	Damage = FMath::IsFinite(Damage) ? FMath::Max(0.f, Damage) : 0.f;
	
	const FGameplayModifierEvaluatedData EvaluatedData(URAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
