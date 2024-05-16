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

//bool URBFL::MakeWidgetControllerParama(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AAOHUD*& OutAOHUD)
//{
//	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
//	{
//		OutAOHUD = Cast<AAOHUD>(PC->GetHUD());
//		if (OutAOHUD)
//		{
//			AAOPlayerState* PS = PC->GetPlayerState<AAOPlayerState>();
//			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
//			UAttributeSet* AS = PS->GetAttributeSet();
//			OutWCParams.PlayerController = PC;
//			OutWCParams.PlayerState = PS;
//			OutWCParams.AbilitySystemComponent = ASC;
//			OutWCParams.AttributeSet = AS;
//			return true;
//		}
//	}
//	return false;
//}

URHUDController* URBFL::GetOverlayController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	//AAOHUD* AOHUD = nullptr;

	//if (MakeWidgetControllerParama(WorldContextObject, WCParams, AOHUD))
	//{
	//	return AOHUD->GetOverlayController(WCParams);
	//}
	return nullptr;
}

URAttributeMenuController* URBFL::GetAttributeMenuController(const UObject* WorldContextObject)
{
	/*FWidgetControllerParams WCParams;
	AAOHUD* AOHUD = nullptr;

	if (MakeWidgetControllerParama(WorldContextObject, WCParams, AOHUD))
	{
		return AOHUD->GetAttributeMenuController(WCParams);
	}*/
	return nullptr;
}

URAbilityMenuController* URBFL::GetAbilityMenuController(const UObject* WorldContextObject)
{
	//FWidgetControllerParams WCParams;
	//AAOHUD* AOHUD = nullptr;

	//if (MakeWidgetControllerParama(WorldContextObject, WCParams, AOHUD))
	//{
	//	return AOHUD->GetAbilityMenuController(WCParams);
	//}
	return nullptr;
}

void URBFL::InitializeDefaultAttributes(const UObject* WorldContextObject, ENPCClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();
	URNPCData* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FNPCClassInfo ClassDefaultInfo = CharacterClassInfo->GetClassInfo(CharacterClass);

	//Add Source Objects(ASC) and Effect Context and Outgoing Spec for AI Attributes Data Asset: CharacterClassInfo
	FGameplayEffectContextHandle PrimaryAttributeContextHandle = ASC->MakeEffectContext();
	PrimaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributeContextHandle = ASC->MakeEffectContext();
	SecondaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttSpecHandle.Data.Get());

	FGameplayEffectContextHandle ResistanceAttributeContextHandle = ASC->MakeEffectContext();
	ResistanceAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle ResistanceAttSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->ResistanceAttributes, Level, ResistanceAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*ResistanceAttSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributeContextHandle = ASC->MakeEffectContext();
	VitalAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttSpecHandle.Data.Get());
}

URNPCData* URBFL::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	ARGameMode* RGameMode = Cast<ARGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (RGameMode == nullptr) return nullptr;
	return RGameMode->CharacterClassInfo;
}

URAbilityInfo* URBFL::GetAbilityInfo(const UObject* WorldContextObject)
{
	ARGameMode* AOGameMode = Cast<ARGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AOGameMode == nullptr) return nullptr;
	return AOGameMode->AbilityInfo;
}

void URBFL::GiveNPCAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ENPCClass CharacterClass)
{
	URNPCData* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->SharedAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	const FNPCClassInfo& DefaultInfo = CharacterClassInfo->GetClassInfo(CharacterClass);
	for (auto AbilityClass : DefaultInfo.StartupAbilities)
	{
		if ((ASC->GetAvatarActor()->Implements<URCombatInterface>()))
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, IRCombatInterface::Execute_GetCharacterLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

bool URBFL::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AOEffectContext->IsBlockedHit();
	}*/
	return false;
}

bool URBFL::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AOEffectContext->IsCriticalHit();
	}*/
	return false;
}

bool URBFL::IsDodgedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AOEffectContext->IsDodgedHit();
	}*/
	return false;
}

bool URBFL::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AOEffectContext->IsSuccessfulDebuff();
	}*/
	return false;
}

void URBFL::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{/*
	if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AOEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}*/
}

void URBFL::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	//if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	//{
	//	AOEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	//}
}

void URBFL::SetIsDodgedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsDodgedHit)
{
	//if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	//{
	//	AOEffectContext->SetIsDodgedHit(bInIsDodgedHit);
	//}
}

void URBFL::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff)
{
	/*if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AOEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}*/
}

void URBFL::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{/*
	if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AOEffectContext->SetDebuffDamage(InDamage);
	}*/
}

void URBFL::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	/*if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AOEffectContext->SetDebuffDuration(InDuration);
	}*/
}

void URBFL::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	/*if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AOEffectContext->SetDebuffFrequency(InFrequency);
	}*/
}

void URBFL::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType)
{
	//if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	//{
	//	const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
	//	AOEffectContext->SetDamageType(DamageType);
	//}
}

void URBFL::GetTargetsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> Overlaps;
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<URCombatInterface>() && !IRCombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(IRCombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

bool URBFL::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool bFriends = FirstActor->ActorHasTag(FName("player")) && SecondActor->ActorHasTag(FName("player")) || FirstActor->ActorHasTag(FName("enemy")) && SecondActor->ActorHasTag(FName("enemy"));
	return !bFriends;
}

int32 URBFL::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ENPCClass CharacterClass, int32 CharacterLevel)
{
	URNPCData* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const FNPCClassInfo& Info = CharacterClassInfo->GetClassInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
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