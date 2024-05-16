// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/RAttributeSet.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Framework/RPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "RGameplayTags.h"
#include "Interface/RCombatInterface.h"
#include "Interface/RPlayerInterface.h"
#include "Framework/RBFL.h"
#include "Net/UnrealNetwork.h"
//#include "AOGameplayEffectTypes.h"
//#include "AlphaOmega/AOLogChannel.h"

URAttributeSet::URAttributeSet()
{
	InitHealth(100.f);
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();

	//Primary Attributes
	AttributeTagMap.Add(GameplayTags.Attributes_Primary_Agility, GetAgilityAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Primary_Perception, GetPerceptionAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Primary_Vigor, GetVigorAttribute);

	//Secondary Attributes
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_Armor, GetArmorAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrationAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, GetCritHitChanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, GetCritHitDamageAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, GetCritHitResistanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_DodgeChance, GetDodgeChanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_Stealth, GetStealthAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_HealthRegen, GetHealthRegenAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_StaminaRegen, GetStaminaRegenAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_EnergyRegen, GetEnergyRegenAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_MaxStamina, GetMaxStaminaAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Secondary_MaxEnergy, GetMaxEnergyAttribute);

	AttributeTagMap.Add(GameplayTags.Attributes_Resistance_Dark, GetDarkResistanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Resistance_Electric, GetElectricResistanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Resistance_Fire, GetFireResistanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Resistance_Ice, GetIceResistanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Resistance_Physical, GetPhysicalResistanceAttribute);
	AttributeTagMap.Add(GameplayTags.Attributes_Resistance_Poison, GetPoisonResistanceAttribute);
}

void URAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Energy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Agility, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Perception, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Vigor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, CritHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, CritHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, CritHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, DodgeChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, Stealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, HealthRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, StaminaRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, EnergyRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAttributeSet, MaxEnergy, COND_None, REPNOTIFY_Always);

}

void URAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	//Clamp Base Attributes
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	if (Attribute == GetEnergyAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxEnergy());
	}
}

void URAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	if (Props.TargetCharacter->Implements<URCombatInterface>() && IRCombatInterface::Execute_IsDead(Props.TargetCharacter)) return;

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
	if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
	{
		SetEnergy(FMath::Clamp(GetEnergy(), 0.f, GetMaxEnergy()));
	}
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		//HandleDamage(Props);
	}
	if (Data.EvaluatedData.Attribute == GetXPAttribute())
	{
		HandleXP(Props);
	}
}

void URAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute() && bRefillHealth)
	{
		SetHealth(GetMaxHealth());
		//bRefillHealth = false;
	}
	if (Attribute == GetMaxStaminaAttribute() && bRefillStamina)
	{
		SetStamina(GetMaxStamina());
		//bRefillStamina = false;
	}
	if (Attribute == GetMaxEnergyAttribute() && bRefillEnergy)
	{
		SetEnergy(GetMaxEnergy());
		//bRefillEnergy = false;
	}
}

void URAttributeSet::HandleXP(const FEffectProperties& Props)
{

}

void URAttributeSet::HandleDebuff(const FEffectProperties& Props)
{

}

void URAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	Props.ContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.ContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourcePC = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourcePC == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
			}
		}
		if (Props.SourcePC)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourcePC->GetPawn());
		}
	}
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void URAttributeSet::ShowFloatingText(const FEffectProperties& Props, float DamageAmount, bool bBlockedHit, bool bCriticalHit) const
{

}

void URAttributeSet::SendXPEvent(const FEffectProperties& Props)
{

}

void URAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Health, OldHealth);
}

void URAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Stamina, OldStamina);
}

void URAttributeSet::OnRep_Energy(const FGameplayAttributeData& OldEnergy) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Energy, OldEnergy);
}

void URAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldAgility) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Agility, OldAgility);
}

void URAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Intelligence, OldIntelligence);
}

void URAttributeSet::OnRep_Perception(const FGameplayAttributeData& OldPerception) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Perception, OldPerception);
}

void URAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Resilience, OldResilience);
}

void URAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Strength, OldStrength);
}

void URAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Vigor, OldVigor);
}

void URAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, MaxHealth, OldMaxHealth);
}

void URAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, MaxStamina, OldMaxStamina);
}

void URAttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, MaxEnergy, OldMaxEnergy);
}

void URAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Armor, OldArmor);
}

void URAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void URAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, BlockChance, OldBlockChance);
}

void URAttributeSet::OnRep_DodgeChance(const FGameplayAttributeData& OldDodgeChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, DodgeChance, OldDodgeChance);
}

void URAttributeSet::OnRep_Stealth(const FGameplayAttributeData& OldStealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, Stealth, OldStealth);
}

void URAttributeSet::OnRep_CritHitChance(const FGameplayAttributeData& OldCritHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, CritHitChance, OldCritHitChance);
}

void URAttributeSet::OnRep_CritHitDamage(const FGameplayAttributeData& OldCritHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, CritHitDamage, OldCritHitDamage);
}

void URAttributeSet::OnRep_CritHitResistance(const FGameplayAttributeData& OldCritHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, CritHitResistance, OldCritHitResistance);
}

void URAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, HealthRegen, OldHealthRegen);
}

void URAttributeSet::OnRep_StaminaRegen(const FGameplayAttributeData& OldStaminaRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, StaminaRegen, OldStaminaRegen);
}

void URAttributeSet::OnRep_EnergyRegen(const FGameplayAttributeData& OldEnergyRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAttributeSet, EnergyRegen, OldEnergyRegen);
}
