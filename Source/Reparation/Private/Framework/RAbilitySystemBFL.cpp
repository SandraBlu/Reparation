// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RAbilitySystemBFL.h"
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
