// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/RUserWidget.h"

void URUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
