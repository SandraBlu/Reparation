// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RHUD.h"
#include "UI/RUserWidget.h"
#include "UI/Controller/RHUDController.h"

URHUDController* ARHUD::GetOverlayController(const FWidgetControllerParams& WCParams)
{
	if (HUDController == nullptr)
	{
		HUDController = NewObject<URHUDController>(this, HUDControllerClass);
		HUDController->SetWidgetControllerParams(WCParams);
		HUDController->BindCallbacksToDependencies();
		return HUDController;
	}
	return HUDController;
}

void ARHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class unintialized, please fill out BP_HUD"));
	checkf(HUDControllerClass, TEXT("Overlay Controller Class unintialized, please fill out BP_HUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<URUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	URHUDController* WidgetController = GetOverlayController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}
