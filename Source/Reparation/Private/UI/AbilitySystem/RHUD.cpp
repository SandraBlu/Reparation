// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitySystem/RHUD.h"
#include "UI/AbilitySystem/RUserWidget.h"
#include "UI/AbilitySystem/Controller/ROverlayController.h"

UROverlayController* ARHUD::GetOverlayController(const FWidgetControllerParams& WCParams)
{
	if (OverlayController == nullptr)
	{
		OverlayController = NewObject<UROverlayController>(this, OverlayControllerClass);
		OverlayController->SetWidgetControllerParams(WCParams);
		return OverlayController;
	}
	return OverlayController;
}

void ARHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_HUD"));
	checkf(OverlayControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_HUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<URUserWidget>(Widget);
	
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UROverlayController* WidgetController = GetOverlayController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}
