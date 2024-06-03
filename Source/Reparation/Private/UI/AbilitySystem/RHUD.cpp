// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitySystem/RHUD.h"
#include "UI/AbilitySystem/RUserWidget.h"

void ARHUD::BeginPlay()
{
	Super::BeginPlay();

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	Widget->AddToViewport();
}
