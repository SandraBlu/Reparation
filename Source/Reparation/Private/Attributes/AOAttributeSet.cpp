// Fill out your copyright notice in the Description page of Project Settings.


#include "Attributes/AOAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AOGameplayTags.h"
#include "Interfaces/CombatInterface.h"
#include "Framework/AOPlayerController.h"
#include "Kismet/GameplayStatics.h"

UAOAttributeSet::UAOAttributeSet()
{
	const FAOGameplayTags& GameplayTags = FAOGameplayTags::Get();

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
}

void UAOAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Primary
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Agility, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Perception, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Vigor, COND_None, REPNOTIFY_Always);
	
	//Secondary
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, DodgeChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, CritHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, CritHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, CritHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Stealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, MaxEnergy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, HealthRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, StaminaRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, EnergyRegen, COND_None, REPNOTIFY_Always);

	//Base
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Energy, COND_None, REPNOTIFY_Always);
	
}

void UAOAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
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

void UAOAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		UE_LOG(LogTemp, Warning, TEXT("Changed Health on %s, Health: %f"), *Props.TargetAvatarActor->GetName(), GetHealth());
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
		const float LocalIncomingDamage = GetDamage();
		SetDamage(0.f);
		if (LocalIncomingDamage > 0.f)
		{
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

			const bool bFatal = NewHealth <= 0.f;
			if (bFatal)
			{
				ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
				if (CombatInterface)
				{
					CombatInterface->Die();
				}
			}
			else
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAOGameplayTags::Get().Effects_HitReact);
				Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
			}
			ShowFloatingText(Props, LocalIncomingDamage);
		}
	}
}

void UAOAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
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

void UAOAttributeSet::ShowFloatingText(const FEffectProperties& Props, float DamageText) const
{
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		if (AAOPlayerController* PC = Cast<AAOPlayerController>(UGameplayStatics::GetPlayerController(Props.SourceCharacter, 0)))
		{
			PC->ShowDamageNumber(DamageText, Props.TargetCharacter);
		}
	}
}

//Base
void UAOAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Health, OldHealth);
}
void UAOAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Stamina, OldStamina);
}
void UAOAttributeSet::OnRep_Energy(const FGameplayAttributeData& OldEnergy) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Energy, OldEnergy);
}

//Primary
void UAOAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldAgility) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Agility, OldAgility);
}
void UAOAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Intelligence, OldIntelligence);
}
void UAOAttributeSet::OnRep_Perception(const FGameplayAttributeData& OldPerception) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Perception, OldPerception);
}
void UAOAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Resilience, OldResilience);
}
void UAOAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Strength, OldStrength);
}
void UAOAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Vigor, OldVigor);
}
//Secondary
void UAOAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, MaxHealth, OldMaxHealth);
}
void UAOAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, MaxStamina, OldMaxStamina);
}
void UAOAttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, MaxEnergy, OldMaxEnergy);
}
void UAOAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Armor, OldArmor);
}
void UAOAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, ArmorPenetration, OldArmorPenetration);
}
void UAOAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, BlockChance, OldBlockChance);
}
void UAOAttributeSet::OnRep_DodgeChance(const FGameplayAttributeData& OldDodgeChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, DodgeChance, OldDodgeChance);
}
void UAOAttributeSet::OnRep_CritHitChance(const FGameplayAttributeData& OldCritHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, CritHitChance, OldCritHitChance);
}
void UAOAttributeSet::OnRep_CritHitDamage(const FGameplayAttributeData& OldCritHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, CritHitDamage, OldCritHitDamage);
}
void UAOAttributeSet::OnRep_CritHitResistance(const FGameplayAttributeData& OldCritHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, CritHitResistance, OldCritHitResistance);
}
void UAOAttributeSet::OnRep_Stealth(const FGameplayAttributeData& OldStealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Stealth, OldStealth);
}
void UAOAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, HealthRegen, OldHealthRegen);
}
void UAOAttributeSet::OnRep_StaminaRegen(const FGameplayAttributeData& OldStaminaRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, StaminaRegen, OldStaminaRegen);
}
void UAOAttributeSet::OnRep_EnergyRegen(const FGameplayAttributeData& OldEnergyRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, EnergyRegen, OldEnergyRegen);
}