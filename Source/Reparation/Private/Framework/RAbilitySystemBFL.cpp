// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RAbilitySystemBFL.h"

#include "AbilitySystemComponent.h"
#include "Framework/RGameMode.h"
#include "UI/AbilityStats/Controllers/RWidgetController.h"
#include "UI/AbilityStats/Controllers/ROverlayController.h"
#include "Framework/RPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/AbilityStats/RHUD.h"
#include "UI/AbilityStats/Controllers/RAttributeWidgetController.h"

URWidgetController* URAbilitySystemBFL::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ARHUD* RHUD = Cast<ARHUD>(PC->GetHUD()))
		{
			ARPlayerState* PS = PC->GetPlayerState<ARPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return RHUD->GetOverlayController(WidgetControllerParams);
		}
	}
	return nullptr;
}

URWidgetController* URAbilitySystemBFL::GetAttributeMenuController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ARHUD* RHUD = Cast<ARHUD>(PC->GetHUD()))
		{
			ARPlayerState* PS = PC->GetPlayerState<ARPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return RHUD->GetAttributeMenuController(WidgetControllerParams);
		}
	}
	return nullptr;
}

void URAbilitySystemBFL::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass,
	float Level, UAbilitySystemComponent* ASC)
{
	ARGameMode* AuraGameMode = Cast<ARGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return;

	AActor* AvatarActor = ASC->GetAvatarActor();
	URCharacterClassData* CharacterClassInfo = AuraGameMode->CharacterClassInfo;
	FCharClassInfo ClassDefaultInfo = CharacterClassInfo->GetClassInfo(CharacterClass);

	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}
