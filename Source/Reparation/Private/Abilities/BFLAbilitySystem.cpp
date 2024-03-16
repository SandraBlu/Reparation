// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BFLAbilitySystem.h"
#include <Kismet/GameplayStatics.h>
#include <UI/HUD/AOHUD.h>
#include <Framework/AOPlayerState.h>
#include "UI/WidgetController/AOWidgetController.h"
#include "Framework/AOGameModeBase.h"
#include "AbilitySystemComponent.h"
#include "AOAbilityTypes.h"
#include <Interfaces/CombatInterface.h>

UOverlayWidgetController* UBFLAbilitySystem::GetOverlayController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AAOHUD* AOHUD = Cast<AAOHUD>(PC->GetHUD()))
		{
			AAOPlayerState* PS = PC->GetPlayerState<AAOPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WCParam(PC, PS, ASC, AS);
			return AOHUD->GetOverlayWidgetController(WCParam);
		}
	}
	return nullptr;
}

UAttributeMenuController* UBFLAbilitySystem::GetAttributeMenuController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AAOHUD* AOHUD = Cast<AAOHUD>(PC->GetHUD()))
		{
			AAOPlayerState* PS = PC->GetPlayerState<AAOPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WCParam(PC, PS, ASC, AS);
			return AOHUD->GetAttributeMenuController(WCParam);
		}
	}
	return nullptr;
}

void UBFLAbilitySystem::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharacterDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	//Add Source Objects(ASC) and Effect Context and Outgoing Spec for AI Attributes Data Asset: CharacterClassInfo
	FGameplayEffectContextHandle PrimaryAttributeContextHandle = ASC->MakeEffectContext();
	PrimaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributeContextHandle = ASC->MakeEffectContext();
	SecondaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributeContextHandle = ASC->MakeEffectContext();
	VitalAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttSpecHandle.Data.Get());
}

void UBFLAbilitySystem::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->SharedAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	const FCharacterDefaultInfo& DefaultInfo =  CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (auto AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ICombatInterface* iCombat = Cast<ICombatInterface>(ASC->GetAvatarActor()))
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, iCombat->GetPlayerLevel()); 
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

UCharacterClassInfo* UBFLAbilitySystem::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	AAOGameModeBase* AOGameMode = Cast<AAOGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AOGameMode == nullptr) return nullptr;
	return AOGameMode->CharacterClassInfo;
}

bool UBFLAbilitySystem::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AOEffectContext->IsBlockedHit();
	}
	return false;
}

bool UBFLAbilitySystem::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAOGameplayEffectContext* AOEffectContext = static_cast<const FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AOEffectContext->IsCriticalHit();
	}
	return false;
}

void UBFLAbilitySystem::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AOEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UBFLAbilitySystem::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FAOGameplayEffectContext* AOEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AOEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UBFLAbilitySystem::GetTargetsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> Overlaps;
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}
