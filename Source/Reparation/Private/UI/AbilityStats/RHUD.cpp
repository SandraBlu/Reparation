// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilityStats/RHUD.h"
#include "UI/AbilityStats/RUserWidget.h"
#include "UI/AbilityStats/Controllers/ROverlayController.h"

 UROverlayController* ARHUD::GetOverlayController(const FWidgetControllerParams& WCParams)
 {
 	if (OverlayController == nullptr)
 	{
 		OverlayController = NewObject<UROverlayController>(this, OverlayControllerClass);
 		OverlayController->SetWidgetControllerParams(WCParams);
 		OverlayController->BindCallbacksToDependencies();
 		return OverlayController;
	}
 	return OverlayController;
 }

void ARHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
 	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_AuraHUD"));
 	checkf(OverlayControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_AuraHUD"));

 	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
 	OverlayWidget = Cast<URUserWidget>(Widget);
 	
 	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
 	UROverlayController* WidgetController = GetOverlayController(WidgetControllerParams);
 	
 	OverlayWidget->SetWidgetController(WidgetController);
 	WidgetController->BroadcastInitialValues();
 	Widget->AddToViewport();
}
