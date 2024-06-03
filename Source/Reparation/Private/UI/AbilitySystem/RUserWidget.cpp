// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitySystem/RUserWidget.h"

void URUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerInit();
}
