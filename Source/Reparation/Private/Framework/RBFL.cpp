// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RBFL.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/RPlayerState.h"
//#include "UI/AOHUD.h"
#include "UI/Controller/RWidgetController.h"
#include "Framework/RGameMode.h"
#include "AbilitySystemComponent.h"
#include "Interface/RCombatInterface.h"
//#include "AOGameplayEffectTypes.h"
#include "RGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include <UI/RHUD.h>

bool URBFL::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, ARHUD*& OutRHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutRHUD = Cast<ARHUD>(PC->GetHUD());
		if (OutRHUD)
		{
			ARPlayerState* PS = PC->GetPlayerState<ARPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			OutWCParams.PlayerController = PC;
			OutWCParams.PlayerState = PS;
			OutWCParams.AbilitySystemComponent = ASC;
			OutWCParams.AttributeSet = AS;
			return true;
		}
	}
	return false;
}

URHUDController* URBFL::GetOverlayController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARHUD* RHUD = nullptr;

	if (MakeWidgetControllerParams(WorldContextObject, WCParams, RHUD))
	{
		return RHUD->GetOverlayController(WCParams);
	}
	return nullptr;
}

//FGameplayEffectContextHandle UAOBFL::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
//{
//	const FRGameplayTags& GameplayTags = FAOGameplayTags::Get();
//	const AActor* SourceAvatarActor = DamageEffectParams.SourceASC->GetAvatarActor();
//
//	FGameplayEffectContextHandle EffectContexthandle = DamageEffectParams.SourceASC->MakeEffectContext();
//	EffectContexthandle.AddSourceObject(SourceAvatarActor);
//	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceASC->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContexthandle);
//
//	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
//	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
//	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
//	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
//	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
//
//	DamageEffectParams.TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
//	return EffectContexthandle;
//}
//
//float UAOBFL::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
//{
//	if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
//	{
//		return AOEffectContext->GetDebuffDamage();
//	}
//	return 0.f;
//}
//
//float UAOBFL::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
//{
//	if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
//	{
//		return AOEffectContext->GetDebuffDuration();
//	}
//	return 0.f;
//}
//
//float UAOBFL::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
//{
//	if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
//	{
//		return AOEffectContext->GetDebuffFrequency();
//	}
//	return 0.f;
//}
//
//FGameplayTag UAOBFL::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
//{
//	if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
//	{
//		if (AOEffectContext->GetDamageType().IsValid())
//		{
//			return *AOEffectContext->GetDamageType();
//		}
//	}
//	return FGameplayTag();
//}