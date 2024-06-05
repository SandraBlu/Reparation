// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RAbilitySystemLibrary.h"

#include "Framework/RPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GAS/RHUD.h"
#include "UI/GAS/Controllers/RWidgetController.h"

UROverlayWidgetController* URAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ARHUD* RHUD = Cast<ARHUD>(PC->GetHUD()))
		{
			ARPlayerState* PS = PC->GetPlayerState<ARPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return RHUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

URAttributeMenuController* URAbilitySystemLibrary::GetAttributeMenuController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ARHUD* RHUD = Cast<ARHUD>(PC->GetHUD()))
		{
			ARPlayerState* PS = PC->GetPlayerState<ARPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return RHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}
