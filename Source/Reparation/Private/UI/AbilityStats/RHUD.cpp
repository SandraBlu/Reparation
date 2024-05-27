// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilityStats/RHUD.h"
#include "UI/AbilityStats/RUserWidget.h"
//#include "UI/AbilityStats/Controllers/ROverlayController.h"

// UROverlayController* ARHUD::GetOverlayController(const FWidgetControllerParams& WCParams)
// {
// 	if (OverlayController == nullptr)
// 	{
// 		OverlayController = NewObject<UROverlayController>(this, OverlayControllerClass);
// 		OverlayController->SetWidgetControllerParams(WCParams);
// 		OverlayController->BindCallbacksToDependencies();
// 	}
// 	return OverlayController;
// }

void ARHUD::BeginPlay()
{
	Super::BeginPlay();
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	Widget->AddToViewport();
}
