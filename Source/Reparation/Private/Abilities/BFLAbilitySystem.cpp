// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/BFLAbilitySystem.h"
#include <Kismet/GameplayStatics.h>
#include <UI/HUD/AOHUD.h>
#include <Framework/AOPlayerState.h>
#include "UI/WidgetController/AOWidgetController.h"

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
