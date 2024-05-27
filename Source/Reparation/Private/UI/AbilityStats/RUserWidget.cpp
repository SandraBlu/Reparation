// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilityStats/RUserWidget.h"

void URUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	InitializeWidgetController();
}
