// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RHUD.h"
#include "UI/RUserWidget.h"
#include "UI/Controller/RAbilityMenuController.h"
#include "UI/Controller/RAttributeMenuController.h"
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

URAttributeMenuController* ARHUD::GetAttributeMenuController(const FWidgetControllerParams& WCParams)
{
	if (AttributeMenuController == nullptr)
	{
		AttributeMenuController = NewObject<URAttributeMenuController>(this, AttributeMenuControllerClass);
		AttributeMenuController->SetWidgetControllerParams(WCParams);
		AttributeMenuController->BindCallbacksToDependencies();
	}
	return AttributeMenuController;
}

/*URAbilityMenuController* ARHUD::GetAbilityMenuController(const FWidgetControllerParams& WCParams)
{
	if (AbilityMenuController == nullptr)
	{
		AbilityMenuController = NewObject<URAbilityMenuController>(this, AbilityMenuControllerClass);
		AbilityMenuController->SetWidgetControllerParams(WCParams);
		AbilityMenuController->BindCallbacksToDependencies();
	}
	return AbilityMenuController;
}*/

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
