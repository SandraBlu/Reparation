// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BFLAbilitySystem.h"
#include <Kismet/GameplayStatics.h>
#include <UI/HUD/AOHUD.h>
#include <Framework/AOPlayerState.h>
#include "UI/WidgetController/AOWidgetController.h"
#include "Framework/AOGameModeBase.h"
#include "AbilitySystemComponent.h"

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
	AAOGameModeBase* AOGM = Cast<AAOGameModeBase>( UGameplayStatics::GetGameMode(WorldContextObject));
	if (AOGM == nullptr) return;

	AActor* AvatarActor = ASC->GetAvatarActor();
	UCharacterClassInfo* CharacterClassInfo = AOGM->CharacterClassInfo;
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
